#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
#include "Render/RenderComponent/QSkeletalMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::D3D11;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	QCamera* camera = widget.setupCamera();
	camera->setPosition(QVector3D(725.0f, 300.0f, 225.0f));
	camera->setRotation(QVector3D(-0.225f, 3.0f, 0.0f));
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent((new QSkeletalMeshRenderComponent)
				->setupSkeletalMeshPath(RESOURCE_DIR"/Catwalk Walk Turn 180 Tight R.fbx")
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}
