#ifndef QSceneOutputRenderPass_h__
#define QSceneOutputRenderPass_h__

#include "Render/IRenderPass.h"

class QSceneOutputRenderPass : public ISceneRenderPass {
	Q_OBJECT
public:
	int getSampleCount() override;
	int getRenderTargetCount() override;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;
	void compile() override;
	void resize(const QSize& size) override;
};

#endif // QSceneOutputRenderPass_h__0