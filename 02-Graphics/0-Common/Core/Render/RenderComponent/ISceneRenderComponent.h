#ifndef ISceneRenderComponent_h__
#define ISceneRenderComponent_h__

#include "Render/IRenderComponent.h"

class ISceneRenderComponent: public IRenderComponent {
	Q_OBJECT
protected:
	QMatrix4x4 getTransform();
protected:
	Q_PROPERTY_AUTO(QVector3D, Position);
	Q_PROPERTY_AUTO(QVector3D, Rotation);
	Q_PROPERTY_AUTO(QVector3D, Scaling) = QVector3D(1, 1, 1);

};

#endif // ISceneRenderComponent_h__
