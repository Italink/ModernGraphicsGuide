#include "private\qrhigles2_p.h"
#include "private\qrhivulkan_p.h"

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <QtGui/private/qrhimetal_p.h>
#endif

#include "QGuiApplication"
#include "QRhiWindow.h"
#include "QVulkanInstance"
#include <QPlatformSurfaceEvent>
#include <QtGui/private/qrhinull_p.h>


QRhiWindow::QRhiWindow(QRhiWindow::InitParams inInitParmas) {
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

void QRhiWindow::initPrivate() {
	if (mInitParams.backend == QRhi::Null) {
		QRhiNullInitParams params;
		mRhi.reset(QRhi::create(QRhi::Null, &params, mInitParams.rhiFlags));
	}

#ifndef QT_NO_OPENGL
	if (mInitParams.backend == QRhi::OpenGLES2) {
		mFallbackSurface = QRhiGles2InitParams::newFallbackSurface();
		QRhiGles2InitParams params;
		params.fallbackSurface = mFallbackSurface;
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::OpenGLES2, &params, mInitParams.rhiFlags));
	}
#endif

#if QT_CONFIG(vulkan)
	if (mInitParams.backend == QRhi::Vulkan) {
		static QVulkanInstance vkInstance;
		if (!vkInstance.isValid()) {
			vkInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
			vkInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
			if (!vkInstance.create())
				qFatal("Failed to create Vulkan instance");
		}
		setVulkanInstance(&vkInstance);
		QRhiVulkanInitParams params;
		params.inst = vulkanInstance();
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::Vulkan, &params, mInitParams.rhiFlags));
	}
#endif

#ifdef Q_OS_WIN
	if (mInitParams.backend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = mInitParams.enableDebugLayer;
		if (mInitParams.framesUntilTdr > 0) {
			params.framesUntilKillingDeviceViaTdr = mInitParams.framesUntilTdr;
			params.repeatDeviceKill = true;
		}
		mRhi.reset(QRhi::create(QRhi::D3D11, &params, mInitParams.rhiFlags));
	}
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if (mInitParams.backend == QRhi::Metal) {
		QRhiMetalInitParams params;
		mRhi.reset(QRhi::create(QRhi::Metal, &params, mInitParams.rhiFlags));
	}
#endif

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

void QRhiWindow::renderPrivate() {
	if (!mHasSwapChain || mNotExposed)
		return;

	// If the window got resized or got newly exposed, resize the swapchain.
	// (the newly-exposed case is not actually required by some
	// platforms/backends, but f.ex. Vulkan on Windows seems to need it)
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize() || mNewlyExposed) {
		mHasSwapChain = mSwapChain->createOrResize();
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
		mHasSwapChain = mSwapChain->createOrResize();
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

void QRhiWindow::exposeEvent(QExposeEvent*)
{
	// initialize and start rendering when the window becomes usable for graphics purposes
	if (isExposed() && !mRunning) {
		mRunning = true;
		initPrivate();
		mHasSwapChain = mSwapChain->createOrResize();
	}
	const QSize surfaceSize = mHasSwapChain ? mSwapChain->surfacePixelSize() : QSize();

	// stop pushing frames when not exposed (or size is 0)
	if ((!isExposed() || (mHasSwapChain && surfaceSize.isEmpty())) && mRunning)
		mNotExposed = true;

	// always render a frame on exposeEvent() (when exposed) in order to update
	// immediately on window resize.
	if (isExposed() && !surfaceSize.isEmpty())
		renderPrivate();
}

bool QRhiWindow::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		renderPrivate();
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