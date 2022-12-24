#ifndef QTextureOutputRenderPass_h__
#define QTextureOutputRenderPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/TexturePainter.h"

class QTextureOutputRenderPass : public IRenderPassBase {
	Q_OBJECT
public:
	enum InSlot {
		BaseColor
	};
	QTextureOutputRenderPass();
	void compile() override;
	void resize(const QSize& size) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
private:
	QScopedPointer<TexturePainter> mTexturePainter;
};

#endif // QTextureOutputRenderPass_h__