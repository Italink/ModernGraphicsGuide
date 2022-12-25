#ifndef QSceneOutputRenderPass_h__
#define QSceneOutputRenderPass_h__

#include "Render/IRenderPass.h"

class QSceneOutputRenderPass : public ISceneRenderPass {
	Q_OBJECT
public:
	int getSampleCount() override;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;
};

#endif // QSceneOutputRenderPass_h__0