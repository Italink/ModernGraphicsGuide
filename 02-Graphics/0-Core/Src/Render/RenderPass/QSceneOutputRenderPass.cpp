#include "QSceneOutputRenderPass.h"

int QSceneOutputRenderPass::getSampleCount() {
	return mRenderer->sampleCount();
}

int QSceneOutputRenderPass::getRenderTargetCount() {
	return 1;
}

QRhiRenderPassDescriptor* QSceneOutputRenderPass::getRenderPassDescriptor() {
	return mRenderer->renderPassDescriptor();
}

QRhiRenderTarget* QSceneOutputRenderPass::getRenderTarget() {
	return mRenderer->renderTaget();
}

void QSceneOutputRenderPass::compile() {
	registerOutputTexture(0, "BaseColor", nullptr);
}

void QSceneOutputRenderPass::resize(const QSize& size) {

}