#include "IRenderer.h"

IRenderer::IRenderer(){}

void IRenderer::buildFrameGraph() {
}

void IRenderer::render(QRhiCommandBuffer* cmdBuffer) {
	if(mFrameGraph)
		mFrameGraph->executable(cmdBuffer);
}

void IRenderer::resize(QSize size) {
	mFrameSize = size;
	if (mFrameGraph)
		mFrameGraph->compile();
}
