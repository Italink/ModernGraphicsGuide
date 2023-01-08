#ifndef QDebugStaticMeshRenderComponent_h__
#define QDebugStaticMeshRenderComponent_h__

#include "Render/RenderComponent/QStaticMeshRenderComponent.h"

class QDebugStaticMeshRenderComponent :public QStaticMeshRenderComponent {
protected:
	void onRebuildResource() override;
};

#endif // QDebugStaticMeshRenderComponent_h__