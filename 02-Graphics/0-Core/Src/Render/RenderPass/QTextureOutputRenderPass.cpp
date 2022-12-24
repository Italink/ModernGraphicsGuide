#include "QTextureOutputRenderPass.h"

QTextureOutputRenderPass::QTextureOutputRenderPass() {
	mTexturePainter.reset(new TexturePainter);
}

void QTextureOutputRenderPass::compile() {
	mTexturePainter->setupRhi(mRhi);
	mTexturePainter->setupSampleCount(mRenderer->sampleCount());
	mTexturePainter->setupRenderPassDesc(mRenderer->renderTaget()->renderPassDescriptor());
	mTexturePainter->setupTexture(this->getInputTexture((int)InSlot::BaseColor));
	mTexturePainter->compile();
}

void QTextureOutputRenderPass::resize(const QSize& size) {

}

void QTextureOutputRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	cmdBuffer->beginPass(mRenderer->renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0 });
	mTexturePainter->paint(cmdBuffer, mRenderer->renderTaget());
	cmdBuffer->endPass();
}
