#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "RHI\QRhiEx.h"

class ISceneRenderPass;

class IRenderComponent: public QObject {
	friend class ISceneRenderPass;
public:
	virtual void recreateResource() {}
	virtual void recreatePipeline() {}
	virtual void uploadResource(QRhiResourceUpdateBatch* batch) {}
	virtual void updatePrePass(QRhiCommandBuffer* cmdBuffer) {}
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
	ISceneRenderPass* sceneRenderPass() { return mScreenRenderPass; }

	void requestRecreateResource(){ bNeedRecreateResource.mark(); }
	void requestRecreatePipeline(){ bNeedRecreatePipeline.mark(); }
protected:
	QSharedPointer<QRhiEx> mRhi;
protected:
	QRhiEx::DirtySignal bNeedRecreateResource;
	QRhiEx::DirtySignal bNeedRecreatePipeline;
	ISceneRenderPass* mScreenRenderPass = nullptr;
};

#endif // IRenderComponent_h__
