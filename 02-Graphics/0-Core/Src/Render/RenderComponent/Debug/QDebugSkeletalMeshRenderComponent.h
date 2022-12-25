#ifndef QDebugSkeletalMeshRenderComponent_h__
#define QDebugSkeletalMeshRenderComponent_h__

#include "Render/RenderComponent/QSkeletalMeshRenderComponent.h"

class QDebugSkeletalMeshRenderComponent: public QSkeletalMeshRenderComponent {
protected:
	void onRebuildResource() override;
};

#endif // QDebugSkeletalMeshRenderComponent_h__
