#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "RHI\QRhiEx.h"

class ISceneRenderPass;

class IRenderComponent: public QObject {
	Q_OBJECT
	friend class ISceneRenderPass;
public:
	QRhiEx::Signal sigonRebuildResource;
	QRhiEx::Signal sigonRebuildPipeline;
public:
	virtual bool isVaild() { return true; }
	virtual void onRebuildResource() {}
	virtual void onRebuildPipeline() {}
	virtual void onPreUpdate(QRhiCommandBuffer* cmdBuffer) {}
	virtual void onUpload(QRhiResourceUpdateBatch* batch) {}
	virtual void onUpdate(QRhiResourceUpdateBatch* batch) {}
	virtual void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
	ISceneRenderPass* sceneRenderPass() { return mScreenRenderPass; }

protected:
	QSharedPointer<QRhiEx> mRhi;
	ISceneRenderPass* mScreenRenderPass = nullptr;
};

#endif // IRenderComponent_h__
