#ifndef QDPI_Color4D_h__
#define QDPI_Color4D_h__

#include "Customization\Item\QDetailWidgetPropertyItem.h"
#include "Utils\QColor4D.h"

class QDPI_Color4D : public QDetailWidgetPropertyItem {
public:
	Q_DETAIL_SUPPORTED_TYPES(QColor4D);
	virtual QWidget* GenerateValueWidget() override;
};
#endif // QDPI_Color4D_h__