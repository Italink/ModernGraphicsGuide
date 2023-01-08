#ifndef QColor4D_h__
#define QColor4D_h__

#include <QVector4D>
#include <QColor>
#include <QMetaType>

class QColor4D: public QVector4D {
public:
	QColor4D(QColor color = Qt::black);
	QColor4D(QVector4D vec4);
	QColor4D(QRgb rgb);
	QColor4D(Qt::GlobalColor color);
	QColor4D(float r, float g, float b, float a = 1.0);

	ushort red()const;
	ushort green()const;
	ushort blue()const;
	ushort alpha()const;

	void setAlpha(ushort val);

	float redF() const;
	float greenF()const;
	float blueF()const;
	float alphaF()const;

	void setRedF(float val);
	void setGreenF(float val);
	void setBlueF(float val);
	void setAlphaF(float val);

	float hsvHueF();
	float hsvSaturationF();
	float valueF();

	ushort hsvSaturation();
	ushort value();

	void fromQColor(QColor color);
	QColor toQColor();
	bool isValid() ;
	QString name() ;
	QRgb rgb() ;
	QRgb rgba();

	QVector4D toHSVA();

	void fromHSVA(float h, float s, float v, float a = 1.0f);
	void fromHSVA(QVector4D hsva);

	static QColor4D fromRgbF(float r, float g, float b, float a = 1.0f);
	static QColor4D fromHsv(int h, int s, int v, int a = 255);
	static QColor4D fromHsvF(float h, float s, float v, float a = 1.0f);
	operator QColor() const {
		return QColor::fromRgbF(redF(), greenF(), blueF(), alphaF());
	}
};

Q_DECLARE_METATYPE(QColor4D);

#endif // QColor4D_h__