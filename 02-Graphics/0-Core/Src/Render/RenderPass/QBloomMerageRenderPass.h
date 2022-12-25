#ifndef QBloomMerageRenderPass_h__
#define QBloomMerageRenderPass_h__

#include "Render/IRenderPass.h"

class QBloomMerageRenderPass :public IRenderPassBase {
	Q_OBJECT
public:
	QBloomMerageRenderPass();

	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	enum InSlot {
		Raw = 0,
		Blur
	};

	enum OutSlot {
		BloomMerageResult = 0,
	};

private:
	struct RTResource {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	RTResource mRT;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
};

#endif // QBloomMerageRenderPass_h__
