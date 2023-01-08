#ifndef QFloatTextureSceneRenderPass_h__
#define QFloatTextureSceneRenderPass_h__

#include "Render/IRenderPass.h"

class QFloatTextureSceneRenderPass : public ISceneRenderPass {
	Q_OBJECT
public:
	enum  OutSlot {
		BaseColor
	};
	struct RTResource {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiRenderBuffer> atDepthStencil;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
protected:
	int getSampleCount() override;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
protected:
	RTResource mRT;
};

#endif // QFloatTextureSceneRenderPass_h__