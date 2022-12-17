#ifndef QDefaultSceneRenderPass_h__
#define QDefaultSceneRenderPass_h__

#include "Render/IRenderPass.h"

class QDefaultSceneRenderPass : public ISceneRenderPass {
	Q_OBJECT
public:
	int getSampleCount() override;
	int getBlendStateCount() override;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;

	void compile() override;
	void resize(const QSize& size) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
};

#endif // QDefaultSceneRenderPass_h__