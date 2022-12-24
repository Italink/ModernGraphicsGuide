#include "QFloatTextureSceneRenderPass.h"

int QFloatTextureSceneRenderPass::getSampleCount() {
	return 1;
}

int QFloatTextureSceneRenderPass::getRenderTargetCount() {
	return 1;
}

QRhiRenderPassDescriptor* QFloatTextureSceneRenderPass::getRenderPassDescriptor() {
	return mRT.renderPassDesc.get();
}

QRhiRenderTarget* QFloatTextureSceneRenderPass::getRenderTarget() {
	return mRT.renderTarget.get();
}

void QFloatTextureSceneRenderPass::compile() {
	registerOutputTexture(0, "BaseColor",mRT.atBaseColor.get());
}

void QFloatTextureSceneRenderPass::resize(const QSize& size) {
	QVector<QRhiColorAttachment> mColorAttachmentList;
	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atBaseColor->create();

	mRT.atDepthStencil.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, size, 1));
	mRT.atDepthStencil->create();

	QRhiTextureRenderTargetDescription RTDesc;
	RTDesc.setColorAttachments({ QRhiColorAttachment(mRT.atBaseColor.get()) });
	RTDesc.setDepthStencilBuffer(mRT.atDepthStencil.get());
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();
}
