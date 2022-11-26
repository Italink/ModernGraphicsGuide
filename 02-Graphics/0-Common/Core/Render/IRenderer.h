#ifndef IRenderer_h__
#define IRenderer_h__

#include "QFrameGraph.h"
#include "RHI/QRhiEx.h"
#include "Utils/QCamera.h"

class IRenderer: public QObject {
	Q_OBJECT
public:
	IRenderer(QSharedPointer<QRhiEx> inRhi,const QSize& inFrameSize);
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void complie();
	virtual void render() = 0;
	virtual QRhiRenderTarget* renderTaget() = 0;
	virtual QRhiRenderPassDescriptor* renderPassDescriptor() { return renderTaget()->renderPassDescriptor();}
	virtual int sampleCount() = 0;
	virtual void resize(const QSize& size);
	QSize getFrameSize() const { return mFrameSize; }
	QCamera* getCamera() const { return mCamera.get(); }
	IRenderPassBase* getRenderPassByName(const QString& inName);
	QSharedPointer<QRhiEx> getRhi() { return mRhi; }
protected:
	QSharedPointer<QRhiEx> mRhi;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QSharedPointer<QCamera> mCamera;
	QSize mFrameSize;
};

#endif // IRenderer_h__
