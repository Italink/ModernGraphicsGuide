#include "QDPI_QMatrix4x4.h"
#include "Core\QPropertyHandler.h"
#include "Utils\MathUtils.h"

QWidget* QDPI_Matrix4x4::GenerateValueWidget() {
	return nullptr;
}

void QDPI_Matrix4x4::BuildContentAndChildren() {
	QDetailWidgetPropertyItem::BuildContentAndChildren();
	mTranslate = QPropertyHandler::FindOrCreate(
		GetInstance()->GetOuterObject(),
		QMetaType::fromType<QVector3D>(),
		GetHandler()->GetPath() + ".Translate",
		[ParentHandle = GetHandler()](){
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatTranslate(mat4); 
		},
		[ParentHandle = GetHandler()](QVariant var) {
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			QVector3D newVar = var.value<QVector3D>();
			MathUtils::setMatTranslate(mat4, newVar);
			ParentHandle->SetValue(mat4);
		},
		GetHandler()->GetMetaData()
	);

	mRotation = QPropertyHandler::FindOrCreate(
		GetInstance()->GetOuterObject(),
		QMetaType::fromType<QVector3D>(),
		GetHandler()->GetPath() + ".Rotation",
		[ParentHandle = GetHandler()]() {
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatRotation(mat4);
		},
		[ParentHandle = GetHandler()](QVariant var) {
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			QVector3D newVar = var.value<QVector3D>();
			MathUtils::setMatRotation(mat4, newVar);
			ParentHandle->SetValue(mat4);
		},
		GetHandler()->GetMetaData()
	);

	mScale3D = QPropertyHandler::FindOrCreate(
		GetInstance()->GetOuterObject(),
		QMetaType::fromType<QVector3D>(),
		GetHandler()->GetPath() + ".Scale",
		[ParentHandle = GetHandler()]() {
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			return MathUtils::getMatScale3D(mat4);
		},
		[ParentHandle = GetHandler()](QVariant var) {
			QMatrix4x4 mat4 = ParentHandle->GetValue().value<QMatrix4x4>();
			QVector3D newVar = var.value<QVector3D>();
			MathUtils::setMatScale3D(mat4, newVar);
			ParentHandle->SetValue(mat4);
		},
		GetHandler()->GetMetaData()
		);

	QDetailWidgetPropertyItem::Create(mTranslate, GetInstance())->AttachTo(this);
	QDetailWidgetPropertyItem::Create(mRotation, GetInstance())->AttachTo(this);
	QDetailWidgetPropertyItem::Create(mScale3D, GetInstance())->AttachTo(this);
}

