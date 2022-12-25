#ifndef QDebugParticlesRenderComponent_h__
#define QDebugParticlesRenderComponent_h__

#include "Render/RenderComponent/QParticlesRenderComponent.h"

class QDebugParticlesRenderComponent : public QParticlesRenderComponent {
protected:
	void onRebuildPipeline() override;
};

#endif // QDebugParticlesRenderComponent_h__
