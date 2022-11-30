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
	virtual void setRenderer(IRenderer* inRenderer) {
		mRenderer = inRenderer;
		mRhi = inRenderer->getRhi();
		setParent(mRenderer);
	}

	IRenderer* getRenderer() const { return mRenderer; }

	virtual void compile() = 0;

	virtual void resize(const QSize& size) {}

	virtual void render(QRhiCommandBuffer* cmdBuffer) = 0;

	virtual QRhiTexture* getOutputTexture(int slot = 0) {
		return mOutputTextures.value(slot, nullptr);
	}
	const QHash<int, QRhiTexture*>& getOutputTextures() {
		return mOutputTextures;
	}

	void registerOutputTexture(int slot, const QByteArray& name, QRhiTexture* texture) {
		if(texture)
			texture->setName(name);
		mOutputTextures[slot] = texture;
	}

	IRenderPassBase* setupInputTexture(int inInputSlot,const QString& inPassName, int inPassSlot) {
		mInputTextures[inInputSlot] = { inPassName ,inPassSlot,nullptr};
	}

	QRhiTexture* getInputTexture(int slot = 0) {
		InputTextureLinker & linker = mInputTextures[slot];
		if (linker.result)
			return linker.result;
		return linker.result = mRenderer->getRenderPassByName(linker.passName)->getOutputTexture(linker.passSlot);
	}

	QList<IRenderPassBase*> getInputRenderPasses() {
		QList<IRenderPassBase*> inputRenderPasses;
		for (auto& inputLinker : mInputTextures) {
			inputRenderPasses << mRenderer->getRenderPassByName(inputLinker.passName);
		}
		return inputRenderPasses;
	}

	void cleanup() {
		for (auto& inputLinker : mInputTextures) {
			inputLinker.result = nullptr;
		}
	}
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
	virtual int getBlendStateCount() = 0;
	virtual QRhiRenderPassDescriptor* getRenderPassDescriptor() = 0;
	void setRenderer(IRenderer* inRenderer) override {
		IRenderPassBase::setRenderer(inRenderer);
		for (auto& comp : mRenderComponents) {
			comp->mRhi = mRhi;
		}
	}

	ISceneRenderPass* addRenderComponent(IRenderComponent* inRenderComponent) {
		inRenderComponent->setParent(this);
		inRenderComponent->mRhi = mRhi;
		inRenderComponent->mScreenRenderPass = this;
		inRenderComponent->sigRecreateResource.request();
		inRenderComponent->sigRecreatePipeline.request();
		mRenderComponents.push_back(inRenderComponent);
		return this;
	}
protected:
	QVector<IRenderComponent*> mRenderComponents;
};

#endif // IRenderPassBase_h__
