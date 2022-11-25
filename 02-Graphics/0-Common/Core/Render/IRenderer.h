#ifndef IRenderer_h__
#define IRenderer_h__

#include "QFrameGraph.h"
#include "RHI/QRhiEx.h"
#include "Utils/QCamera.h"

class IRenderer: public QObject {
	Q_OBJECT
public:
	IRenderer(QSharedPointer<QRhiEx> inRhi,const QSize& inFrameSize);

	QFrameGraphBuilder* beginFrameGraph();
	IRenderer* endFrameGraph();

	virtual void render() = 0;
	virtual QRhiRenderTarget* renderTaget() = 0;
	virtual QRhiRenderPassDescriptor* renderPassDescriptor() { return renderTaget()->renderPassDescriptor();}
	virtual int sampleCount() = 0;
	virtual void resize(const QSize& size);

	QSize getFrameSize() const { return mFrameSize; }
	QCamera* getCamera() const { return mCamera.get(); }

	template<typename _Ty>
	_Ty* newRenderPass(const QString& name) {
		_Ty* renderPass = new _Ty();
		renderPass->setObjectName(name);
		renderPass->setParent(this);
		renderPass->mRhi = mRhi;
		renderPass->mRenderer = this;
		return renderPass;
	}

protected:
	QSharedPointer<QRhiEx> mRhi;
	QSharedPointer<QFrameGraphBuilder> mFrameGraphBuilder;
	QSharedPointer<QFrameGraph> mFrameGraph;
	QSharedPointer<QCamera> mCamera;
	QSize mFrameSize;
};

#endif // IRenderer_h__
