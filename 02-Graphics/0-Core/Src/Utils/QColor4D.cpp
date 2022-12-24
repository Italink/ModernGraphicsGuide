#include "QColor4D.h"

#define Q_MAX_3(a, b, c) ( ( a > b && a > c) ? a : (b > c ? b : c) )
#define Q_MIN_3(a, b, c) ( ( a < b && a < c) ? a : (b < c ? b : c) )

QColor4D::QColor4D(QColor color)
{
	fromQColor(color);
}

QColor4D::QColor4D(float r, float g, float b, float a /*= 1.0*/)
{
	setRedF(r);
	setGreenF(g);
	setBlueF(b);
	setAlphaF(a);
}

QColor4D::QColor4D(Qt::GlobalColor color)
	: QColor4D(QColor(color)) {
}

QColor4D::QColor4D(QRgb rgb)
	: QColor4D(QColor(rgb)) {
}

QColor4D::QColor4D(QVector4D vec4):QVector4D(vec4){}

ushort QColor4D::red()const
{
	return redF() * 255;
}

ushort QColor4D::green()const
{
	return greenF() * 255;
}

ushort QColor4D::blue()const
{
	return blueF() * 255;
}

ushort QColor4D::alpha() const
{
	return alphaF() * 255;
}

void QColor4D::setAlpha(ushort val)
{
	setAlphaF(val / 255.0f);
}

float QColor4D::redF()const
{
	return x();
}

float QColor4D::greenF()const
{
	return y();
}

float QColor4D::blueF()const
{
	return z();
}

float QColor4D::alphaF()const
{
	return w();
}

void QColor4D::setRedF(float val)
{
	setX(val);
}

void QColor4D::setGreenF(float val)
{
	setY(val);
}

void QColor4D::setBlueF(float val)
{
	setZ(val);
}

void QColor4D::setAlphaF(float val)
{
	setW(val);
}

float QColor4D::hsvHueF()
{
	return toHSVA().x();
}

float QColor4D::hsvSaturationF()
{
	return toHSVA().y();
}

float QColor4D::valueF()
{
	return Q_MAX_3(x(), y(), z());
}

ushort QColor4D::hsvSaturation()
{
	return hsvSaturationF() * 255;
}

ushort QColor4D::value()
{
	return valueF() * 255;
}

void QColor4D::fromQColor(QColor color)
{
	setRedF(color.redF());
	setGreenF(color.greenF());
	setBlueF(color.blueF());
	setAlphaF(color.alphaF());
}

QColor QColor4D::toQColor()
{
	float value = valueF();
	if (value > 1.0f) {
		return QColor::fromRgbF(redF() / value, greenF() / value, blueF() / value, alphaF() / value);
	}
	return QColor::fromRgbF(redF(), greenF(), blueF(), alphaF());
}

bool QColor4D::isValid() 
{
	return toQColor().isValid();
}

QString QColor4D::name() 
{
	return toQColor().name();
}

QVector4D QColor4D::toHSVA()
{
	QVector4D hsva;
	const float r = redF();
	const float g = greenF();
	const float b = blueF();
	const float max = Q_MAX_3(r, g, b);
	const float min = Q_MIN_3(r, g, b);
	const float delta = max - min;

	hsva.setZ(max);
	hsva.setW(alphaF());

	if (qFuzzyIsNull(delta)) {
		hsva.setX(1.0f);
		hsva.setY(0.0f);
	}
	else {
		float hue = 0;
		hsva.setY(delta / max);
		if (qFuzzyCompare(r, max)) {
			hue = ((g - b) / delta);
		}
		else if (qFuzzyCompare(g, max)) {
			hue = (2.0f + (b - r) / delta);
		}
		else if (qFuzzyCompare(b, max)) {
			hue = (4.0f + (r - g) / delta);
		}
		else {
			Q_ASSERT_X(false, "QColor::toHsv", "internal error");
		}
		hue *= 60.0f;
		if (hue < 0.0f)
			hue += 360.0f;
		hsva.setX(hue);
	}
	return hsva;
}

void QColor4D::fromHSVA(float h, float s, float v, float a /*= 1.0f*/)
{
	setAlphaF(a);

	float      hh, p, q, t, ff;
	long        i;
	if (s <= 0.0) {       // < is bogus, just shuts up warnings
		setRedF(v);
		setGreenF(v);
		setBlueF(v);
	}
	hh = h;
	if (hh >= 360.0) 
		hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = v * (1.0 - s);
	q = v * (1.0 - (s * ff));
	t = v * (1.0 - (s * (1.0 - ff)));

	switch (i) {
	case 0:
		setRedF(v);
		setGreenF(t);
		setBlueF(p);
		break;
	case 1:
		setRedF(q);
		setGreenF(v);
		setBlueF(p);
		break;
	case 2:
		setRedF(p);
		setGreenF(v);
		setBlueF(t);
		break;

	case 3:
		setRedF(p);
		setGreenF(q);
		setBlueF(v);
		break;
	case 4:
		setRedF(t);
		setGreenF(p);
		setBlueF(v);
		break;
	case 5:
	default:
		setRedF(v);
		setGreenF(p);
		setBlueF(q);
		break;
	}
}

void QColor4D::fromHSVA(QVector4D hsva)
{
	fromHSVA(hsva.x(), hsva.y(), hsva.z(), hsva.w());
}

QRgb QColor4D::rgb()
{
	return toQColor().rgb();
}

QRgb QColor4D::rgba() 
{
	return toQColor().rgba();
}

QColor4D QColor4D::fromRgbF(float r, float g, float b, float a)
{
	return QColor4D(r, g, b);
}

QColor4D QColor4D::fromHsv(int h, int s, int v, int a /*= 255*/)
{
	return QColor4D::fromHsvF(h / 36000.0f, s / 255.0f, v / 255.0f, a / 255.0f);
}

QColor4D QColor4D::fromHsvF(float h, float s, float v, float a)
{
	QColor4D color;
	color.fromHSVA(h, s, v, a);
	return color;
}