#ifndef ISceneRenderComponent_h__
#define ISceneRenderComponent_h__

#include "Render/IRenderComponent.h"
#include "QDetailWidgetMacros.h"

class ISceneRenderComponent: public IRenderComponent {
	Q_OBJECT
protected:
	QMatrix4x4 calculateMatrixMVP();
	QMatrix4x4 calculateMatrixModel();
protected:
	Q_PROPERTY_VAR(QVector3D, Position);
	Q_PROPERTY_VAR(QVector3D, Rotation);
	Q_PROPERTY_VAR(QVector3D, Scaling) = QVector3D(1, 1, 1);

};

#endif // ISceneRenderComponent_h__
