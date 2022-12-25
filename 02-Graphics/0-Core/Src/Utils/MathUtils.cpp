#include "MathUtils.h"
#include "imgui.h"
#include "ImGuizmo.h"

const float ZPI = 3.14159265358979323846f;
const float RAD2DEG = (180.f / ZPI);

void MathUtils::setMatTranslate(QMatrix4x4& mat4, QVector3D newTranslate) {
	QVector3D translate;
	QVector3D rotation;
	QVector3D scale3D;
	ImGuizmo::DecomposeMatrixToComponents(mat4.constData(), (float*)&translate, (float*)&rotation, (float*)&scale3D);
	ImGuizmo::RecomposeMatrixFromComponents((float*)&newTranslate, (float*)&rotation, (float*)&scale3D, mat4.data());
}

void MathUtils::setMatRotation(QMatrix4x4& mat4, QVector3D newRotation) {
	QVector3D translate;
	QVector3D rotation;
	QVector3D scale3D;
	ImGuizmo::DecomposeMatrixToComponents(mat4.constData(), (float*)&translate, (float*)&rotation, (float*)&scale3D);
	ImGuizmo::RecomposeMatrixFromComponents((float*)&translate, (float*)&newRotation, (float*)&scale3D, mat4.data());
}

void MathUtils::setMatScale3D(QMatrix4x4& mat4, QVector3D newScale3D) {
	QVector3D translate;
	QVector3D rotation;
	QVector3D scale3D;
	ImGuizmo::DecomposeMatrixToComponents(mat4.constData(), (float*)&translate, (float*)&rotation, (float*)&scale3D);
	ImGuizmo::RecomposeMatrixFromComponents((float*)&translate, (float*)&rotation, (float*)&newScale3D, mat4.data());
}

void MathUtils::setMatOrthoNormalize(QMatrix4x4& mat4) {
	QVector4D right = mat4.row(0).normalized();
	QVector4D up = mat4.row(1).normalized();
	QVector4D dir = mat4.row(2).normalized();

	mat4.setRow(0, right);
	mat4.setRow(1, up);
	mat4.setRow(2, dir);
}

QVector3D MathUtils::getMatTranslate(const QMatrix4x4& mat4) {
	return QVector3D(mat4(0, 3), mat4(1, 3), mat4(2, 3));
}

QVector3D MathUtils::getMatRotation(const QMatrix4x4& mat4) {
	QMatrix4x4 normalMatrix = mat4;
	setMatOrthoNormalize(normalMatrix);
	QVector3D rotation;
	rotation[0] = RAD2DEG * atan2f(normalMatrix(2, 1), normalMatrix(2, 2));
	rotation[1] = RAD2DEG * atan2f(-normalMatrix(2, 0), sqrtf(normalMatrix(2, 1) * normalMatrix(2, 1) + normalMatrix(2, 2) * normalMatrix(2, 2)));
	rotation[2] = RAD2DEG * atan2f(normalMatrix(1, 0), normalMatrix(0, 0));
	return rotation;
}

QVector3D MathUtils::getMatScale3D(const QMatrix4x4& mat4) {
	return QVector3D(mat4.row(0).toVector3D().length(), mat4.row(1).toVector3D().length(), mat4.row(2).toVector3D().length());
}

