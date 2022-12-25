#ifndef ISceneRenderComponent_h__
#define ISceneRenderComponent_h__

#include "Render/IRenderComponent.h"
#include "QDetailWidgetMacros.h"

class ISceneRenderComponent: public IRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QMatrix4x4 Transform READ calculateMatrixModel WRITE setTransform)
public:
	QMatrix4x4 calculateMatrixMVP();
	QMatrix4x4 getMatrixView();
	QMatrix4x4 getMatrixClipWithCorr();
	QMatrix4x4 calculateMatrixModel();

	void setTranslate(QVector3D translate);
	void setRotation(QVector3D rotation);
	void setScale3D(QVector3D scale3D);
	void setTransform(QMatrix4x4 transform);

	QVector3D getTranslate();
	QVector3D getRotation();
	QVector3D getScale3D();
protected:
	QMatrix4x4 mTransform;
};

#endif // ISceneRenderComponent_h__
