#include "QFrameGraph.h"
#include "QQueue"
#include "IRenderer.h"

void QFrameGraphNode::tryCompile() {
	if (isCompiled)
		return;
	bool canSetup = true;
	for (auto& dep : mDependencyList) {
		if (!dep->isCompiled) {
			canSetup = false;
		}
	}
	if (canSetup) {
		mRenderPass->compile();
		isCompiled = true;
	}
	for (int i = 0; i < mSubPassList.size(); i++) {
		mSubPassList[i]->tryCompile();
	}
}

void QFrameGraph::compile() {
	for (auto& node : mGraphNodeMap) {
		node->isCompiled = false;
	}

	for (auto& node : mGraphNodeMap) {
		node->tryCompile();
	}
}

void QFrameGraph::render(QRhiCommandBuffer* cmdBuffer) {
	for (auto& renderPass : mRenderQueue) {
		renderPass->mRenderPass->render(cmdBuffer);
	}
}

void QFrameGraph::resize(const QSize& size)
{
	for (auto& renderPass : mRenderQueue) {
		renderPass->mRenderPass->resize(size);
	}
}

QFrameGraphBuilder::QFrameGraphBuilder(IRenderer* renderer) {
	mFrameGraph = QSharedPointer<QFrameGraph>::create();
	mRenderer = renderer;
}

QFrameGraphBuilder* QFrameGraphBuilder::node(QString name, QSharedPointer<IRenderPassBase> renderPass) {
	QSharedPointer<QFrameGraphNode>& node = mFrameGraph->mGraphNodeMap[name];
	node.reset(new QFrameGraphNode);
	node->mName = name;
	node->mRenderPass = renderPass;
	node->mRenderPass->mRenderer = mRenderer;
	mCurrentNodeName = name;
	mFrameGraph->mRenderQueue << node;
	return this;
}

QFrameGraphBuilder* QFrameGraphBuilder::dependency(QStringList dependencyList) {
	Q_ASSERT(mFrameGraph->mGraphNodeMap.contains(mCurrentNodeName));
	QSharedPointer<QFrameGraphNode>& node = mFrameGraph->mGraphNodeMap[mCurrentNodeName];
	for (QString& dep : dependencyList) {
		Q_ASSERT(mFrameGraph->mGraphNodeMap.contains(mCurrentNodeName));
		QSharedPointer<QFrameGraphNode>& depNode = mFrameGraph->mGraphNodeMap[dep];
		node->mDependencyList << depNode.get();
		depNode->mSubPassList << node.get();
	}
	return this;
}
