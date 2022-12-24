#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
#include "Render/RenderComponent/QSkyboxRenderComponent.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::D3D11;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent((new QSkyboxRenderComponent())
				->setupSkyBoxImage(QImage(PROJECT_PATH"/Skybox.jpeg"))
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#include "main.moc"