#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/RenderComponent/QStaticMeshRenderComponent.h"

int main(int argc, char **argv)
{
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->node("Triangle", (new QDefaultSceneRenderPass())
			->addRenderComponent((new QStaticMeshRenderComponent)
				->setupStaticMeshPath(PROJECT_PATH"/Genji/Genji.FBX")
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

