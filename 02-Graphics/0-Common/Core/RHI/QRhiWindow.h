#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>

#include "QRhiEx.h"

class QRhiWindow :public QWindow {
	friend class QWindowRenderer;
public:
	struct InitParams {
		QRhi::Implementation backend = QRhi::Vulkan;
		QRhi::Flags rhiFlags;
		QRhiSwapChain::Flags swapChainFlags;
		QRhi::BeginFrameFlags beginFrameFlags;
		QRhi::EndFrameFlags endFrameFlags;
		int sampleCount = 1;
		bool printFPS = true;
	};
	QRhiWindow(QRhiWindow::InitParams inInitParmas);
	QSharedPointer<QRhiEx> getRhi() const { return mRhi; }
private:
	void initializeInternal();
	void renderInternal();
	void resizeInternal();
protected:
	virtual void onRenderTick() {}
	virtual void onResizeEvent(const QSize& inSize) {}
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
	QSharedPointer<QRhiEx> mRhi;
	QScopedPointer<QRhiSwapChain> mSwapChain;
	QScopedPointer<QRhiRenderBuffer> mDSBuffer  ;
	QScopedPointer<QRhiRenderPassDescriptor> mSwapChainPassDesc;
};

#endif // QRhiWindow_h__
