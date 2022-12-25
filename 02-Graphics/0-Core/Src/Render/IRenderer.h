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
	void requestComplie();
	virtual QRhiRenderTarget* renderTaget() = 0;
	virtual QRhiRenderPassDescriptor* renderPassDescriptor() { return renderTaget()->renderPassDescriptor();}
	virtual int sampleCount() = 0;
	virtual void resize(const QSize& size);

	void setCamera(QCamera* inCamera);
	void setFrameGraph(QSharedPointer<QFrameGraph> inFrameGraph);
	void setCurrentObject(QObject* val);

	QSize getFrameSize() const { return mFrameSize; }
	QCamera* getCamera() const { return mCamera; }
	IRenderPassBase* getRenderPassByName(const QString& inName);
	QSharedPointer<QRhiEx> getRhi() { return mRhi; }
	QObject* getCurrentObject() const { return mCurrentObject; }
Q_SIGNALS:
	void asCurrentObjectChanged(QObject*);
protected:
	void complie();
	virtual void render();
protected:
	QSharedPointer<QRhiEx> mRhi;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QCamera* mCamera;
	QSize mFrameSize;
	bool bRequestCompile = false;
	QObject* mCurrentObject = nullptr;
};

#endif // IRenderer_h__
