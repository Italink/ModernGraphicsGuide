#include "QImGUIRenderComponent.h"
#include "Render/IRenderPass.h"
#include "Render/Renderer/QWindowRenderer.h"

QImGUIRenderComponent::QImGUIRenderComponent()
	: mPainter(new ImGuiPainter)
{
}

QImGUIRenderComponent* QImGUIRenderComponent::setupPaintFunctor(std::function<void()> inFunctor) {
	mPainter->setupPaintFunctor(inFunctor);
	return this;
}

void QImGUIRenderComponent::onRebuildResource() {
	mPainter->setupRenderPassDesc(sceneRenderPass()->getRenderPassDescriptor());
	mPainter->setupSampleCount(sceneRenderPass()->getSampleCount());
	mPainter->setupRhi(mRhi);
	if (QWindowRenderer* renderer = qobject_cast<QWindowRenderer*>(sceneRenderPass()->getRenderer())) {
		mPainter->setupWindow(renderer->getWindow());
	}
	mPainter->compile();
}

void QImGUIRenderComponent::onRebuildPipeline() {
	
}

void QImGUIRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {

}

void QImGUIRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	mPainter->resourceUpdate(batch);
}

void QImGUIRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	mPainter->paint(cmdBuffer, sceneRenderPass()->getRenderTarget());
}

bool QImGUIRenderComponent::isVaild() {
	return !mPainter.isNull();
}
