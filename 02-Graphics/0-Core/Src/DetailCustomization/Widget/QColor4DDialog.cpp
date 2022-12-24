#include "QColor4DDialog.hpp"
#include "QBoxLayout"
#include "Widgets\Color\Component\QColorPicker.h"

QColor4DDialog::QColor4DDialog()
	: mColorWheel(new ColorWheel)
	, mColorPreview(new ColorPreview)
	, mRedBox(new QColor4DChannelSlider("R"))
	, mGreenBox(new QColor4DChannelSlider("G"))
	, mBlueBox(new QColor4DChannelSlider("B"))
	, mAlphaBox(new QColor4DChannelSlider("A"))
	, mHueBox(new QColor4DChannelSlider("H"))
	, mSaturationBox(new QColor4DChannelSlider("S"))
	, mValueBox(new QColor4DChannelSlider("V"))
	, mPbPick(new QPushButton("Pick"))
	, mPbOk(new QPushButton("OK"))
	, mPbCancel(new QPushButton("Cancel"))
	, mLeHex(new ColorLineEdit()) {
	CreateUI();
	ConnectUI();
}

QColor4DDialog::~QColor4DDialog() {
	Current = nullptr;
}

void QColor4DDialog::SetColor(QColor4D color) {
	mLastColor = color;
	mColorPreview->setComparisonColor(color);
	SetCurrentColorInternal(color);
}

int QColor4DDialog::CreateAndShow(QColor4D color, QRect inButtonGemotry) {
	QColor4DDialog* dialog = QColor4DDialog::Current;
	dialog->disconnect();
	if (dialog == nullptr) {
		dialog = new QColor4DDialog;
		QColor4DDialog::Current = dialog;
	}
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->SetCloseWhenLoseFocus(true);
	dialog->SetColor(color);
	dialog->show();
	dialog->activateWindow();
	dialog->setFocus();
	QRect mGeom = dialog->geometry();

	mGeom.moveCenter(inButtonGemotry.center());
	mGeom.moveBottom(inButtonGemotry.top());
	if (mGeom.top() < 0) {
		mGeom.moveTop(inButtonGemotry.bottom());
	}
	dialog->setGeometry(mGeom);
	return 0;
}

void QColor4DDialog::CreateUI() {
	setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);
	setFocusPolicy(Qt::NoFocus);
	mColorWheel->setSelectorShape(ColorWheel::ShapeSquare);
	mColorWheel->setFocusPolicy(Qt::NoFocus);
	QVBoxLayout* v = new QVBoxLayout(this);
	QHBoxLayout* h = new QHBoxLayout();
	h->addWidget(mColorWheel, 5);
	QVBoxLayout* tool = new QVBoxLayout;
	tool->setAlignment(Qt::AlignTop);
	tool->addSpacing(40);
	tool->addWidget(mColorPreview);
	tool->addWidget(mLeHex);
	tool->addWidget(mPbPick);
	h->addLayout(tool, 2);

	v->addLayout(h);
	QHBoxLayout* sliderPanle = new QHBoxLayout();
	sliderPanle->setAlignment(Qt::AlignBottom);
	QVBoxLayout* RGBA = new QVBoxLayout;
	QVBoxLayout* HSV = new QVBoxLayout;

	RGBA->setAlignment(Qt::AlignTop);
	RGBA->addWidget(mRedBox);
	RGBA->addWidget(mGreenBox);
	RGBA->addWidget(mBlueBox);
	RGBA->addWidget(mAlphaBox);

	HSV->setAlignment(Qt::AlignTop);
	HSV->addWidget(mHueBox);
	HSV->addWidget(mSaturationBox);
	HSV->addWidget(mValueBox);
	QHBoxLayout* buttonPanel = new QHBoxLayout();
	buttonPanel->setAlignment(Qt::AlignRight);
	buttonPanel->addWidget(mPbOk);
	buttonPanel->addWidget(mPbCancel);
	HSV->addLayout(buttonPanel);

	sliderPanle->addLayout(RGBA);
	sliderPanle->addLayout(HSV);
	v->addLayout(sliderPanle);

	mLeHex->setFixedHeight(25);
	mColorPreview->setFixedHeight(40);
	mColorPreview->setDisplayMode(ColorPreview::SplitColor);
	mColorWheel->setMinimumSize(150, 150);
	mColorWheel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mPbPick->setFocusPolicy(Qt::NoFocus);
	mPbOk->setFocusPolicy(Qt::NoFocus);
	mPbCancel->setFocusPolicy(Qt::NoFocus);
}

