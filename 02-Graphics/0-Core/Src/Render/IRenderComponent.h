#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "RHI\QRhiEx.h"

class ISceneRenderPass;

class IRenderComponent: public QObject {
	Q_OBJECT
	friend class ISceneRenderPass;
public:
	QRhiEx::Signal sigRecreateResource;
	QRhiEx::Signal sigRecreatePipeline;
public:
	virtual bool isVaild() { return true; }
	virtual void recreateResource() {}
	virtual void recreatePipeline() {}
	virtual void uploadResource(QRhiResourceUpdateBatch* batch) {}
	virtual void updatePrePass(QRhiCommandBuffer* cmdBuffer) {}
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
	ISceneRenderPass* sceneRenderPass() { return mScreenRenderPass; }

protected:
	QSharedPointer<QRhiEx> mRhi;
	ISceneRenderPass* mScreenRenderPass = nullptr;
};

#endif // IRenderComponent_h__
