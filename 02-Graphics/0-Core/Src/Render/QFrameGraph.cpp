#include "QFrameGraph.h"
#include "IRenderer.h"
#include "IRenderPass.h"
void QFrameGraph::compile(IRenderer* renderer) {
	rebuildTopology();
	for (auto& renderPass : mRenderPassTopology) {
		renderPass->setRenderer(renderer);
		renderPass->resize(renderer->renderTaget()->pixelSize());
		renderPass->compile();
	}
}

void QFrameGraph::render(QRhiCommandBuffer* cmdBuffer) {
	for (auto& renderPass : mRenderPassTopology) {
		renderPass->render(cmdBuffer);
	}
}

void QFrameGraph::resize(const QSize& size)
{
	for (auto& renderPass : mRenderPassTopology) {
		renderPass->resize(size);
	}
}

void QFrameGraph::rebuildTopology() {
	mDependMap.clear();
	for (auto& renderPass : mRenderPassMap) {
		for (auto name : renderPass->getInputRenderPassNames()) {
			mDependMap[renderPass] << mRenderPassMap.value(name);
		}
	}
	mRenderPassTopology.clear();
	while (mRenderPassTopology.size() != mRenderPassMap.size()) {
		for (auto& renderPass : mRenderPassMap) {
			bool bHasDepend = false;
			for (auto& depend : mDependMap[renderPass]) {
				if (!mRenderPassTopology.contains(depend)) {
					bHasDepend = true;
				}
			}
			if (!bHasDepend && !mRenderPassTopology.contains(renderPass)) {
				mRenderPassTopology << renderPass;
				renderPass->cleanupInputLinkerCache();
			}
		}
	}
}

QFrameGraphBuilder::QFrameGraphBuilder() {
	mFrameGraph = QSharedPointer<QFrameGraph>::create();
}

QFrameGraphBuilder* QFrameGraphBuilder::begin() {
	return new QFrameGraphBuilder;
}

QFrameGraphBuilder* QFrameGraphBuilder::addPass(const QString& inName, IRenderPassBase* inRenderPass) {
	inRenderPass->setObjectName(inName);
	mFrameGraph->mRenderPassMap[inRenderPass->objectName()] = inRenderPass;
	return this;
}

QSharedPointer<QFrameGraph> QFrameGraphBuilder::end() {
	QSharedPointer<QFrameGraph> frameGraph = mFrameGraph;
	delete this;
	return frameGraph;
}
