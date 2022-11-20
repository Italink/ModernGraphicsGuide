#ifndef IRenderItem_h__
#define IRenderItem_h__

#include "RHI/QRhiEx.h"
#include "Render/IRenderPass.h"

class IRenderItem {
public:
	virtual void recreateResource() {}
	virtual void recreatePipeline() {}
	virtual void uploadResource(QRhiResourceUpdateBatch* batch) {}
	virtual void updatePrePass(QRhiCommandBuffer* cmdBuffer) {}
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;

	QRhiEx::DirtySignal bNeedRecreateResource;
	QRhiEx::DirtySignal bNeedRecreatePipeline;
protected:
	ISceneRenderPass* mScreenRenderPass = nullptr;
};

#endif // IRenderItem_h__
