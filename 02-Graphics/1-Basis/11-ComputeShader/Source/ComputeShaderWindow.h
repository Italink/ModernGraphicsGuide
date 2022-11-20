#ifndef ComputeShaderWindow_h__
#define ComputeShaderWindow_h__

#include "RHI/QRhiWindow.h"
#include "Render/Painter/TexturePainter.h"

class ComputeShaderWindow : public QRhiWindow {
private:
	QRhiEx::DirtySignal bNeedInit;
	QRhiEx::DirtySignal bNeedSubmit;

	QScopedPointer<QRhiBuffer> mStorageBuffer;
	QScopedPointer<QRhiTexture> mTexture;

	QScopedPointer<QRhiComputePipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;

	QScopedPointer<TexturePainter> mTexturePainter;

	const int ImageWidth = 64;
	const int ImageHeight = 64;
public:
	ComputeShaderWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		bNeedInit.mark();
		bNeedSubmit.mark();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* currentCmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (bNeedInit.handle()) {
			initRhiResource();
		}
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (bNeedSubmit.handle()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			submitRhiData(resourceUpdates);
		}

		currentCmdBuffer->beginComputePass(resourceUpdates);
		currentCmdBuffer->setComputePipeline(mPipeline.get());
		currentCmdBuffer->setShaderResources();
		currentCmdBuffer->dispatch(ImageWidth, ImageHeight, 1);
		currentCmdBuffer->endComputePass();

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue);
		mTexturePainter->paint(currentCmdBuffer, currentRenderTarget);
		currentCmdBuffer->endPass();
	}

	void initRhiResource() {
		mStorageBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, sizeof(float)));
		mStorageBuffer->create();
		mTexture.reset(mRhi->newTexture(QRhiTexture::RGBA8,QSize(ImageWidth,ImageHeight),1,QRhiTexture::UsedWithLoadStore));
		mTexture->create();

		mTexturePainter.reset(new TexturePainter);
		mTexturePainter->setupRhi(mRhi);
		mTexturePainter->setupRenderPassDesc(mSwapChain->renderPassDescriptor());
		mTexturePainter->setupSampleCount(mSwapChain->sampleCount());
		mTexturePainter->setupTexture(mTexture.get());
		mTexturePainter->compile();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::bufferLoadStore(0,QRhiShaderResourceBinding::ComputeStage,mStorageBuffer.get()),
			QRhiShaderResourceBinding::imageLoadStore(1,QRhiShaderResourceBinding::ComputeStage,mTexture.get(),0),
		});
		mShaderBindings->create();

		mPipeline.reset(mRhi->newComputePipeline());

		QShader cs = QRhiEx::newShaderFromCode(QShader::ComputeStage, R"(#version 440
layout(std140, binding = 0) buffer StorageBuffer{
	int counter;
}SSBO;
layout (binding = 1, rgba8) uniform image2D Tex;

void main(){
	int currentCounter = atomicAdd(SSBO.counter,1);
	//int currentCounter = SSBO.counter = SSBO.counter + 1;
	ivec2 pos = ivec2(gl_GlobalInvocationID.xy);
	imageStore(Tex,pos,vec4(sin(currentCounter/100.0f),0,0,1));
}
)");
		Q_ASSERT(cs.isValid());

		mPipeline->setShaderStage({
			QRhiShaderStage(QRhiShaderStage::Compute, cs),
		});

		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->create();
	}

	void submitRhiData(QRhiResourceUpdateBatch* resourceUpdates) {
		const int counter = 0;
		resourceUpdates->uploadStaticBuffer(mStorageBuffer.get(),&counter);
	}

};

#endif // ComputeShaderWindow_h__
