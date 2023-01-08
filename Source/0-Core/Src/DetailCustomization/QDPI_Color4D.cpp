#include "QDPI_Color4D.h"
#include "Widget\QColor4DButton.hpp"

QWidget* QDPI_Color4D::GenerateValueWidget() {
	QColor4DButton* colorButton = new QColor4DButton(GetValue().value<QColor4D>());
	GetHandler()->Bind(colorButton, &QColor4DButton::AsColorChanged,
		[colorButton]() {
			return QVariant::fromValue<QColor4D>(colorButton->GetColor());
		},
		[colorButton](QVariant var) {
			colorButton->SetColor(var.value<QColor4D>());
		}
		);
	return colorButton;
}

