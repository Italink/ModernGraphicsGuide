#ifndef IRenderer_h__
#define IRenderer_h__

#include "QFrameGraph.h"
#include "RHI/QRhiToolkit.h"

class IRenderer: public QObject {
	Q_OBJECT
	friend class QRenderSystem;
public:
	IRenderer();
	virtual void buildFrameGraph();
	void render(QRhiCommandBuffer* cmdBuffer) ;
	void resize(QSize size);
	QSize getFrameSize() const { return mFrameSize; }
	std::shared_ptr<QFrameGraph> getFrameGraph() const { return mFrameGraph; }
protected:
	std::shared_ptr<QFrameGraph> mFrameGraph;
	QSize mFrameSize;
};

#endif // IRenderer_h__
