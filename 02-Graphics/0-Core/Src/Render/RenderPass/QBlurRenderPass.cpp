#include "QBlurRenderPass.h"

QBlurRenderPass::QBlurRenderPass() {
	setupBlurSize(20);
	setupBlurIter(2);
}

QBlurRenderPass* QBlurRenderPass::setupBlurSize(int size) {
	if (size <= 0 || size == mBlurState.size || size >= std::size(mBlurState.weight))
		return this;
	mBlurState.size = size;
	float sum = 1, s = 1;
	mBlurState.weight[size - 1] = 1;
	for (int i = size - 2; i >= 0; i--) {
		mBlurState.weight[i] = (mBlurState.weight[i + 1] + s);
		++s;
		sum += mBlurState.weight[i] * 2;
	}
	mBlurState.weight[0] /= sum / 2;
	for (int i = 1; i < size; i++) {
		mBlurState.weight[i] /= sum;
	}
	sigUpdateBlurState.request();
	return this;
}

QBlurRenderPass* QBlurRenderPass::setupBlurIter(int val) {
	mBlurIter = val;
	return this;
}

void QBlurRenderPass::resize(const QSize& size) {
	for (int i = 0; i < 2; i++) {
		mBlurRT[i].colorAttachment.reset(mRhi->newTexture(QRhiTexture::RGBA32F, getInputTexture(InpSlot::Src)->pixelSize(), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
		mBlurRT[i].colorAttachment->create();
		mBlurRT[i].renderTarget.reset(mRhi->newTextureRenderTarget({ mBlurRT[i].colorAttachment.get() }));
	}
	renderPassDesc.reset(mBlurRT[0].renderTarget->newCompatibleRenderPassDescriptor());

	for (int i = 0; i < 2; i++) {
		mBlurRT[i].renderTarget->setRenderPassDescriptor(renderPassDesc.get());
		mBlurRT[i].renderTarget->create();
	}
}

void QBlurRenderPass::compile() {
	registerOutputTexture(OutSlot::BlurResult, "BlurResult", mBlurRT[0].colorAttachment.get());
	mSampler.reset(mRhi->newSampler(QRhiSampler::Linear,
					   QRhiSampler::Linear,
					   QRhiSampler::None,
					   QRhiSampler::ClampToEdge,
					   QRhiSampler::ClampToEdge));
	mSampler->create();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(BlurState)));
	mUniformBuffer->create();

	mPipelineH.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = false;
	mPipelineH->setTargetBlends({ blendState });
	mPipelineH->setSampleCount(1);
	mPipelineH->setDepthTest(false);
	mPipelineH->setDepthWrite(false);
	QString vsCode = R"(#version 450
layout (location = 0) out vec2 vUV;
out gl_PerVertex{
	vec4 gl_Position;
};
void main() {
	vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
	%1
}
)";
	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());
	QShader fsH = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outFragColor;
layout (binding = 0) uniform sampler2D uTexture;

layout (binding = 1 ) uniform BlurState{
	int size;
	vec4 weight[50];
}blurState;

void main(){
	vec2 tex_offset = 1.0 / textureSize(uTexture, 0); // gets size of single texel
	vec4 raw = texture(uTexture, vUV);
	vec4 result = raw * blurState.weight[0][0]; // current fragment's contribution
	for(int i = 1; i < blurState.size; ++i){
		const float weight = blurState.weight[i/4][i%4];
		result += texture(uTexture, vUV + vec2(tex_offset.x * i, 0.0)) * weight;
		result += texture(uTexture, vUV - vec2(tex_offset.x * i, 0.0)) * weight;
	}
    outFragColor = result;
}
	)");
	mPipelineH->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fsH }
								});
	QRhiVertexInputLayout inputLayout;
	mBindingsH.reset(mRhi->newShaderResourceBindings());
	mBindingsH->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[0].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
							});
	mBindingsH->create();
	mPipelineH->setVertexInputLayout(inputLayout);
	mPipelineH->setShaderResourceBindings(mBindingsH.get());
	mPipelineH->setRenderPassDescriptor(renderPassDesc.get());
	mPipelineH->create();

	//-------------------------------------------------------------

	mPipelineV.reset(mRhi->newGraphicsPipeline());
	mPipelineV->setTargetBlends({ blendState });
	mPipelineV->setSampleCount(1);
	mPipelineV->setDepthTest(false);
	mPipelineV->setDepthWrite(false);
	QShader fsV = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
layout (location = 0) in vec2 vUV;
layout (location = 0) out vec4 outFragColor;
layout (binding = 0) uniform sampler2D uTexture;

layout (binding = 1 ) uniform BlurState{
	int size;
	vec4 weight[50];
}blurState;

void main(){
	vec2 tex_offset = 1.0 / textureSize(uTexture, 0); // gets size of single texel
	vec4 raw = texture(uTexture, vUV);
	vec4 result = raw * blurState.weight[0][0]; // current fragment's contribution
	for(int i = 1; i < blurState.size; ++i){
		const float weight = blurState.weight[i/4][i%4];
		result += texture(uTexture, vUV + vec2(0.0,tex_offset.y * i)) * weight;
		result += texture(uTexture, vUV - vec2(0.0,tex_offset.y * i)) * weight;
	}
    outFragColor = result;
}
	)");
	mPipelineV->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fsV }
								});
	mBindingsV.reset(mRhi->newShaderResourceBindings());
	mBindingsV->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mBlurRT[1].colorAttachment.get(),mSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
							});

	mBindingsV->create();
	mPipelineV->setVertexInputLayout(inputLayout);
	mPipelineV->setShaderResourceBindings(mBindingsV.get());
	mPipelineV->setRenderPassDescriptor(renderPassDesc.get());
	mPipelineV->create();
	sigUpdateBlurState.request();
}

void QBlurRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiTexture* srcTexture = getInputTexture(InpSlot::Src);
	QRhiResourceUpdateBatch* copyBatch = mRhi->nextResourceUpdateBatch();
	copyBatch->copyTexture(mBlurRT[0].colorAttachment.get(), srcTexture);
	if (sigUpdateBlurState.receive()) {
		copyBatch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(BlurState), &mBlurState);
	}
	cmdBuffer->resourceUpdate(copyBatch);
	for (int i = 0; i < mBlurIter; i++) {
		cmdBuffer->beginPass(mBlurRT[1].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
		cmdBuffer->setGraphicsPipeline(mPipelineH.get());
		cmdBuffer->setShaderResources(mBindingsH.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, srcTexture->pixelSize().width(), srcTexture->pixelSize().height()));
		cmdBuffer->draw(4);
		cmdBuffer->endPass();

		cmdBuffer->beginPass(mBlurRT[0].renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
		cmdBuffer->setGraphicsPipeline(mPipelineV.get());
		cmdBuffer->setShaderResources(mBindingsV.get());
		cmdBuffer->setViewport(QRhiViewport(0, 0, srcTexture->pixelSize().width(), srcTexture->pixelSize().height()));
		cmdBuffer->draw(4);
		cmdBuffer->endPass();
	}
}
