#ifndef QColor4DChannelSlider_h__
#define QColor4DChannelSlider_h__

#include "Widgets\QNumberBox.h"
#include "QLinearGradient"
#include "QLabel"

class QColor4DChannelValueBox :public QHoverWidget {
	Q_OBJECT
public:
	QColor4DChannelValueBox(float inValue){
		CreateUI();
		ConnectUI();
	}
	void SetEditEnabled(bool enable);
	bool GetEditEnabled();
	QString GetText();
	float GetVar();
	void SetVar(float var);
Q_SIGNALS:
	void AsValueChanged(float);
protected:
	void CreateUI();
	void ConnectUI();
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void paintEvent(QPaintEvent* event) override;
	virtual QSize sizeHint() const override;
private:
	float mValue;
	QLineEdit_HasFocusSignal* mLeValue;
	QLabel* mLbArrow;
	QPointF mClickPosition;
};


class QColor4DChannelSlider : public QWidget {
	Q_OBJECT
public:
	QColor4DChannelSlider(QString inName, float inDefault = 0.0f);

	void SetGradientStops(const QGradientStops& inStops);

	void SetChannelValue(float inValue);

protected:
	virtual void paintEvent(QPaintEvent* event) override;

Q_SIGNALS:
	void AsValueChanged(float);

private:
	QLabel mLbName;
	QColor4DChannelValueBox mValueBox;
	QGradientStops mGradientStops;
};


#endif // QColor4DChannelSlider_h__
