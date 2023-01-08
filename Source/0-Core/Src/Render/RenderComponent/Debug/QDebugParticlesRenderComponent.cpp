#include "QDebugParticlesRenderComponent.h"
#include "Render/RenderPass/QDebugSceneRenderPass.h"

void QDebugParticlesRenderComponent::onRebuildPipeline() {
	sigonRebuildPipeline.receive();

	QRhiVertexInputLayout inputLayout;
	inputLayout.setBindings({
		QRhiVertexInputBinding(sizeof(float) * 2),
		QRhiVertexInputBinding(sizeof(float) * 16, QRhiVertexInputBinding::Classification::PerInstance),
		});
	inputLayout.setAttributes({
		QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
		QRhiVertexInputAttribute(1, 1, QRhiVertexInputAttribute::Float4, 0,0),
		QRhiVertexInputAttribute(1, 2, QRhiVertexInputAttribute::Float4, 4 * sizeof(float),1),
		QRhiVertexInputAttribute(1, 3, QRhiVertexInputAttribute::Float4, 8 * sizeof(float),2),
		QRhiVertexInputAttribute(1, 4, QRhiVertexInputAttribute::Float4, 12 * sizeof(float),3),
		});
	QString vsCode = R"(#version 440
	layout(location = 0) in vec2 inPosition;
	layout(location = 1) in mat4 inInstTransform;
	layout(location = 0) out vec4 vColor;
	layout(std140,binding = 0) uniform UniformBuffer{
		mat4 MV;
		mat4 P;
		vec4 Color;
	}UBO;

	out gl_PerVertex{
		vec4 gl_Position;
	};
	%1
)";
	if (bFacingCamera) {
		vsCode = vsCode.arg(R"(
			void main(){
				mat4 Transform = UBO.MV * inInstTransform;
				mat3 CorrRotation = inverse(mat3(Transform));
				vec3 CorrPos        = CorrRotation * vec3(inPosition,0.0f);
				gl_Position = UBO.P * Transform * vec4(CorrPos, 1.0); 
				vColor = UBO.Color;
			})");
	}
	else {
		vsCode = vsCode.arg(R"(
			void main(){
				mat4 Transform = UBO.MV * inInstTransform;
				gl_Position =  UBO.P * Transform * vec4(inPosition,0.0f,1.0f);
				vColor = UBO.Color;
			})");
	}


	QShader vs = mRhi->newShaderFromCode(QShader::Stage::VertexStage, vsCode.toLocal8Bit());

	QDebugSceneRenderPass* debugPass = qobject_cast<QDebugSceneRenderPass*>(sceneRenderPass());
	QShader fs = mRhi->newShaderFromCode(QShader::Stage::FragmentStage, QString(R"(#version 440
	layout(location = 0) in vec4 vColor;
	layout(location = 0) out vec4 BaseColor;
	layout(location = 1) out vec4 DebugID;
	void main(){
		BaseColor = vColor;
		DebugID = %1;
	}
	)").arg(debugPass->getIDText(this))
		.toLocal8Bit()
	);
	mPipeline.reset(mRhi->newGraphicsPipeline());
	mPipeline->setVertexInputLayout(inputLayout);
	QVector<QRhiGraphicsPipeline::TargetBlend> targetBlends(sceneRenderPass()->getRenderTargetSlots().size());
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

