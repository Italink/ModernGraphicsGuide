#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderComponent/QImGUIRenderComponent.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Scene", (new QSceneOutputRenderPass())
			->addRenderComponent((new QImGUIRenderComponent)
				->setupPaintFunctor([]() {
					ImGui::ShowFontSelector("Font");
					ImGui::ShowStyleSelector("Style");
					ImGui::ShowDemoWindow();
				})
			)
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

