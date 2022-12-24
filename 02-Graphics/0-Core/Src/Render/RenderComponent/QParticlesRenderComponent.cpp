#include "QParticlesRenderComponent.h"
#include "Render/IRenderPass.h"
#include "QVulkanInstance"
#include "qvulkanfunctions.h"

static float ParticleShape[] = {
	//position(xy)	
	 0.01f,   0.01f,
	-0.01f,   0.01f,
	 0.01f,  -0.01f,
	-0.01f,  -0.01f,
};

QParticlesRenderComponent::QParticlesRenderComponent() {
	setType(QParticleSystem::Type::GPU);
}

QParticlesRenderComponent* QParticlesRenderComponent::setupColor(QColor4D val) {
	mColor = val;
	return this;
}

QParticlesRenderComponent* QParticlesRenderComponent::setType(QParticleSystem::Type inType) {
	if (inType == QParticleSystem::Type::CPU){
	}
	else {
		mParticleSystem.reset(new QGPUParticleSystem);
	}
	sigonRebuildResource.request();
	sigonRebuildPipeline.request();
	return this;
}

QParticleSystem::Type QParticlesRenderComponent::getType() {
	return mParticleSystem->type();
}

QColor4D QParticlesRenderComponent::getColor() const {
	return mColor;
}

void QParticlesRenderComponent::onRebuildResource() {
	mParticleSystem->onInit(mRhi);

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer, sizeof(ParticleShape)));
	mVertexBuffer->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::UniformBuffer, sizeof(UniformBlock)));
	mUniformBuffer->create();

	if (mParticleSystem && mParticleSystem->type() == QParticleSystem::Type::GPU) {
		mIndirectDrawBuffer.reset(mRhi->newVkBuffer(QRhiBuffer::Static, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, sizeof(IndirectDrawBuffer)));
		mIndirectDrawBuffer->create();
		mIndirectDrawBuffer->setName("IndirectDrawBuffer");
	}
}

void QParticlesRenderComponent::onRebuildPipeline() {
	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		QRhiVertexInputBinding(sizeof(float) * 2),
		QRhiVertexInputBinding(sizeof(float) * 16, QRhiVertexInputBinding::Classification::PerInstance),
	});
	inputLayout.setAttributes({
		QRhiVertexInputAttribute( 0, 0, QRhiVertexInputAttribute::Float2, 0 ),
		QRhiVertexInputAttribute( 1, 1, QRhiVertexInputAttribute::Float4, 0,0 ),
		QRhiVertexInputAttribute( 1, 2, QRhiVertexInputAttribute::Float4, 4 * sizeof(float),1 ),
		QRhiVertexInputAttribute( 1, 3, QRhiVertexInputAttribute::Float4, 8 * sizeof(float),2 ),
		QRhiVertexInputAttribute( 1, 4, QRhiVertexInputAttribute::Float4, 12 * sizeof(float),3 ),
	});
	QShader vs = mRhi->newShaderFromCode(QShader::Stage::VertexStage, R"(#version 440
	layout(location = 0) in vec2 inPosition;
	layout(location = 1) in mat4 inInstTransform;
	layout(location = 0) out vec4 vColor;
	layout(std140,binding = 0) uniform UniformBuffer{
		mat4 MVP;
		mat4 M;
		vec4 color;
	}UBO;

	out gl_PerVertex{
		vec4 gl_Position;
	};
	void main(){
		vec4 pos = inInstTransform * vec4(inPosition,0.0f,1.0f);
		gl_Position = UBO.MVP * pos;
		vColor = UBO.color;
	}
)");

	QShader fs = mRhi->newShaderFromCode(QShader::Stage::FragmentStage, R"(#version 440
layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 outBaseColor;
void main(){
	outBaseColor = vColor;
}
)");

	mPipeline.reset(mRhi->newGraphicsPipeline());
	mPipeline->setVertexInputLayout(inputLayout);
	QVector<QRhiGraphicsPipeline::TargetBlend> targetBlends(sceneRenderPass()->getRenderTargetCount());
	mPipeline->setTargetBlends(targetBlends.begin(), targetBlends.end());
	mPipeline->setTopology(QRhiGraphicsPipeline::Topology::TriangleStrip);
	mPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	mPipeline->setDepthTest(true);
	mPipeline->setDepthWrite(true);
	mPipeline->setSampleCount(sceneRenderPass()->getSampleCount());

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
		});
	mBindings.reset(mRhi->newShaderResourceBindings());
	mBindings->setBindings({
		 QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, mUniformBuffer.get())
	});
	mBindings->create();
	mPipeline->setShaderResourceBindings(mBindings.get());
	mPipeline->setRenderPassDescriptor(sceneRenderPass()->getRenderPassDescriptor());
	mPipeline->create();
}

