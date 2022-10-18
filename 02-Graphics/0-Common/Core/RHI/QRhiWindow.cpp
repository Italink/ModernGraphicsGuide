#include "private\qrhid3d11_p.h"
#include "private\qrhigles2_p.h"
#include "private\qrhimetal_p.h"
#include "private\qrhivulkan_p.h"
#include "QGuiApplication"
#include "QRhiWindow.h"
#include "QVulkanInstance"
#include <QPlatformSurfaceEvent>
#include <QtGui/private/qrhinull_p.h>

QRhiWindow::QRhiWindow(QRhi::Implementation backend)
	: mBackend(backend)
{
	switch (backend) {
	case QRhi::OpenGLES2:
		setSurfaceType(OpenGLSurface);
		setFormat(QRhiGles2InitParams::adjustedFormat());
		break;
	case QRhi::Vulkan:
		setSurfaceType(VulkanSurface);
		break;
	case QRhi::D3D11:
		setSurfaceType(Direct3DSurface);
		break;
	case QRhi::Metal:
		setSurfaceType(MetalSurface);
		break;
	default:
		break;
	}
}

void QRhiWindow::waitExposed()
{
	while (!mRunning) {
		QGuiApplication::processEvents();
	}
}

void QRhiWindow::initInternal()
{
	QRhi::Flags rhiFlags = QRhi::EnableProfiling;
	if (mBackend == QRhi::Null) {
		QRhiNullInitParams params;
		mRhi.reset(QRhi::create(QRhi::Null, &params, rhiFlags));
	}
	else if (mBackend == QRhi::OpenGLES2) {
		mFallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
		QRhiGles2InitParams params;
		params.fallbackSurface = mFallbackSurface.get();
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::OpenGLES2, &params, rhiFlags));
	}

	else if (mBackend == QRhi::Vulkan) {
		QRhiVulkanInitParams params;
		static QVulkanInstance vkinstance;
		if (!vkinstance.isValid()) {
			vkinstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
			vkinstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
			if (!vkinstance.create())
				qFatal("Failed to create Vulkan instance");
		}
		setVulkanInstance(&vkinstance);
		params.inst = vulkanInstance();
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::Vulkan, &params, rhiFlags));
	}
	else  if (mBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		mRhi.reset(QRhi::create(QRhi::D3D11, &params, rhiFlags));
	}
	else if (mBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		mRhi.reset(QRhi::create(QRhi::Metal, &params, rhiFlags));
	}

	mSwapChain.reset(mRhi->newSwapChain());
	mSwapChain->setWindow(this);
	mSwapChain->setSampleCount(8);
	mDepthStencilFrameBuffer.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
								   QSize(), // no need to set the size here, due to UsedWithSwapChainOnly
								   mSwapChain->sampleCount(),
								   QRhiRenderBuffer::UsedWithSwapChainOnly));
	mSwapChain->setDepthStencil(mDepthStencilFrameBuffer.get());
	mRenderPassDesciptor.reset(mSwapChain->newCompatibleRenderPassDescriptor());
	mSwapChain->setRenderPassDescriptor(mRenderPassDesciptor.get());
	customInitEvent();
	resizeSwapChain();
}

void QRhiWindow::renderInternal()
{
	if (!isExposed())
		return;
	if (!mHasSwapChain || !mSwapChain)
		return;
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize() || mNewlyExposed) {
		resizeSwapChain();
		if (!mHasSwapChain)
			return;
		mNewlyExposed = false;
	}
	customRenderEvent(mSwapChain.get());
	mFrameCount += 1;
	
	if (mTimer.elapsed() > 1000) {
		mFPS = mFrameCount;
		//qDebug() << mFPS;
		mTimer.restart();
		mFrameCount = 0;
	}

}

void QRhiWindow::resizeSwapChain()
{
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize()) {
		mHasSwapChain = mSwapChain->createOrResize();
		customResizeEvent();
	}
	mFrameCount = 0;
	mTimer.restart();
}

void QRhiWindow::exposeEvent(QExposeEvent*)
{
	if (isExposed()) {
		if (!mRunning) {
			mRunning = true;
			initInternal();
		}
		resizeSwapChain();
	}
	const QSize surfaceSize = mHasSwapChain ? mSwapChain->surfacePixelSize() : QSize();
	// stop pushing frames when not exposed (or size is 0)
	if ((!isExposed() || (mHasSwapChain && surfaceSize.isEmpty())) && mRunning && !mNotExposed) {
		qDebug("not exposed");
		mNotExposed = true;
	}

	// Continue when exposed again and the surface has a valid size. Note that
	// surfaceSize can be (0, 0) even though size() reports a valid one, hence
	// trusting surfacePixelSize() and not QWindow.
	if (isExposed() && mRunning && mNotExposed && !surfaceSize.isEmpty()) {
		qDebug("exposed again");
		mNotExposed = false;
		mNewlyExposed = true;
	}

	// always render a frame on exposeEvent() (when exposed) in order to update
	// immediately on window resize.
	//if (isExposed() && !surfaceSize.isEmpty())
	//	renderInternal();
}

bool QRhiWindow::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		renderInternal();
		break;
	case QEvent::PlatformSurface:
		// this is the proper time to tear down the swapchain (while the native window and surface are still around)
		if (static_cast<QPlatformSurfaceEvent*>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
			mHasSwapChain = false;
			mDepthStencilFrameBuffer.reset(nullptr);
			mRenderPassDesciptor.reset(nullptr);
			mSwapChain.reset(nullptr);
			mRunning = false;
			mNotExposed = false;
			mHasClosed = true;
		}
		break;
	default:
		break;
	}
	return QWindow::event(e);
}