#ifndef IRenderItem_h__
#define IRenderItem_h__

#include "RHI/QRhiDefine.h"
#include "Render/IRenderPass.h"

class IRenderItem {
public:
	virtual void recreateResource() {}
	virtual void recreatePipeline() {}
	virtual void uploadResource(QRhiResourceUpdateBatch* batch) {}
	virtual void updatePrePass(QRhiCommandBuffer* cmdBuffer) {}
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;

	QRhiSignal bNeedRecreateResource;
	QRhiSignal bNeedRecreatePipeline;
protected:
	ISceneRenderPass* mScreenRenderPass = nullptr;
};

#endif // IRenderItem_h__
