#ifndef RegisterEntry_h__
#define RegisterEntry_h__

#include "Customization\QDetailWidgetManager.h"
#include "DetailCustomization\QInstacneDetail_QRhiUniformBlock.h"
#include "QDPI_Color4D.h"

static void RegisterDetailCustomization() {
	QDetailWidgetManager::instance()->RegisterInstanceFilter<QInstacneDetail_QRhiUniformBlock>();
	QDetailWidgetManager::instance()->RegisterPropertyItemCreator<QDPI_Color4D>();
}



#endif // RegisterEntry_h__
