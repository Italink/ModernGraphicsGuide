#ifndef IPainter_h__
#define IPainter_h__

#include "RHI/QRhiEx.h"	

class IPainter {
public:
	void setupRhi(QSharedPointer<QRhiEx> inRhi) {
		mRhi = inRhi;
	}
	void setupRenderPassDesc(QRhiRenderPassDescriptor* desc) {
		mRenderPassDesc = desc;
	}
	void setupSampleCount(int sampleCount) {
		mSampleCount = sampleCount;
	}

	virtual void compile() = 0;
	virtual void resourceUpdate(QRhiResourceUpdateBatch* batch) {}
	virtual void paint(QRhiCommandBuffer* cmdBuffer,QRhiRenderTarget *renderTarget) = 0;
protected:
	QRhiRenderPassDescriptor* mRenderPassDesc;
	int mSampleCount;
	QSharedPointer<QRhiEx> mRhi;
};


#endif // IPainter_h__
