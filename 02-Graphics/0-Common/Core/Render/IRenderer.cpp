#include "IRenderer.h"

IRenderer::IRenderer(QSharedPointer<QRhiEx> inRhi, const QSize& inFrameSize)
	: mRhi(inRhi)
	, mFrameSize(inFrameSize)
{
}

void IRenderer::complie() {
	mFrameGraph->compile(this);
}

void IRenderer::setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph) {
	mFrameGraph = inFrameGraph;
}


void IRenderer::resize(const QSize& size) {
	mFrameSize = size;
	if (mFrameGraph)
		mFrameGraph->resize(size);
}

void IRenderer::setCamera(QSharedPointer<QCamera> inCamera) {
	mCamera = inCamera;
}

IRenderPassBase* IRenderer::getRenderPassByName(const QString& inName) {
	return mFrameGraph->getRenderPassMap().value(inName);
}
