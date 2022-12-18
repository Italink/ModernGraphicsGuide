#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/RenderComponent/QSkeletalMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	QCamera* camera = widget.setupCamera();
	camera->setPosition(QVector3D(725.0f, 300.0f, 225.0f));
	camera->setRotation(QVector3D(-0.225f, 3.0f, 0.0f));
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->node("Triangle", (new QDefaultSceneRenderPass())
			->addRenderComponent((new QSkeletalMeshRenderComponent)
				->setupStaticMeshPath(PROJECT_PATH"/Catwalk Walk Turn 180 Tight R.fbx")
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}
