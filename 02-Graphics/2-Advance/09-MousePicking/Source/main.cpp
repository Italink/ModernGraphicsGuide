#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/QFrameGraph.h"
#include "Render/RenderPass/QDebugSceneRenderPass.h"
#include "Render/RenderComponent/Debug/QDebugStaticMeshRenderComponent.h"
#include "Render/RenderPass/QTextureOutputRenderPass.h"
#include "Render/RenderComponent/Debug/QDebugSkeletalMeshRenderComponent.h"
#include "Render/RenderComponent/Debug/QDebugParticlesRenderComponent.h"
#include "Render/RenderComponent/Debug/QDebugSkyboxRenderComponent.h"
#include "Render/RenderPass/QPixelFilterRenderPass.h"
#include "Render/RenderPass/QBlurRenderPass.h"
#include "Render/RenderPass/QBloomMerageRenderPass.h"

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();

	widget.setupCamera()
		->setPosition(QVector3D(0,100,800));

	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("DebugScene", (new QDebugSceneRenderPass())
			->addRenderComponent((new QDebugSkyboxRenderComponent)
				->setupSkyBoxImage(QImage(RESOURCE_DIR"/Skybox.jpeg"))
			)
			->addRenderComponent((new QDebugStaticMeshRenderComponent)
				->setupStaticMeshPath(RESOURCE_DIR"/Genji/Genji.FBX")
				->setTranslate(QVector3D(-200,0,0))
				->setScale3D(QVector3D(10,10,10))
			)
			->addRenderComponent((new QDebugSkeletalMeshRenderComponent)
				->setupSkeletalMeshPath(RESOURCE_DIR"/Catwalk Walk Turn 180 Tight R.fbx")
				->setTranslate(QVector3D(200, 0, 0))
			)
			->addRenderComponent((new QDebugParticlesRenderComponent))
		)
		->addPass("BrightPixelFilter", (new QPixelFilterRenderPass)
			->setupDownSamplerCount(2)
			->setupFilterCode(R"(
				const float threshold = 1.0f;
				void main() {
					vec4 color = texture(uTexture, vUV);
					float value = max(max(color.r,color.g),color.b);
					outFragColor = (1-step(value, threshold)) * color * 100;
				}
			)")
			->setupInputTexture(QPixelFilterRenderPass::InSlot::Src, "DebugScene", QDebugSceneRenderPass::OutSlot::BaseColor)
		)
		->addPass("BloomBlur", (new QBlurRenderPass)
			->setupBlurIter(1)
			->setupInputTexture(QBlurRenderPass::InpSlot::Src, "BrightPixelFilter", QPixelFilterRenderPass::OutSlot::FilterResult)
		)
		->addPass("BloomMerage", (new QBloomMerageRenderPass)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Raw, "DebugScene", QDebugSceneRenderPass::OutSlot::BaseColor)
			->setupInputTexture(QBloomMerageRenderPass::InSlot::Blur, "BloomBlur", QBlurRenderPass::OutSlot::BlurResult)
		)
		->addPass("TextureOutput", (new QTextureOutputRenderPass)
			->addTexture("BloomMerage", QBloomMerageRenderPass::OutSlot::BloomMerageResult)
			->addTexture("DebugScene", QDebugSceneRenderPass::OutSlot::DebugUI)
		)
		->end()
		);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

