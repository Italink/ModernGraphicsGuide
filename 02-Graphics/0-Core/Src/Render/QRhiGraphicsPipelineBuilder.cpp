#include "QRhiGraphicsPipelineBuilder.h"
#include "Render/IRenderPass.h"

void QRhiGraphicsPipelineBuilder::setShaderMainCode(QRhiShaderStage::Type inStage, QByteArray inCode) {
	mStageInfos[inStage].MainCode = inCode;
}

QByteArray QRhiGraphicsPipelineBuilder::getInputFormatTypeName(QRhiVertexInputAttribute::Format inFormat) {
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
	return "unknown";
}

QByteArray QRhiGraphicsPipelineBuilder::getOutputFormatTypeName(QRhiTexture::Format inFormat) {
	switch (inFormat) {
	case QRhiTexture::RGBA8:
	case QRhiTexture::BGRA8:
	case QRhiTexture::RGBA16F:
	case QRhiTexture::RGBA32F:
	case QRhiTexture::RGB10A2:
		return "vec4";
	case QRhiTexture::R8:
	case QRhiTexture::R16:
	case QRhiTexture::RED_OR_ALPHA8:
	case QRhiTexture::R16F:
	case QRhiTexture::R32F:
		return "float";
	case QRhiTexture::RG8:
	case QRhiTexture::RG16:
		return "vec2";
	}
	return "unknown";
}

void QRhiGraphicsPipelineBuilder::setInputAttribute(QVector<QRhiVertexInputAttributeEx> inInputAttributes) {
	mInputAttributes = inInputAttributes;
	mVertexInputLayout.setAttributes(inInputAttributes.begin(), inInputAttributes.end());
}

void QRhiGraphicsPipelineBuilder::setInputBindings(QVector<QRhiVertexInputBindingEx> inInputBindings) {
	mInputBindings = inInputBindings;
	mVertexInputLayout.setBindings(mInputBindings.begin(), mInputBindings.end());
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, const QString& inName) {
	QSharedPointer<QRhiUniformBlock> unifrom = QSharedPointer<QRhiUniformBlock>::create(inStage);
	unifrom->setObjectName(inName);
	mStageInfos[inStage].mUniformBlocks << unifrom;
	mUniformMap[inName] = unifrom.get();
	sigRebuild.request();
	return unifrom.get();
}

void QRhiGraphicsPipelineBuilder::addUniformBlock(QRhiShaderStage::Type inStage, QSharedPointer<QRhiUniformBlock> inUniformBlock) {
	mStageInfos[inStage].mUniformBlocks << inUniformBlock;
	mUniformMap[inUniformBlock->objectName()] = inUniformBlock.get();
	sigRebuild.request();
}

QRhiUniformBlock* QRhiGraphicsPipelineBuilder::getUniformBlock(const QString& inName) {
	return mUniformMap.value(inName);
}

QVector<QRhiCommandBuffer::VertexInput> QRhiGraphicsPipelineBuilder::getVertexInputs() {
	QVector<QRhiCommandBuffer::VertexInput> vertexInputs;
	for (auto& input : mInputBindings) {
		vertexInputs << QRhiCommandBuffer::VertexInput{ input.mBuffer,input.mOffset };
	}
	return vertexInputs;
}

QRhiShaderResourceBindings* QRhiGraphicsPipelineBuilder::getShaderResourceBindings() {
	return mShaderBindings.get();
}

