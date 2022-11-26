#include "IRenderer.h"

IRenderer::IRenderer(QSharedPointer<QRhiEx> inRhi, const QSize& inFrameSize)
	: mRhi(inRhi)
	, mFrameSize(inFrameSize)
{
}

void IRenderer::setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph) {
	mFrameGraph = inFrameGraph;
}

void IRenderer::complie() {
	mFrameGraph->compile(this);
}

void IRenderer::resize(const QSize& size) {
	mFrameSize = size;
	if (mFrameGraph)
		mFrameGraph->resize(size);
}

IRenderPassBase* IRenderer::getRenderPassByName(const QString& inName) {
	return mFrameGraph->getRenderPassMap().value(inName);
}
