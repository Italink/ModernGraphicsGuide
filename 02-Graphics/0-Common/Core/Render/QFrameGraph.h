#ifndef QFrameGraph_h__
#define QFrameGraph_h__

#include "IRenderPass.h"

class IRenderer;

class QFrameGraphNode {
	friend class QFrameGraph;
private:
	void tryCompile();
public:
	QString mName;
	QSharedPointer<IRenderPassBase> mRenderPass;
	QList<QFrameGraphNode*> mDependencyList;
	QList<QFrameGraphNode*> mSubPassList;
	std::atomic_bool isCompiled = false;
};

class QFrameGraph {
	friend class QFrameGraphBuilder;
public:
	void compile();
	void render(QRhiCommandBuffer* cmdBuffer);
	void resize(const QSize& size);
	const QHash<QString, QSharedPointer<QFrameGraphNode>>& getGraphNodeMap() const { return mGraphNodeMap; }
private:
	QHash<QString, QSharedPointer<QFrameGraphNode>> mGraphNodeMap;
	QList<QSharedPointer<QFrameGraphNode>> mRenderQueue;
};

class QFrameGraphBuilder {
public:
	QFrameGraphBuilder(IRenderer* renderer);
	QFrameGraphBuilder* node(QString name, QSharedPointer<IRenderPassBase> renderPass);
	QFrameGraphBuilder* dependency(QStringList dependencyList);
private:
	QSharedPointer<QFrameGraph> mFrameGraph;
	QString mCurrentNodeName;
	QList<QSharedPointer<IRenderPassBase>> mRenderPassNodeList;
	IRenderer* mRenderer = nullptr;
};

#endif // QFrameGraphBuilder_h__
