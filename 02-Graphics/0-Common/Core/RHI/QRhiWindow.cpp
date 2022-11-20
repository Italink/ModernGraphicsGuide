#include "QGuiApplication"
#include "QRhiWindow.h"
#include <QPlatformSurfaceEvent>

QRhiWindow::QRhiWindow(QRhiWindow::InitParams inInitParmas)
	: mInitParams(inInitParmas)
{
	switch (mInitParams.backend) {
	case QRhi::OpenGLES2:
		setSurfaceType(OpenGLSurface);
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

void QRhiWindow::initializeInternal() {
	mRhi = QRhiEx::newRhiEx(mInitParams.backend, mInitParams.rhiFlags, this);
	if (!mRhi)
		qFatal("Failed to create RHI backend");

	// now onto the backend-independent init

	mSwapChain.reset(mRhi->newSwapChain());
	// allow depth-stencil, although we do not actually enable depth test/write for the triangle
	mDSBuffer.reset( mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
		QSize(), // no need to set the size here, due to UsedWithSwapChainOnly
		mInitParams.sampleCount,
		QRhiRenderBuffer::UsedWithSwapChainOnly));

	mSwapChain->setWindow(this);
	mSwapChain->setDepthStencil(mDSBuffer.get());
	mSwapChain->setSampleCount(mInitParams.sampleCount);
	mSwapChain->setFlags(mInitParams.swapChainFlags);
	mSwapChainPassDesc.reset( mSwapChain->newCompatibleRenderPassDescriptor());
	mSwapChain->setRenderPassDescriptor(mSwapChainPassDesc.get());
	if (mInitParams.printFPS) {
		mFrameCount = 0;
		mFPSTimer.restart();
	}
}

void QRhiWindow::renderInternal() {
	if (!mHasSwapChain || mNotExposed)
		return;

	// If the window got resized or got newly exposed, resize the swapchain.
	// (the newly-exposed case is not actually required by some
	// platforms/backends, but f.ex. Vulkan on Windows seems to need it)
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize() || mNewlyExposed) {
		resizeInternal();
		if (!mHasSwapChain)
			return;
		mNewlyExposed = false;
	}

	// Start a new frame. This is where we block when too far ahead of
	// GPU/present, and that's what throttles the thread to the refresh rate.
	// (except for OpenGL where it happens either in endFrame or somewhere else
	// depending on the GL implementation)
	QRhi::FrameOpResult r = mRhi->beginFrame(mSwapChain.get(), mInitParams.beginFrameFlags);
	if (r == QRhi::FrameOpSwapChainOutOfDate) {
		resizeInternal();
		if (!mHasSwapChain)
			return;
		if (mInitParams.printFPS) {
			mFrameCount = 0;
			mFPSTimer.restart();
		}
		r = mRhi->beginFrame(mSwapChain.get());
	}
	if (r != QRhi::FrameOpSuccess) {
		requestUpdate();
		return;
	}

	if (mInitParams.printFPS) {
		mFrameCount += 1;
		if (mFPSTimer.elapsed() > 1000) {
			qDebug("ca. %d fps", mFrameCount);
			mFPSTimer.restart();
			mFrameCount = 0;
		}
	}

	onRenderTick();

	mRhi->endFrame(mSwapChain.get(), mInitParams.endFrameFlags);

	if (!mInitParams.swapChainFlags.testFlag(QRhiSwapChain::NoVSync))
		requestUpdate();
	else // try prevent all delays when NoVSync
		QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

void QRhiWindow::resizeInternal() {
	mHasSwapChain = mSwapChain->createOrResize();
	onResizeEvent(mSwapChain->currentPixelSize());
}

void QRhiWindow::exposeEvent(QExposeEvent*)
{
	// initialize and start rendering when the window becomes usable for graphics purposes
	if (isExposed() && !mRunning) {
		mRunning = true;
		initializeInternal();
		mHasSwapChain = mSwapChain->createOrResize();
	}
	const QSize surfaceSize = mHasSwapChain ? mSwapChain->surfacePixelSize() : QSize();

	// stop pushing frames when not exposed (or size is 0)
	if ((!isExposed() || (mHasSwapChain && surfaceSize.isEmpty())) && mRunning)
		mNotExposed = true;

	// always render a frame on exposeEvent() (when exposed) in order to update
	// immediately on window resize.
	if (isExposed() && !surfaceSize.isEmpty())
		renderInternal();
}

bool QRhiWindow::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		renderInternal();
		break;
	case QEvent::PlatformSurface:
		if (static_cast<QPlatformSurfaceEvent*>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed) {
			mSwapChain.reset();
			mHasSwapChain = false;
		}
		break;
	default:
		break;
	}
	return QWindow::event(e);
}