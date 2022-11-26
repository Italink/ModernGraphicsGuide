#ifndef QFrameGraph_h__
#define QFrameGraph_h__

#include "RHI/QRhiEx.h"

class IRenderer;
class IRenderPassBase;

class QFrameGraph {
	friend class QFrameGraphBuilder;
public:
	void compile(IRenderer* renderer);
	void render(QRhiCommandBuffer* cmdBuffer);
	void resize(const QSize& size);
	const QHash<QString, IRenderPassBase*>& getRenderPassMap() const { return mRenderPassMap; }
protected:
	void rebuildTopology();
private:
	QHash<QString, IRenderPassBase*> mRenderPassMap;
	QList<IRenderPassBase*> mRenderPassTopology;
};

class QFrameGraphBuilder {
public:
	static QFrameGraphBuilder* begin();
	QFrameGraphBuilder* node(const QString& inName, IRenderPassBase* inRenderPass);
	QSharedPointer<QFrameGraph> end();
private:
	QFrameGraphBuilder();

	QSharedPointer<QFrameGraph> mFrameGraph;
	QString mCurrentNodeName;
	QList<IRenderPassBase*> mRenderPassNodeList;
};

#endif // QFrameGraphBuilder_h__
