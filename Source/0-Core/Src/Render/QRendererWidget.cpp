#include "QRendererWidget.h"
#include "QBoxLayout"
#include "QDetailWidget.h"
#include "Render/Renderer/QWindowRenderer.h"
#include "QSplitter"
#include "Core/QDetailWidgetStyleManager.h"
#include "Customization/Instance/QInstanceDetail_QObject.h"

class QInnerRhiWindow : public QRhiWindow {
public:
	QInnerRhiWindow(QRhiWindow::InitParams inInitParams) 
		:QRhiWindow(inInitParams) {
	}
	void setTickFunctor(std::function<void()> inFunctor) {
		mTickFunctor = inFunctor;
	}
	void setResizeFunctor(std::function<void(const QSize&)> inFunctor) {
		mResizeFunctor = inFunctor;
	}
	void onRenderTick() override {
		mTickFunctor();
	}
	void onResizeEvent(const QSize& inSize) {
		mResizeFunctor(inSize);
	}
private:
	std::function<void()> mTickFunctor = []() {};
	std::function<void(const QSize&)> mResizeFunctor = [](const QSize&) {};
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
				connect(mRenderer, &IRenderer::asCurrentObjectChanged, this, [this](QObject* object) {
					mDetailWidget->SetSelectedInstance(QInstance::CreateObjcet(object));
				});
				connect(mDetailWidget, &QDetailWidget::AsSelectedInstanceChanged, this, [this](QSharedPointer<QInstance> instance) {
					if (mRenderer->getCurrentObject() != instance->GetOuterObject()) {
						mRenderer->setCurrentObject(instance->GetOuterObject());
					}
				});
			}
			mRenderer->setFrameGraph(mFrameGraph);
			mRenderer->complie();
			mDetailWidget->SetInstances(mRenderer);
		}
		mRenderer->render();
	});
	mRhiWindow->setResizeFunctor([this](const QSize& inSize) {
		if (mRenderer) {
			mRenderer->resize(inSize);
		}
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
