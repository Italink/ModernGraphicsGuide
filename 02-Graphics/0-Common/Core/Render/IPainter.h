#ifndef IPainter_h__
#define IPainter_h__

#include "RHI/QRhiToolkit.h"	

class IPainter {
public:
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
};


#endif // IPainter_h__
