#ifndef IRenderer_h__
#define IRenderer_h__

#include "QFrameGraph.h"
#include "RHI/QRhiEx.h"
#include "Utils/QCamera.h"

class IRenderer: public QObject {
	Q_OBJECT
		Q_PROPERTY(QCamera* Camera READ getCamera WRITE setCamera)
public:
	IRenderer(QSharedPointer<QRhiEx> inRhi,const QSize& inFrameSize);

	void complie();
	virtual void render() = 0;
	virtual QRhiRenderTarget* renderTaget() = 0;
	virtual QRhiRenderPassDescriptor* renderPassDescriptor() { return renderTaget()->renderPassDescriptor();}
	virtual int sampleCount() = 0;
	virtual void resize(const QSize& size);

	void setCamera(QCamera* inCamera);
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);

	QSize getFrameSize() const { return mFrameSize; }
	QCamera* getCamera() const { return mCamera; }
	IRenderPassBase* getRenderPassByName(const QString& inName);
	QSharedPointer<QRhiEx> getRhi() { return mRhi; }
protected:
	QSharedPointer<QRhiEx> mRhi;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QCamera* mCamera;
	QSize mFrameSize;
};

#endif // IRenderer_h__
