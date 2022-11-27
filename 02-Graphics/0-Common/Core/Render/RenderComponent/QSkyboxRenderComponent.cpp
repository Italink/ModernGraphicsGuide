#include "QSkyboxRenderComponent.h"
#include "Render/IRenderPass.h"

static float CubeData[] = { // Y up, front = CCW
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
};

QImage QSkyboxRenderComponent::getSkyBoxImage() const {
	return mSkyBoxImage;
}

QSkyboxRenderComponent::QSkyboxRenderComponent()
{
	Scaling = QVector3D(2000, 2000, 2000);
}

QSkyboxRenderComponent* QSkyboxRenderComponent::setupSkyBoxImage(QImage inImage) {
	mSkyBoxImage = inImage;
	inImage = inImage.convertToFormat(QImage::Format::Format_RGBA8888);
	QSize mCubeFaceSize;
	if (inImage.width() == inImage.height())
		mCubeFaceSize = inImage.size();
	else if (inImage.width() * 3 == 4 * inImage.height())
		mCubeFaceSize = QSize(inImage.width() / 4, inImage.width() / 4);
	else if (inImage.width() * 4 == 3 * inImage.height())
		mCubeFaceSize = QSize(inImage.width() / 3, inImage.width() / 3);
	else
		return this;
	mSubImageList[0] = inImage.copy(QRect(QPoint(2 * mCubeFaceSize.width(), mCubeFaceSize.width()), mCubeFaceSize));
	mSubImageList[1] = inImage.copy(QRect(QPoint(0, mCubeFaceSize.width()), mCubeFaceSize));

	mSubImageList[2] = inImage.copy(QRect(QPoint(mCubeFaceSize.width(), 0), mCubeFaceSize));
	mSubImageList[3] = inImage.copy(QRect(QPoint(mCubeFaceSize.width(), mCubeFaceSize.width() * 2), mCubeFaceSize));

	mSubImageList[4] = inImage.copy(QRect(QPoint(mCubeFaceSize.width(), mCubeFaceSize.width()), mCubeFaceSize));
	mSubImageList[5] = inImage.copy(QRect(QPoint(3 * mCubeFaceSize.width(), mCubeFaceSize.width()), mCubeFaceSize));

	requestRecreateResource();
	requestRecreatePipeline();
	return this;
}

void QSkyboxRenderComponent::recreateResource() {
	mTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8, mSubImageList.front().size(), 1,
		QRhiTexture::CubeMap
		| QRhiTexture::MipMapped
		| QRhiTexture::UsedWithGenerateMips));
	mTexture->create();
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::Repeat,
		QRhiSampler::Repeat));
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QMatrix4x4)));
	mUniformBuffer->create();

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(CubeData)));
	mVertexBuffer->create();
}

void QSkyboxRenderComponent::recreatePipeline() {
	mPipeline.reset(mRhi->newGraphicsPipeline());
	QVector<QRhiGraphicsPipeline::TargetBlend> blendStates(sceneRenderPass()->getBlendStateCount());
	mPipeline->setTargetBlends(blendStates.begin(), blendStates.end());
	mPipeline->setTopology(QRhiGraphicsPipeline::Triangles);
	mPipeline->setDepthTest(true);
	mPipeline->setDepthWrite(true);
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

	QString fragShaderCode = QString(R"(#version 440
	layout(location = 0) in vec3 vPosition;
	layout(location = 0) out vec4 outColor;
	layout(binding = 1) uniform samplerCube uSkybox;
	void main(){
		outColor = texture(uSkybox,vPosition);
	}
	)");
	
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

void QSkyboxRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), CubeData);

	QRhiTextureSubresourceUploadDescription subresDesc[6];
	for (int i = 0; i < std::size(mSubImageList); i++) {
		subresDesc[i].setImage(mSubImageList[i]);
	}

	QRhiTextureUploadDescription desc({
										  { 0, 0, subresDesc[0] },  // +X
										  { 1, 0, subresDesc[1] },  // -X
										  { 2, 0, subresDesc[2] },  // +Y
										  { 3, 0, subresDesc[3] },  // -Y
										  { 4, 0, subresDesc[4] },  // +Z
										  { 5, 0, subresDesc[5] }   // -Z
		});
	batch->uploadTexture(mTexture.get(), desc);
	batch->generateMips(mTexture.get());
}

void QSkyboxRenderComponent::updatePrePass(QRhiCommandBuffer* cmdBuffer) {

}

void QSkyboxRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
	QMatrix4x4 MVP = calculateMatrixMVP();
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(QMatrix4x4), MVP.constData());
}

void QSkyboxRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	cmdBuffer->setGraphicsPipeline(mPipeline.get());
	cmdBuffer->setViewport(viewport);
	cmdBuffer->setShaderResources();
	const QRhiCommandBuffer::VertexInput VertexInput(mVertexBuffer.get(), 0);
	cmdBuffer->setVertexInput(0, 1, &VertexInput, mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
	cmdBuffer->draw(36);
}

bool QSkyboxRenderComponent::isVaild() {
	return !mSkyBoxImage.isNull()&&!mSubImageList.front().isNull();
}
