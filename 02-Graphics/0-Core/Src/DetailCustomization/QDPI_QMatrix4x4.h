#ifndef QDPI_Matrix4x4_h__
#define QDPI_Matrix4x4_h__

#include "Customization\Item\QDetailWidgetPropertyItem.h"
#include "Widgets\QVectorBox.h"
#include "QMatrix4x4"

class QDPI_Matrix4x4 : public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QMatrix4x4);
	virtual QWidget* GenerateValueWidget() override;
	virtual void BuildContentAndChildren() override;
private:
	QPropertyHandler* mTranslate;
	QPropertyHandler* mRotation;
	QPropertyHandler* mScale3D;
};
#endif // QDPI_Matrix4x4_h__