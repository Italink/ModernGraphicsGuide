#include "QRendererWidget.h"
#include "QBoxLayout"
#include "QDetailWidget.h"
#include "Render/Renderer/QWindowRenderer.h"
#include "QSplitter"
#include "Core/QDetailWidgetStyleManager.h"

class QInnerRhiWindow : public QRhiWindow {
public:
	QInnerRhiWindow(QRhiWindow::InitParams inInitParams) 
		:QRhiWindow(inInitParams) {
	}
	void setTickFunctor(std::function<void()> inTickFunctor) {
		mTickFunctor = inTickFunctor;
	}
	virtual void onRenderTick() override {
		mTickFunctor();
	}
private:
	std::function<void()> mTickFunctor = []() {};
};

QRendererWidget::QRendererWidget(QRhiWindow::InitParams inInitParams)
	: mRhiWindow(new QInnerRhiWindow(inInitParams))
	, mDetailWidget(new QDetailWidget(QDetailWidgetFlag::DisplayObjectTree| QDetailWidgetFlag::DisplaySearcher))
	, mCamera(new QCamera)
{
	mRhiWindow->setTickFunctor([this]() {
		if (sigRecompileRenderer.receive()) {
			if (mRenderer == nullptr) {
				mRenderer = new QWindowRenderer(mRhiWindow);
				mRenderer->setCamera(mCamera);
			}
			mRenderer->setFrameGraph(mFrameGraph);
			mRenderer->complie();
			mDetailWidget->SetInstances(mRenderer);
		}
		mRenderer->render();
	});

	QSplitter* splitter = new QSplitter;
	QWidget* Container = QWidget::createWindowContainer(mRhiWindow);
	Container->setMinimumWidth(400);
	splitter->addWidget(Container);
	splitter->addWidget(mDetailWidget);
	QHBoxLayout* hLayout = new QHBoxLayout(this);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);
	hLayout->addWidget(splitter);
	setStyleSheet(QDetailWidgetStyleManager::Instance()->GetStylesheet());
	mDetailWidget->setVisible(false);
}

QCamera* QRendererWidget::setupCamera() {
	mCamera->setupWindow(mRhiWindow);
	return mCamera;
}

void QRendererWidget::setupDetailWidget() {
	mDetailWidget->setVisible(true);
}

void QRendererWidget::setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph) {
	mFrameGraph = inFrameGraph;
	requestCompileRenderer();
}

void QRendererWidget::requestCompileRenderer() {
	sigRecompileRenderer.request();
}
