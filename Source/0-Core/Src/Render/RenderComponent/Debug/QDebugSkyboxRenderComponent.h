#ifndef QDebugSkyboxRenderComponent_h__
#define QDebugSkyboxRenderComponent_h__

#include "Render/RenderComponent/QSkyboxRenderComponent.h"

class QDebugSkyboxRenderComponent :public QSkyboxRenderComponent {
protected:
	void onRebuildPipeline() override;
};

#endif // QDebugSkyboxRenderComponent_h__
