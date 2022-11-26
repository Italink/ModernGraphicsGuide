#ifndef IRenderComponent_h__
#define IRenderComponent_h__

#include "RHI\QRhiEx.h"

#define Q_PROPERTY_AUTO(Type,Name)\
    Q_PROPERTY(Type Name READ get_##Name WRITE set_##Name) \
    Type get_##Name(){ return Name; } \
    void set_##Name(Type var){ \
        Name = var;  \
		qDebug()<<"Set "<<#Name<<": "<<var; \
    } \
    Type Name

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
