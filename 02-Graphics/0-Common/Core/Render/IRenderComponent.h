#ifndef IComponent_h__
#define IComponent_h__

#include "RHI\QRhiEx.h"

class IComponent {
public:
	void setupRhi(QSharedPointer<QRhiEx> inRhi) {
		mRhi = inRhi;
	}
protected:
	virtual void recreateResource() {}
	virtual void recreatePipeline() {}
	virtual void uploadResource(QRhiResourceUpdateBatch* batch) {}
	virtual void updatePrePass(QRhiCommandBuffer* cmdBuffer) {}
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) = 0;
protected:
	ISceneRenderPass* mScreenRenderPass = nullptr;
	QRhiEx::DirtySignal bNeedRecreateResource;
	QRhiEx::DirtySignal bNeedRecreatePipeline;
	QSharedPointer<QRhiEx> mRhi;
};

#endif // IComponent_h__