void QRhiGraphicsPipelineBuilder::create(IRenderComponent* inRenderComponent) {
	QRhiEx* rhi = inRenderComponent->sceneRenderPass()->getRenderer()->getRhi().get();
	mPipeline.reset(rhi->newGraphicsPipeline());
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
	mPipeline->setSampleCount(inRenderComponent->sceneRenderPass()->getSampleCount());
	mPipeline->setLineWidth(mLineWidth);
	mPipeline->setDepthBias(mDepthBias);
	mPipeline->setSlopeScaledDepthBias(mSlopeScaledDepthBias);
	mPipeline->setPatchControlPointCount(mPatchControlPointCount);
	mPipeline->setPolygonMode(mPolygonMode);
	mPipeline->setVertexInputLayout(mVertexInputLayout);
	mPipeline->setRenderPassDescriptor(inRenderComponent->sceneRenderPass()->getRenderPassDescriptor());

	recreateShaderBindings(inRenderComponent, rhi);

	QVector<QRhiShaderStage> stages;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QShader shader = QRhiEx::newShaderFromCode((QShader::Stage)stage.first, stage.second.VersionCode + stage.second.DefineCode + stage.second.MainCode);
		stages << QRhiShaderStage(stage.first, shader);
	}
	mPipeline->setShaderStages(stages.begin(), stages.end());
	mPipeline->setShaderResourceBindings(mShaderBindings.get());
	mPipeline->create();
	for (const auto& stage : mStageInfos) {
		for (const auto& uniformBlock : stage.mUniformBlocks) {
			uniformBlock->sigRecreateBuffer.receive();
		}
	}
}

void QRhiGraphicsPipelineBuilder::update(QRhiResourceUpdateBatch* batch) {
	for (const auto& stage : mStageInfos) {
		for (const auto& uniformBlock : stage.mUniformBlocks) {
			if (uniformBlock->sigRecreateBuffer.receive()) {
			}
			uniformBlock->updateResource(batch);
		}
	}
}

void QRhiGraphicsPipelineBuilder::recreateShaderBindings(IRenderComponent* inRenderComponent, QRhiEx* inRhi) {
	for (auto& stage : mStageInfos) {
		stage.DefineCode = QByteArray();
	}

	QString vertexInputCode;
	for (auto& input : mInputAttributes) {
		vertexInputCode += QString::asprintf("layout(location = %d) in %s %s;\n", input.location(), getInputFormatTypeName(input.format()).data(), input.mName.toLocal8Bit().data());
	}
	vertexInputCode += "out gl_PerVertex { vec4 gl_Position;}; \n";
	mStageInfos[QRhiShaderStage::Vertex].DefineCode = vertexInputCode.toLocal8Bit();

	QVector<QRhiShaderResourceBinding> bindings;
	int bindingOffset = 0;
	for (const auto& stage : mStageInfos.asKeyValueRange()) {
		QString uniformDefineCode;
		for (const auto& uniformBlock : stage.second.mUniformBlocks) {
			if (!uniformBlock->isEmpty()) {
				uniformBlock->create(inRhi);
				bindings << QRhiShaderResourceBinding::uniformBuffer(bindingOffset, (QRhiShaderResourceBinding::StageFlag)(1 << (int)stage.first), uniformBlock->getUniformBlock());
				uniformDefineCode += QString("layout(binding =  %1) uniform %2Block{\n").arg(bindingOffset).arg(uniformBlock->objectName());
				for (auto& param : uniformBlock->getParamList()) {
					uniformDefineCode += QString("    %1 %2;\n").arg(param->getTypeName()).arg(param->name);
				}
				uniformDefineCode += QString::asprintf("}%s;\n", uniformBlock->objectName().toLocal8Bit().data());
				bindingOffset++;
			}
		}
		stage.second.DefineCode += uniformDefineCode.toLocal8Bit();
	}
	QString fragOutputCode;
	auto outputTextures = inRenderComponent->sceneRenderPass()->getOutputTextures();
	for (int i = 0; i < outputTextures.size(); i++) {
		QRhiTexture* texture = outputTextures[i];
		QByteArray textureType = "vec4";
		QByteArray textureName = "FragColor";
		if (texture) {
			textureType = getOutputFormatTypeName(texture->format());
			textureName = texture->name();
		}
		fragOutputCode += QString::asprintf("layout(location = %d) out %s %s;\n", i, textureType.data(), textureName.data());
	}

	mStageInfos[QRhiShaderStage::Fragment].DefineCode += fragOutputCode.toLocal8Bit();

	mShaderBindings.reset(inRhi->newShaderResourceBindings());
	mShaderBindings->setBindings(bindings.begin(), bindings.end());
	mShaderBindings->create();
}