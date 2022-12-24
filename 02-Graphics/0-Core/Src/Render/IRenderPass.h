#ifndef IRenderPassBase_h__
#define IRenderPassBase_h__

#include "RHI/QRhiEx.h"
#include "IRenderer.h"
#include "IRenderComponent.h"

class IRenderPassBase: public QObject{
	friend class QFrameGraph;
	struct InputTextureLinker {
		QString passName;
		int passSlot;
		QRhiTexture* result = nullptr;
	};
public:
	virtual void setRenderer(IRenderer* inRenderer);

	IRenderer* getRenderer() const { return mRenderer; }

	virtual void compile() = 0;

	virtual void resize(const QSize& size) {}

	virtual void render(QRhiCommandBuffer* cmdBuffer) = 0;

	virtual QRhiTexture* getOutputTexture(int slot = 0);

	const QHash<int, QRhiTexture*>& getOutputTextures();

	void registerOutputTexture(int slot, const QByteArray& name, QRhiTexture* texture);

	IRenderPassBase* setupInputTexture(int inInputSlot, const QString& inPassName, int inPassSlot);

	QRhiTexture* getInputTexture(int slot = 0);

	QStringList getInputRenderPassNames();

	void cleanupInputLinkerCache();

protected:
	IRenderer* mRenderer = nullptr;
	QSharedPointer<QRhiEx> mRhi;
	QHash<int, InputTextureLinker> mInputTextures;
	QHash<int, QRhiTexture*> mOutputTextures;
};

class ISceneRenderPass :public IRenderPassBase {
	Q_OBJECT
public:
	virtual int getSampleCount() = 0;
	virtual int getRenderTargetCount() = 0;
	virtual QRhiRenderPassDescriptor* getRenderPassDescriptor() = 0;
	virtual QRhiRenderTarget* getRenderTarget() = 0;
	void setRenderer(IRenderer* inRenderer) override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
	ISceneRenderPass* addRenderComponent(IRenderComponent* inRenderComponent);
protected:
	QVector<IRenderComponent*> mRenderComponents;
};

#endif // IRenderPassBase_h__