void QColor4DDialog::ConnectUI() {
	connect(mColorWheel, &ColorWheel::OnColorChanged, this, [this](QColor color) {
		SetCurrentColorInternal(color);
	});

	connect(mPbPick, &QPushButton::clicked, this, [this]() {
		bool flag = false;
		if (bCloseWhenLoseFocus) {
			flag = bCloseWhenLoseFocus;
			bCloseWhenLoseFocus = false;
		}
		QColor color = QColorPicker::Pick();
		if (color.isValid()) {
			SetCurrentColorInternal(color);
		}
		this->activateWindow();
		this->setFocus();
		if (flag) {
			bCloseWhenLoseFocus = true;
		}
	});

	connect(mLeHex, &ColorLineEdit::OnColorChanged, this, [this](QColor color) {
		SetCurrentColorInternal(color);
	});

	connect(mRedBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = mCurrentColor;
		next.setRedF(var);
		SetCurrentColorInternal(next);
	});
	connect(mGreenBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = mCurrentColor;
		next.setGreenF(var);
		SetCurrentColorInternal(next);
	});
	connect(mBlueBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = mCurrentColor;
		next.setBlueF(var);
		SetCurrentColorInternal(next);
	});
	connect(mAlphaBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = mCurrentColor;
		next.setAlphaF(var);
		SetCurrentColorInternal(next);
	});

	connect(mHueBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = QColor4D::fromHsvF(var, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF());
		SetCurrentColorInternal(next);
	});
	connect(mSaturationBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), var, mCurrentColor.valueF());
		SetCurrentColorInternal(next);
	});
	connect(mValueBox, &QColor4DChannelSlider::AsValueChanged, this, [this](float var) {
		QColor4D next = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), mCurrentColor.hsvSaturationF(), var);
		SetCurrentColorInternal(next);
	});

	connect(mPbOk, &QPushButton::clicked, this, [this]() {
		close();
	});
	connect(mPbCancel, &QPushButton::clicked, this, [this]() {
		SetCurrentColorInternal(mLastColor);
		close();
	});
}

void QColor4DDialog::SetCurrentColorInternal(QColor4D color) {
	if (mCurrentColor == color)
		return;
	mCurrentColor = color;
	RefleshChannelGradiant();

	mColorWheel->blockSignals(true);
	mColorPreview->blockSignals(true);
	mRedBox->blockSignals(true);
	mGreenBox->blockSignals(true);
	mBlueBox->blockSignals(true);
	mAlphaBox->blockSignals(true);
	mHueBox->blockSignals(true);
	mSaturationBox->blockSignals(true);
	mValueBox->blockSignals(true);
	mLeHex->blockSignals(true);

	mColorWheel->SetColor(color.toQColor());
	mColorPreview->SetColor(color.toQColor());
	mLeHex->SetColor(color.toQColor());

	mRedBox->SetChannelValue(color.redF());
	mGreenBox->SetChannelValue(color.greenF());
	mBlueBox->SetChannelValue(color.blueF());
	mAlphaBox->SetChannelValue(color.alphaF());
	mHueBox->SetChannelValue(color.hsvHueF());
	mSaturationBox->SetChannelValue(color.hsvSaturationF());
	mValueBox->SetChannelValue(color.valueF());

	mColorWheel->blockSignals(false);
	mColorPreview->blockSignals(false);
	mRedBox->blockSignals(false);
	mGreenBox->blockSignals(false);
	mBlueBox->blockSignals(false);
	mAlphaBox->blockSignals(false);
	mHueBox->blockSignals(false);
	mSaturationBox->blockSignals(false);
	mValueBox->blockSignals(false);
	mLeHex->blockSignals(false);

	Q_EMIT AsColorChanged(mCurrentColor);
}

void QColor4DDialog::RefleshChannelGradiant() {
	QGradientStops stops;
	QColor begin = mCurrentColor.toQColor();
	QColor end = mCurrentColor.toQColor();

	begin.setRedF(0.0f);
	end.setRedF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mRedBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor.toQColor();
	end = mCurrentColor.toQColor();
	begin.setGreenF(0.0f);
	end.setGreenF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mGreenBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor.toQColor();
	end = mCurrentColor.toQColor();
	begin.setBlueF(0.0f);
	end.setBlueF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mBlueBox->SetGradientStops(stops);
	stops.clear();

	begin = mCurrentColor.toQColor();
	end = mCurrentColor.toQColor();
	begin.setAlphaF(0.0f);
	end.setAlphaF(1.0f);
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mAlphaBox->SetGradientStops(stops);
	stops.clear();

	begin = QColor4D::fromHsvF(0.0f, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF()).toQColor();
	end = QColor4D::fromHsvF(1.0f, mCurrentColor.hsvSaturationF(), mCurrentColor.valueF()).toQColor();
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mHueBox->SetGradientStops(stops);
	stops.clear();

	begin = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), 0.0f, mCurrentColor.valueF()).toQColor();
	end = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), 1.0f, mCurrentColor.valueF()).toQColor();
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mSaturationBox->SetGradientStops(stops);
	stops.clear();

	begin = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), mCurrentColor.hsvSaturationF(), 0.0f).toQColor();
	end = QColor4D::fromHsvF(mCurrentColor.hsvHueF(), mCurrentColor.hsvSaturationF(), 1.0f).toQColor();
	stops.push_back(QGradientStop{ 0.0f,begin });
	stops.push_back(QGradientStop{ 1.0f,end });
	mValueBox->SetGradientStops(stops);
}

void QColor4DDialog::focusOutEvent(QFocusEvent* event) {
	if (bCloseWhenLoseFocus && event->reason() == Qt::FocusReason::ActiveWindowFocusReason) {
		close();
	}
}
