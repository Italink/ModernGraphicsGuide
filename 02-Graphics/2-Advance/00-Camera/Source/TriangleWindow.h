#ifndef TriangleWindow_h__
#define TriangleWindow_h__

#include "RHI/QRhiWindow.h"
#include "Render/Renderer/QWindowRenderer.h"

class TriangleWindow : public QRhiWindow {
private:
	QScopedPointer<QWindowRenderer> mRenderer;
	QRhiEx::DirtySignal bNeedInitRenderer;
public:
	TriangleWindow(QRhiWindow::InitParams inInitParams) 
		: QRhiWindow(inInitParams) {
		bNeedInitRenderer.mark();
	}

protected:
	virtual void onRenderTick() override {
		if (bNeedInitRenderer.handle()) {
			initRenderer();
		}
		mRenderer->render();
	}

	void initRenderer() {
		mRenderer.reset(new QWindowRenderer(this));
		mRenderer->beginFrameGraph()
			->node
	}

};

#endif // TriangleWindow_h__
