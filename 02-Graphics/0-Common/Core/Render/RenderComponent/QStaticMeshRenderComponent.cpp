#include "QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"

QStaticMeshRenderComponent* QStaticMeshRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	requestRecreateResource();
	requestRecreatePipeline();
	return this;
}

void QStaticMeshRenderComponent::recreateResource() {
	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, sizeof(UniformMatrix)));
	mUniformBuffer->create();

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(Vertex) * mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(Index) * mIndices.size()));
	mIndexBuffer->create();
}

void QStaticMeshRenderComponent::recreatePipeline() {
	QVector<QRhiVertexInputBinding> inputBindings;
	inputBindings << QRhiVertexInputBinding{ sizeof(Vertex) };

	QVector<QRhiVertexInputAttribute> attributeList;
	attributeList << QRhiVertexInputAttribute{ 0, 0, QRhiVertexInputAttribute::Float3, offsetof(Vertex,position) };
	attributeList << QRhiVertexInputAttribute{ 0, 1, QRhiVertexInputAttribute::Float3, offsetof(Vertex,normal) };
	attributeList << QRhiVertexInputAttribute{ 0, 2, QRhiVertexInputAttribute::Float3, offsetof(Vertex,tangent) };
	attributeList << QRhiVertexInputAttribute{ 0, 3, QRhiVertexInputAttribute::Float3, offsetof(Vertex,bitangent) };
	attributeList << QRhiVertexInputAttribute{ 0, 4, QRhiVertexInputAttribute::Float2, offsetof(Vertex,texCoord) };

	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings(inputBindings.begin(), inputBindings.end());
	inputLayout.setAttributes(attributeList.begin(), attributeList.end());

	QShader vs = QRhiEx::newShaderFromCode(QShader::Stage::VertexStage, R"(#version 440
	layout(location = 0) in vec3 inPosition;
	layout(location = 1) in vec3 inNormal;
	layout(location = 2) in vec3 inTangent;
	layout(location = 3) in vec3 inBitangent;
	layout(location = 4) in vec2 inUV;

	layout(location = 0) out vec2 vUV;
	layout(location = 1) out vec3 vWorldPosition;
	layout(location = 2) out mat3 vTangentBasis;

	out gl_PerVertex{
		vec4 gl_Position;
	};

	layout(std140,binding = 0) uniform UnifromMatrix{
		mat4 MVP;
		mat4 M;
	}UBO;
	void main(){
		vUV = inUV;
		vWorldPosition = vec3(UBO.M * vec4(inPosition,1.0f));
		vTangentBasis =  mat3(UBO.M) * mat3(inTangent, inBitangent, inNormal);
		gl_Position = UBO.MVP * vec4(inPosition,1.0f);
	}
	)");

		QShader fs = QRhiEx::newShaderFromCode(QShader::Stage::FragmentStage, R"(#version 440
	layout(location = 0) in vec2 vUV;
	layout(location = 1) in vec3 vWorldPosition;
	layout(location = 2) in mat3 vTangentBasis;

	layout(location = 0) out vec4 outBaseColor;

	void main(){
		outBaseColor = vec4(1);
	}
	)");

	mPipeline.reset(mRhi->newGraphicsPipeline());

	mPipeline->setVertexInputLayout(inputLayout);

	QVector<QRhiGraphicsPipeline::TargetBlend> blendStates(sceneRenderPass()->getBlendStateCount());
	mPipeline->setTargetBlends(blendStates.begin(), blendStates.end());
	mPipeline->setTopology(QRhiGraphicsPipeline::Topology::Triangles);
	mPipeline->setDepthOp(QRhiGraphicsPipeline::LessOrEqual);
	mPipeline->setDepthTest(true);
	mPipeline->setDepthWrite(true);
	mPipeline->setSampleCount(sceneRenderPass()->getSampleCount());

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
		});

	mShaderResourceBindings.reset(mRhi->newShaderResourceBindings());
	QVector<QRhiShaderResourceBinding> shaderBindings;
	shaderBindings << QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, mUniformBuffer.get());
	mShaderResourceBindings->setBindings(shaderBindings.begin(), shaderBindings.end());
	mShaderResourceBindings->create();

	mPipeline->setShaderResourceBindings(mShaderResourceBindings.get());
	mPipeline->setRenderPassDescriptor(sceneRenderPass()->getRenderPassDescriptor());
	mPipeline->create();
}

void QStaticMeshRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mIndices.constData());
}


void QStaticMeshRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
	UniformMatrix Matrix;
	Matrix.MVP = calculateMatrixMVP().toGenericMatrix<4, 4>();
	Matrix.M = calculateMatrixModel().toGenericMatrix<4, 4>();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(UniformMatrix), &Matrix);
}

void QStaticMeshRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	const QRhiCommandBuffer::VertexInput VertexInput(mVertexBuffer.get(), 0);
	cmdBuffer->setVertexInput(0, 1, &VertexInput, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
	cmdBuffer->drawIndexed(mIndices.size());
}

bool QStaticMeshRenderComponent::isVaild() {
	return mVertices.isEmpty() || mIndices.isEmpty();
}

