#include "QRhiGraphicsPipelineEx.h"
#include "Render/IRenderPass.h"

QRhiGraphicsPipelineEx::QRhiGraphicsPipelineEx(IRenderComponent* inRenderComponent)
	:mRenderComponent(inRenderComponent)
{
	mRhi = mRenderComponent->sceneRenderPass()->getRenderer()->getRhi();
	mPipeline.reset(mRhi->newGraphicsPipeline());
}

void QRhiGraphicsPipelineEx::setShaderMainCode(QRhiShaderStage::Type inStage, QByteArray inCode) {
	mStageInfos[inStage].MainCode = inCode;
}

QByteArray QRhiGraphicsPipelineEx::getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat) {
	switch (inFormat) {
	case QRhiVertexInputAttribute::Float4:
		return "vec4";
	case QRhiVertexInputAttribute::Float3:
		return "vec3";
	case QRhiVertexInputAttribute::Float2:
		return "vec2";
	case QRhiVertexInputAttribute::Float:
		return "float";
	case QRhiVertexInputAttribute::UNormByte4:
		return "vec4";
	case QRhiVertexInputAttribute::UNormByte2:
		return "vec4";
	case QRhiVertexInputAttribute::UNormByte:
		return "vec4";
	case QRhiVertexInputAttribute::UInt4:
		return "vec4";
	case QRhiVertexInputAttribute::UInt3:
		return "vec4";
	case QRhiVertexInputAttribute::UInt2:
		return "vec4";
	case QRhiVertexInputAttribute::UInt:
		return "vec4";
	case QRhiVertexInputAttribute::SInt4:
		return "vec4";
	case QRhiVertexInputAttribute::SInt3:
		return "vec4";
	case QRhiVertexInputAttribute::SInt2:
		return "vec4";
	case QRhiVertexInputAttribute::SInt:
		return "vec4";
	default:
		break;
	}
}

void QRhiGraphicsPipelineEx::setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes) {
	mInputAttributes = inInputAttributes;
	mVertexInputLayout.setAttributes(inInputAttributes.begin(), inInputAttributes.end());
}

void QRhiGraphicsPipelineEx::setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings) {
	mInputBindings = inInputBindings;
	mVertexInputLayout.setBindings(mInputBindings.begin(), mInputBindings.end());
}

QRhiUniform* QRhiGraphicsPipelineEx::addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName) {
	QSharedPointer<QRhiUniform> unifrom = QSharedPointer<QRhiUniform>::create(mRhi, inStage);
	unifrom->setObjectName(inName);
	mStageInfos[inStage].mUniformBlocks << unifrom;
	return unifrom.get();
}

QVector<QRhiCommandBuffer::VertexInput> QRhiGraphicsPipelineEx::getVertexInputs() {
	QVector<QRhiCommandBuffer::VertexInput> vertexInputs;
	for (auto& input : mInputBindings) {
		vertexInputs << QRhiCommandBuffer::VertexInput{ input.mBuffer,input.mOffset };
	}
	return vertexInputs;
}

QRhiShaderResourceBindings* QRhiGraphicsPipelineEx::getShaderResourceBindings() {
	return mShaderBindings.get();
}

void QRhiGraphicsPipelineEx::create() {
	mPipeline->setTopology(mTopology);
	mPipeline->setCullMode(mCullMode);
	mPipeline->setFrontFace(mFrontFace);
	mPipeline->setTargetBlends(mBlendStates.begin(), mBlendStates.end());
	mPipeline->setDepthTest(bEnableDepthTest);
	mPipeline->setDepthWrite(bEnableDepthWrite);
	mPipeline->setDepthOp(mDepthTestOp);
	mPipeline->setStencilTest(bEnableStencilTest);
	mPipeline->setStencilFront(mStencilFrontOp);
	mPipeline->setStencilBack(mStencilBackOp);
	mPipeline->setStencilReadMask(mStencilReadMask);
	mPipeline->setStencilWriteMask(mStencilWriteMask);
	mPipeline->setSampleCount(mRenderComponent->sceneRenderPass()->getSampleCount());
	mPipeline->setLineWidth(mLineWidth);
	mPipeline->setDepthBias(mDepthBias);
	mPipeline->setSlopeScaledDepthBias(mSlopeScaledDepthBias);
	mPipeline->setPatchControlPointCount(mPatchControlPointCount);
	mPipeline->setPolygonMode(mPolygonMode);
	mPipeline->setVertexInputLayout(mVertexInputLayout);
	mPipeline->setRenderPassDescriptor(mRenderComponent->sceneRenderPass()->getRenderPassDescriptor());

	recreateShaderBindings();
	QVector<QRhiShaderStage> stages;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {

		QShader shader = QRhiEx::newShaderFromCode((QShader::Stage)stage.first, stage.second.VersionCode + stage.second.DefineCode + stage.second.MainCode);
		stages << QRhiShaderStage(stage.first, shader);
	}
	mPipeline->setShaderStages(stages.begin(), stages.end());
	mPipeline->setShaderResourceBindings(mShaderBindings.get());
	mPipeline->create();
}

void QRhiGraphicsPipelineEx::recreateShaderBindings() {
	QString vertexInputCode;
	for (auto& input : mInputAttributes) {
		vertexInputCode = QString::asprintf("layout(location = %d) in %s %s;\n", input.location(), getInputFormatTypeName(input.format()).data(), input.mName.toLocal8Bit().data());
	}
	vertexInputCode += "out gl_PerVertex { vec4 gl_Position;}; \n";
	mStageInfos[QRhiShaderStage::Vertex].DefineCode = vertexInputCode.toLocal8Bit();

	QString uniformCode;
	QVector<QRhiShaderResourceBinding> bindings;
	int bindingOffset = 0;
	QString blockName = "UBO";
	//if (!mDataList.isEmpty()) {
	//	bindings << QRhiShaderResourceBinding::uniformBuffer(bindingOffset, QRhiShaderResourceBinding::StageFlag::FragmentStage, mUniformBlock.get());
	//	uniformCode = "layout(binding = " + QString::number(bindingOffset) + ") uniform " + blockName + "Block { \n";
	//	for (auto& param : mDataList) {
	//		uniformCode += QString("    %1 %2;\n").arg(param->getTypeName()).arg(param->name);
	//	}
	//	uniformCode += "}" + blockName + ";\n";
	//	bindingOffset++;
	//}
	//for (auto& texture : mTextureList) {
	//	bindings << QRhiShaderResourceBinding::sampledTexture(bindingOffset, QRhiShaderResourceBinding::StageFlag::FragmentStage, texture->texture.get(), mSampler.get());
	//	uniformCode += QString("layout (binding = %1) uniform %2 %3;\n").arg(bindingOffset).arg("sampler2D").arg(texture->name);
	//	bindingOffset++;
	//}
	mStageInfos[QRhiShaderStage::Fragment].DefineCode = uniformCode.toLocal8Bit();
}
