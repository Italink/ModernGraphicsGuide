#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>

#ifndef QT_NO_OPENGL
#include <QOffscreenSurface>
#endif

#include "QRhiToolkit.h"

class QRhiWindow :public QWindow {
public:
	struct InitParams {
		QRhi::Implementation backend = QRhi::Vulkan;
		QRhi::Flags rhiFlags;
		QRhiSwapChain::Flags swapChainFlags;
		QRhi::BeginFrameFlags beginFrameFlags;
		QRhi::EndFrameFlags endFrameFlags;
		int sampleCount = 1;
		int framesUntilTdr = -1;
		bool enableDebugLayer = true;
		bool printFPS = true;
	};
	QRhiWindow(QRhiWindow::InitParams inInitParmas);
private:
	void initPrivate();
	void renderPrivate();
protected:
	virtual void onRenderTick() {}
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
private:
	InitParams mInitParams;

	bool mRunning = false;
	bool mNotExposed = false;
	bool mNewlyExposed = false;
	bool mHasSwapChain = false;

	QElapsedTimer mFPSTimer;
	int mFrameCount;
protected:
	QScopedPointer<QRhi> mRhi;
	QScopedPointer<QRhiSwapChain> mSwapChain;
	QScopedPointer<QRhiRenderBuffer> mDSBuffer  ;
	QScopedPointer<QRhiRenderPassDescriptor> mSwapChainPassDesc;

#ifndef QT_NO_OPENGL
	QOffscreenSurface* mFallbackSurface = nullptr;
#endif
};

#endif // QRhiWindow_h__
