#include "QColor4DButton.hpp"
#include <QPainter>
#include "Core\QDetailWidgetStyleManager.h"
#include "QColor4DDialog.hpp"

QColor4DButton::QColor4DButton(QColor4D color)
	: mColor(color) {
	setMinimumWidth(100);
	setFixedHeight(20);
	SetColor(color);
}

void QColor4DButton::SetColor(QColor4D color) {
	mColor = color;
	update();
}

QColor4D QColor4DButton::GetColor() const {
	return mColor;
}

void QColor4DButton::paintEvent(QPaintEvent* event) {
	QHoverWidget::paintEvent(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(Qt::NoPen);
	painter.setBrush(mColor.toQColor());
	painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 2, 2);
}

void QColor4DButton::mousePressEvent(QMouseEvent* event) {
	QHoverWidget::mousePressEvent(event);
	QRect geom = rect();
	geom.moveTopLeft(mapToGlobal(QPoint(0, 0)));
	QColor4DDialog::CreateAndShow(mColor, geom);
	QColor4DDialog::Current->setStyleSheet(QDetailWidgetStyleManager::Instance()->GetStylesheet());
	QObject::connect(QColor4DDialog::Current, &QColor4DDialog::AsColorChanged, this, [&](const QColor& color) {
		SetColor(color);
		Q_EMIT AsColorChanged(mColor);
	});
}
