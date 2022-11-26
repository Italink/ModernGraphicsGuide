#include "ISceneRenderComponent.h"

QMatrix4x4 ISceneRenderComponent::getTransform() {
	QMatrix4x4 mat;
	mat.rotate(Rotation.x(), QVector3D(1.f, 0.f, 0.f));
	mat.rotate(Rotation.y(), QVector3D(0.f, 1.f, 0.f));
	mat.rotate(Rotation.z(), QVector3D(0.f, 0.f, 1.f));
	mat.scale(Scaling);
	mat.translate(Position);
	return mat;
}
