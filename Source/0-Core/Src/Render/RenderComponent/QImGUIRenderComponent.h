#ifndef QImGUIRenderComponent_h__
#define QImGUIRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/Painter/ImGuiPainter.h"

class QImGUIRenderComponent :public IRenderComponent {
	Q_OBJECT
public:
	QImGUIRenderComponent();
	QImGUIRenderComponent* setupPaintFunctor(std::function<void()> inFunctor);
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QScopedPointer<ImGuiPainter> mPainter;
};

#endif // QImGUIRenderComponent_h__
