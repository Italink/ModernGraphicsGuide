#include "QWindowRenderer.h"
#include "RHI/QRhiWindow.h"

QWindowRenderer::QWindowRenderer(QRhiWindow* inWindow)
	: IRenderer(inWindow->mRhi,inWindow->mSwapChain->currentPixelSize())
	, mWindow(inWindow)
{
	setParent(mWindow);
}

void QWindowRenderer::render()
{
	IRenderer::render();
	mFrameGraph->render(mWindow->mSwapChain->currentFrameCommandBuffer());
}

QRhiRenderTarget* QWindowRenderer::renderTaget()
{
	return mWindow->mSwapChain->currentFrameRenderTarget();
}

int QWindowRenderer::sampleCount()
{
	return mWindow->mSwapChain->sampleCount();
}

