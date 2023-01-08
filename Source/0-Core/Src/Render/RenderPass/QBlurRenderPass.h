#ifndef QBlurRenderPass_h__
#define QBlurRenderPass_h__

#include "Render/IRenderPass.h"
#include "Core/QMetaDataDefine.h"

class QBlurRenderPass: public IRenderPassBase {
	Q_OBJECT
		Q_PROPERTY(int BlurIterations READ getBlurIter WRITE setupBlurIter)
		Q_PROPERTY(int BlurSize READ getBlurSize WRITE setupBlurSize)
	Q_META_BEGIN(QBlurRenderPass)
		Q_META_P_NUMBER_LIMITED(BlurIterations, 0, 10)
		Q_META_P_NUMBER_LIMITED(BlurSize, 1, 40)
	Q_META_END()
public:
	QBlurRenderPass();

	QBlurRenderPass* setupBlurSize(int size);
	QBlurRenderPass* setupBlurIter(int val);

	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;

	int getBlurIter() const { return mBlurIter; }
	int getBlurSize() const { return mBlurState.size; }
	QRhiTextureRenderTarget* getInputRenderTaget() { return mBlurRT[0].renderTarget.get(); }

	enum InpSlot {
		Src = 0,
	};

	enum OutSlot {
		BlurResult = 0,
	};
private:
	QRhiTexture* mSrcTexture = nullptr;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	struct BlurRT {
		QScopedPointer<QRhiTexture> colorAttachment;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
	};
	BlurRT mBlurRT[2];
	QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineH;
	QScopedPointer<QRhiGraphicsPipeline> mPipelineV;
	QScopedPointer<QRhiShaderResourceBindings> mBindingsH;
	QScopedPointer<QRhiShaderResourceBindings> mBindingsV;
	struct BlurState {
		uint32_t size = 0;
		uint32_t padding[3];
		float weight[50] = { 0 };
	}mBlurState;
	int mBlurIter = 2;
	QRhiEx::Signal sigUpdateBlurState;
};

#endif // QBlurRenderPass_h__
