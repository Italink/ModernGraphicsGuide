#ifndef TexturePainter_h__
#define TexturePainter_h__

#include "Render/IPainter.h"

class TexturePainter :public IPainter {
public:
	TexturePainter();

	void setupTexture(QRhiTexture* texture);

	void setupSampler(QRhiSampler::Filter magFilter = QRhiSampler::Nearest,
		QRhiSampler::Filter minFilter = QRhiSampler::Nearest,
		QRhiSampler::Filter mipmapMode = QRhiSampler::Nearest,
		QRhiSampler::AddressMode addressU = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressV = QRhiSampler::Repeat,
		QRhiSampler::AddressMode addressW = QRhiSampler::Repeat);

	virtual void compile() override;
	virtual void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	QRhiTexture* mTexture;

	QScopedPointer<QRhiSampler> mSampler;
	QRhiSampler::Filter mMagFilter = QRhiSampler::Nearest;
	QRhiSampler::Filter mMinFilter = QRhiSampler::Nearest;
	QRhiSampler::Filter mMipmapMode = QRhiSampler::Nearest;
	QRhiSampler::AddressMode mAddressU = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressV = QRhiSampler::Repeat;
	QRhiSampler::AddressMode mAddressW = QRhiSampler::Repeat;

	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
};

#endif // TexturePainter_h__
