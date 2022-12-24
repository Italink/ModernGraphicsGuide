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

void IRenderPassBase::registerOutputTexture(int slot, const QByteArray& name, QRhiTexture* texture) {
	if (texture)
		texture->setName(name);
	mOutputTextures[slot] = texture;
}

IRenderPassBase* IRenderPassBase::setupInputTexture(int inInputSlot, const QString& inPassName, int inPassSlot) {
	mInputTextures[inInputSlot] = { inPassName ,inPassSlot,nullptr };
	return this;
}

QRhiTexture* IRenderPassBase::getInputTexture(int slot /*= 0*/) {
	InputTextureLinker& linker = mInputTextures[slot];
	if (linker.result)
		return linker.result;
	return linker.result = mRenderer->getRenderPassByName(linker.passName)->getOutputTexture(linker.passSlot);
}

QStringList IRenderPassBase::getInputRenderPassNames() {
	QStringList inputRenderPassNames;
	for (auto& inputLinker : mInputTextures) {
		inputRenderPassNames << inputLinker.passName;
	}
	return inputRenderPassNames;
}

void IRenderPassBase::cleanupInputLinkerCache() {
	for (auto& inputLinker : mInputTextures) {
		inputLinker.result = nullptr;
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
