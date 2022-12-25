#include "IRenderPass.h"

void IRenderPassBase::setRenderer(IRenderer* inRenderer) {
	mRenderer = inRenderer;
	mRhi = inRenderer->getRhi();
	setParent(mRenderer);
}

QRhiTexture* IRenderPassBase::getOutputTexture(int slot /*= 0*/) {
	return mOutputTextures.value(slot, nullptr);
}

const QHash<int, QRhiTexture*>& IRenderPassBase::getOutputTextures() {
	return mOutputTextures;
}

IRenderPassBase* IRenderPassBase::setupInputTexture(int inInputSlot, const QString& inPassName, int inPassSlot) {
	mInputTextureLinks[inInputSlot] = { inPassName ,inPassSlot,nullptr };
	return this;
}

QStringList IRenderPassBase::getDependentRenderPassNames() {
	QStringList inputRenderPassNames;
	for (auto& inputLink : mInputTextureLinks) {
		inputRenderPassNames << inputLink.passName;
	}
	return inputRenderPassNames;
}

void IRenderPassBase::cleanupInputLinkerCache() {
	for (auto& inputLinker : mInputTextureLinks) {
		inputLinker.cache = nullptr;
	}
}

void ISceneRenderPass::setRenderer(IRenderer* inRenderer) {
	IRenderPassBase::setRenderer(inRenderer);
	for (auto& comp : mRenderComponents) {
		comp->mRhi = mRhi;
	}
}

void ISceneRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	QVector<IRenderComponent*> uploadItems;
	for (auto& item : mRenderComponents) {
		if (item->sigonRebuildResource.receive()) {
			item->onRebuildResource();
			uploadItems << item;
		}
		if (item->sigonRebuildPipeline.receive()) {
			item->onRebuildPipeline();
		}
		item->onPreUpdate(cmdBuffer);
	}
	QRhiResourceUpdateBatch* resUpdateBatch = mRhi->nextResourceUpdateBatch();
	for (auto& item : uploadItems) {
		item->onUpload(resUpdateBatch);
	}
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->onUpdate(resUpdateBatch);
	}
	cmdBuffer->beginPass(getRenderTarget(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 1.0f), { 1.0f, 0 }, resUpdateBatch, QRhiCommandBuffer::ExternalContent);
	QRhiViewport viewport(0, 0, getRenderTarget()->pixelSize().width(), getRenderTarget()->pixelSize().height());
	for (auto& item : mRenderComponents) {
		if (!item->isVaild())
			continue;
		item->onRender(cmdBuffer, viewport);
	}
	cmdBuffer->endPass();
}

ISceneRenderPass* ISceneRenderPass::addRenderComponent(IRenderComponent* inRenderComponent) {
	inRenderComponent->setParent(this);
	inRenderComponent->mRhi = mRhi;
	inRenderComponent->mScreenRenderPass = this;
	inRenderComponent->sigonRebuildResource.request();
	inRenderComponent->sigonRebuildPipeline.request();
	mRenderComponents.push_back(inRenderComponent);
	return this;
}

QRhiTexture* TextureLinker::getInputTexture(int slot) const{
	InputTextureLinkInfo& linker = mRenderPass->mInputTextureLinks[slot];
	if (linker.cache)
		return linker.cache;
	return linker.cache = mRenderPass->mRenderer->getRenderPassByName(linker.passName)->getOutputTexture(linker.passSlot);
}

void TextureLinker::setOutputTexture(int slot, const QByteArray& name, QRhiTexture* texture) const {
	if (texture)
		texture->setName(name);
	mRenderPass->mOutputTextures[slot] = texture;
}
