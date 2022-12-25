#ifndef QTextureOutputRenderPass_h__
#define QTextureOutputRenderPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/TexturePainter.h"

class QTextureOutputRenderPass : public IRenderPassBase {
	Q_OBJECT
public:
	QTextureOutputRenderPass();
	QTextureOutputRenderPass* addTexture(const QString& inPassName, int inSlot);
protected:
	void compile() override;
	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QList<QSharedPointer<TexturePainter>> mTexturePainters;
};

#endif // QTextureOutputRenderPass_h__