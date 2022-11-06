#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>
#include <QOffscreenSurface>
#include "QRhiDefine.h"

class QRhiWindow :public QWindow {
public:
	QRhiWindow(QRhi::Implementation backend);
	void waitExposed();
	int getFPS() const { return mFPS; }
	bool hasClosed() const { return mHasClosed; }
	QRhiSwapChain* getSwapChain() { return mSwapChain.get(); };

	virtual void customInitEvent() {}
	virtual void customRenderEvent(QRhiSwapChain *swapchain) {}
	virtual void customResizeEvent() {}
private:
	void initInternal();
	void renderInternal();
	void resizeSwapChain();
protected:
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
protected:
	QRhi::Implementation mBackend;
	std::shared_ptr<QRhi> mRhi;
	QRhiScopedPointer<QRhiSwapChain> mSwapChain;
	QRhiScopedPointer<QRhiRenderBuffer> mDepthStencilFrameBuffer;
	QRhiScopedPointer<QRhiRenderPassDescriptor> mRenderPassDesciptor;
	std::shared_ptr<QOffscreenSurface> mFallbackSurface;
	QElapsedTimer mTimer;
	int mFrameCount = 0;
	int mFPS = 0;

	bool mRunning = false;
	bool mNotExposed = false;
	bool mNewlyExposed = false;
	bool mHasSwapChain = false;
	bool mHasClosed = false;
};

#endif // QRhiWindow_h__
