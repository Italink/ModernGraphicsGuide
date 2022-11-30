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
	registerOutputTexture(0, "BaseColor", nullptr);
}

void QDefaultSceneRenderPass::resize(const QSize& size) {

}

void QDefaultSceneRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QRhiResourceUpdateBatch* resUpdateBatch = mRhi->nextResourceUpdateBatch();
	for (auto& item : mRenderComponents) {
		if(!item->isVaild())
			continue;
		if (item->sigRecreateResource.receive()) {
			item->recreateResource();
			item->uploadResource(resUpdateBatch);
		}
		if (item->sigRecreatePipeline.receive()) {
			item->recreatePipeline();
		}
		item->updateResourcePrePass(resUpdateBatch);

	}
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->updatePrePass(cmdBuffer);
	}
	cmdBuffer->beginPass(mRenderer->renderTaget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0 }, resUpdateBatch);
	QRhiViewport viewport(0, 0, mRenderer->renderTaget()->pixelSize().width(), mRenderer->renderTaget()->pixelSize().height());
	for (auto& item : mRenderComponents) {
		if (!item->isVaild()|| item->sigRecreatePipeline.peek()|| item->sigRecreateResource.peek())
			continue;
		item->renderInPass(cmdBuffer, viewport);
	}
	cmdBuffer->endPass();
}