void QParticlesRenderComponent::onPreUpdate(QRhiCommandBuffer* cmdBuffer){
	 mParticleSystem->onTick(cmdBuffer);
	 if (mParticleSystem->type() == QParticleSystem::Type::GPU) {
		 QGPUParticleSystem* gpuSystem = static_cast<QGPUParticleSystem*>(mParticleSystem.get());
		 QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		 QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		 VkBuffer indirectDispatchBuffer = *(VkBuffer*)gpuSystem->getCurrentIndirectBuffer()->nativeBuffer().objects[0];
		 VkBuffer indirectDrawBuffer = *(VkBuffer*)mIndirectDrawBuffer->nativeBuffer().objects[0];
		 QVulkanInstance* vkInstance = mRhi->getVkInstance();
		 VkBufferCopy bufferCopy;
		 bufferCopy.srcOffset = 0;
		 bufferCopy.dstOffset = offsetof(IndirectDrawBuffer, instanceCount);
		 bufferCopy.size = sizeof(int);
		 vkInstance->deviceFunctions(vkHandles->dev)->vkCmdCopyBuffer(vkCmdBufferHandle->commandBuffer, indirectDispatchBuffer, indirectDrawBuffer, 1, &bufferCopy);
	 }
}

void QParticlesRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), &ParticleShape);
	if (mIndirectDrawBuffer) {
		IndirectDrawBuffer indirectDraw;
		indirectDraw.vertexCount = 4;
		indirectDraw.instanceCount = 0;
		indirectDraw.firstVertex = 0;
		indirectDraw.firstInstance = 0;
		batch->uploadStaticBuffer(mIndirectDrawBuffer.get(), &indirectDraw);
	}
}

void QParticlesRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	UniformBlock uniform;
	uniform.MVP = calculateMatrixMVP().toGenericMatrix<4, 4>();
	uniform.M = calculateMatrixModel().toGenericMatrix<4, 4>();
	uniform.color = QVector4D(mColor.redF(), mColor.greenF(), mColor.blueF(), mColor.alphaF());
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformBlock), &uniform);
}

void QParticlesRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	if (mParticleSystem->type() == QParticleSystem::Type::GPU) {
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		QRhiCommandBuffer::VertexInput VertexInputs[] = {
			{mVertexBuffer.get(), 0},
			{mParticleSystem->getTransformBuffer(),0 }
		};
		cmdBuffer->setVertexInput(0, 2, VertexInputs);
		QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)cmdBuffer->nativeHandles();
		QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
		auto buffer = mIndirectDrawBuffer->nativeBuffer();
		VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
		QVulkanInstance* vkInstance = mRhi->getVkInstance();
		vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDrawIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0, 1, sizeof(IndirectDrawBuffer));
	}
	else {
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		QRhiCommandBuffer::VertexInput VertexInputs[] = {
			{mVertexBuffer.get(), 0},
			{mParticleSystem->getTransformBuffer(),0 }
		};
		cmdBuffer->setVertexInput(0, 2, VertexInputs);
		cmdBuffer->beginExternal();
		cmdBuffer->draw(4);
	}
}

bool QParticlesRenderComponent::isVaild() {
	return !mParticleSystem.isNull();
}
