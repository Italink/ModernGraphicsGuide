#include "QDefaultSceneRenderPass.h"

int QDefaultSceneRenderPass::getSampleCount() {
	return mRenderer->sampleCount();
}

int QDefaultSceneRenderPass::getBlendStateCount() {
	return 1;
}

QRhiRenderPassDescriptor* QDefaultSceneRenderPass::getRenderPassDescriptor() {
	return mRenderer->renderPassDescriptor();
}

void QDefaultSceneRenderPass::compile() {

}

void QDefaultSceneRenderPass::resize(const QSize& size) {

}

void QDefaultSceneRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* resUpdateBatch = mRhi->nextResourceUpdateBatch();
	for (auto& item : mRenderComponents) {
		if(!item->isVaild())
			continue;
		if (needRecreateResource(item)) {
			item->recreateResource();
			item->uploadResource(resUpdateBatch);
		}
		item->updateResourcePrePass(resUpdateBatch);
		if (needRecreatePipeline(item)) {
			item->recreatePipeline();
		}
	}
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->updatePrePass(cmdBuffer);
	}
	cmdBuffer->beginPass(mRenderer->renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0 }, resUpdateBatch);
	QRhiViewport viewport(0, 0, mRenderer->renderTaget()->pixelSize().width(), mRenderer->renderTaget()->pixelSize().height());
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		if (needRecreatePipeline(item)) {
			item->recreatePipeline();
		}
		item->renderInPass(cmdBuffer, viewport);
	}
	cmdBuffer->endPass();
}
