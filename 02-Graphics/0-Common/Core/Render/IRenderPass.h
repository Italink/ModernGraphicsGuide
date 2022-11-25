#ifndef IRenderPassBase_h__
#define IRenderPassBase_h__

#include "RHI/QRhiEx.h"
#include "IRenderComponent.h"

class IRenderer;

class IRenderPassBase: public QObject{
	friend class QFrameGraphBuilder;
public:
	virtual void compile() = 0;

	virtual void resize(const QSize& size) {}

	virtual void render(QRhiCommandBuffer* cmdBuffer) = 0;

	virtual QRhiTexture* getOutputTexture(int slot = 0) {
		return mOutputTextures.value(slot, nullptr);
	}
	IRenderPassBase* setupInputTexture(int inInputSlot,const QString& inPassName, int inPassSlot) {
		mInputTextures[inInputSlot] = { inPassName ,inPassSlot,nullptr};
	}
	const QHash<int, QRhiTexture*>& getOutputTextures() {
		return mOutputTextures;
	}
protected:
	IRenderer* mRenderer = nullptr;
	QSharedPointer<QRhiEx> mRhi;
	struct InputTextureLinker {
		QString passName;
		int passSlot;
		QRhiTexture* result = nullptr;
	};
	QHash<int, InputTextureLinker> mInputTextures;
	QHash<int, QRhiTexture*> mOutputTextures;
};

class ISceneRenderPass :public IRenderPassBase {
public:
	virtual int getSampleCount() = 0;
	virtual QVector<QRhiGraphicsPipeline::TargetBlend> getBlendStates() = 0;
	virtual QRhiRenderPassDescriptor* getRenderPassDescriptor() = 0;

	ISceneRenderPass* addRenderComponent(IRenderComponent* inRenderComponent) {
		inRenderComponent->mRhi = mRhi;
		inRenderComponent->mScreenRenderPass = this;
		mRenderComponents.push_back(inRenderComponent);
		return this;
	}
protected:
	QVector<IRenderComponent*> mRenderComponents;
};

#endif // IRenderPassBase_h__
