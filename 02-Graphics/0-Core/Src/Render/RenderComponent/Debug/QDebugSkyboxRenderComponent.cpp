#include "QDebugSkyboxRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Render/RenderPass/QDebugSceneRenderPass.h"

void QDebugSkyboxRenderComponent::onRebuildPipeline() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QVector<QRhiGraphicsPipeline::TargetBlend> blendStates(sceneRenderPass()->getRenderTargetSlots().size());
	mPipeline->setTargetBlends(blendStates.begin(), blendStates.end());
	mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);
	mPipeline->setDepthTest(false);
	mPipeline->setDepthWrite(false);
	mPipeline->setSampleCount(sceneRenderPass()->getSampleCount());

	QVector<QRhiVertexInputBinding> inputBindings;
	inputBindings << QRhiVertexInputBinding{ sizeof(float) * 3 };
	QVector<QRhiVertexInputAttribute> attributeList;
	attributeList << QRhiVertexInputAttribute{ 0, 0, QRhiVertexInputAttribute::Float3, 0 };
	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings(inputBindings.begin(), inputBindings.end());
	inputLayout.setAttributes(attributeList.begin(), attributeList.end());
	mPipeline->setVertexInputLayout(inputLayout);

	QString vertexShaderCode = R"(#version 440
	layout(location = 0) in vec3 inPosition;
	layout(location = 0) out vec3 vPosition
;
	out gl_PerVertex{
		vec4 gl_Position;
	};

	layout(std140 , binding = 0) uniform buf{
		mat4 mvp;
	}ubuf;

	void main(){
		vPosition = inPosition;
		gl_Position = ubuf.mvp * vec4(inPosition,1.0f);
		gl_Position.z = 1.0f;
	}
	)";

	QShader vs = QRhiEx::newShaderFromCode(QShader::Stage::VertexStage, vertexShaderCode.toLocal8Bit());
	
	QDebugSceneRenderPass* debugPass = qobject_cast<QDebugSceneRenderPass*>(sceneRenderPass());
	QString fragShaderCode = QString(QString(R"(#version 440
	layout(location = 0) in vec3 vPosition;
	layout(location = 0) out vec4 BaseColor;
	layout(location = 1) out vec4 DebugID;
	layout(binding = 1) uniform samplerCube uSkybox;
	void main(){
		BaseColor = texture(uSkybox,vPosition);
		DebugID = %1;
	}
	)").arg(debugPass->getIDText(this))
	);

	QShader fs = QRhiEx::newShaderFromCode(QShader::Stage::FragmentStage, fragShaderCode.toLocal8Bit());
	Q_ASSERT(fs.isValid());

	mPipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
		});
	mShaderResourceBindings.reset(mRhi->newShaderResourceBindings());

	QVector<QRhiShaderResourceBinding> shaderBindings;
	shaderBindings << QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, mUniformBuffer.get());
	shaderBindings << QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, mTexture.get(), mSampler.get());
	mShaderResourceBindings->setBindings(shaderBindings.begin(), shaderBindings.end());

	mShaderResourceBindings->create();

	mPipeline->setShaderResourceBindings(mShaderResourceBindings.get());

	mPipeline->setRenderPassDescriptor(sceneRenderPass()->getRenderPassDescriptor());

	mPipeline->create();
}