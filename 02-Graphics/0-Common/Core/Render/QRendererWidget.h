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
	QRhiEx::Signal sigRecompileRenderer;
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
	QCamera* mCamera;
	QSharedPointer<QFrameGraph> mFrameGraph;
};

#endif // QRendererWidget_h__
