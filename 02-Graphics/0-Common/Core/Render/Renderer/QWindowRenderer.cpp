#include "QWindowRenderer.h"
#include "RHI/QRhiWindow.h"

QWindowRenderer::QWindowRenderer(QRhiWindow* inWindow)
	: IRenderer(inWindow->mRhi,inWindow->mSwapChain->currentPixelSize())
	, mWindow(inWindow)
{

}

void QWindowRenderer::render()
{
	
}

QRhiRenderTarget* QWindowRenderer::renderTaget()
{
	return mWindow->mSwapChain->currentFrameRenderTarget();
}

int QWindowRenderer::sampleCount()
{
	return mWindow->mSwapChain->sampleCount();
}

