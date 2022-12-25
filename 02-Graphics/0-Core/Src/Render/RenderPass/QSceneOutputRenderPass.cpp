#include "QSceneOutputRenderPass.h"

int QSceneOutputRenderPass::getSampleCount() {
	return mRenderer->sampleCount();
}

QRhiRenderPassDescriptor* QSceneOutputRenderPass::getRenderPassDescriptor() {
	return mRenderer->renderPassDescriptor();
}

QRhiRenderTarget* QSceneOutputRenderPass::getRenderTarget() {
	return mRenderer->renderTaget();
}
