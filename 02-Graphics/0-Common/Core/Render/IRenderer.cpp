#include "IRenderer.h"

IRenderer::IRenderer(QSharedPointer<QRhiEx> inRhi, const QSize& inFrameSize)
	: mRhi(inRhi)
	, mFrameSize(inFrameSize)
{
}

QFrameGraphBuilder* IRenderer::beginFrameGraph()
{
	mFrameGraphBuilder = QSharedPointer<QFrameGraphBuilder>::create(this);
	return mFrameGraphBuilder.get();
}

IRenderer* IRenderer::endFrameGraph()
{
	return this;
}

void IRenderer::resize(const QSize& size) {
	mFrameSize = size;
	if (mFrameGraph)
		mFrameGraph->resize(size);
}
