#ifndef QRendererWidget_h__
#define QRendererWidget_h__

#include "QWidget"
#include "RHI/QRhiWindow.h"

class QCamera;
class QDetailWidget;
class QFrameGraph;
class QInnerRhiWindow;
class QWindowRenderer;

class QRendererWidget :public QWidget {
public:
	QRendererWidget(QRhiWindow::InitParams inInitParams);
	void setupCamera();
	void setupDetailWidget();
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void requestCompileRenderer();
protected:
	QRhiWindow::InitParams mInitParams;
	QInnerRhiWindow* mRhiWindow = nullptr;
	QDetailWidget* mDetailWidget = nullptr;
	QWindowRenderer* mRenderer = nullptr;
	QSharedPointer<QCamera> mCamera;
	QSharedPointer<QFrameGraph> mFrameGraph;

	QRhiEx::DirtySignal bNeedRecompileRenderer;
};

#endif // QRendererWidget_h__
