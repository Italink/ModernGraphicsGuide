#ifndef QPixelFilterRenderPass_h__
#define QPixelFilterRenderPass_h__

#include "Render/IRenderPass.h"
#include "Core/QMetaDataDefine.h"

class QPixelFilterRenderPass : public IRenderPassBase {
	Q_OBJECT
		Q_PROPERTY(int DownSamplerCount READ getDownSamplerCount WRITE setupDownSamplerCount)
		Q_META_BEGIN(QPixelFilterRenderPass)
			Q_META_P_NUMBER_LIMITED(DownSamplerCount, 1, 16)
		Q_META_END()
public:
	enum InSlot {
		Src
	};
	enum OutSlot {
		FilterResult
	};
	QPixelFilterRenderPass* setupFilterCode(QByteArray code);
	QPixelFilterRenderPass* setupDownSamplerCount(int count);
	int getDownSamplerCount() const;
protected:
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QByteArray mFilterCode;
	int mDownSamplerCount = 2;
	QRhiEx::Signal sigRebuild;
};

#endif // QPixelFilterRenderPass_h__