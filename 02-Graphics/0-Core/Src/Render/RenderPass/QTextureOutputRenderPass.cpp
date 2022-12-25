#include "QTextureOutputRenderPass.h"

QTextureOutputRenderPass::QTextureOutputRenderPass() {
}

QTextureOutputRenderPass* QTextureOutputRenderPass::addTexture(const QString& inPassName, int inSlot) {
	setupInputTexture(mTexturePainters.size(), inPassName, inSlot);
	mTexturePainters << QSharedPointer<TexturePainter>::create();
	return this;
}

void QTextureOutputRenderPass::compile() {

}

void QTextureOutputRenderPass::resizeAndLink(const QSize& size, const TextureLinker& linker) {
	for (int i = 0; i < mTexturePainters.size(); i++) {
		auto painter = mTexturePainters[i];
		painter->setupRhi(mRhi);
		painter->setupSampleCount(mRenderer->sampleCount());
		painter->setupRenderPassDesc(mRenderer->renderTaget()->renderPassDescriptor());
		painter->setupTexture(linker.getInputTexture(i));
		painter->compile();
	}
}

void QTextureOutputRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	cmdBuffer->beginPass(mRenderer->renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 });
	for (auto& painter : mTexturePainters) {
		painter->paint(cmdBuffer, mRenderer->renderTaget());
	}
	cmdBuffer->endPass();
}
