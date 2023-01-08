#ifndef QColor4DButton_h__
#define QColor4DButton_h__

#include "Widgets\Toolkits\QHoverWidget.h"
#include "Utils\QColor4D.h"

class QColor4DButton :public QHoverWidget {
	Q_OBJECT
public:
	QColor4DButton(QColor4D color);
	void SetColor(QColor4D color);
	QColor4D GetColor() const;
Q_SIGNALS:
	void AsColorChanged(QColor4D color);
protected:
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
private:
	QColor4D mColor;
};

#endif // QColor4DButton_h__
