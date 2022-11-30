#ifndef DepthStencilTestWindow_h__
#define DepthStencilTestWindow_h__

#include "RHI/QRhiWindow.h"

static float VertexData1[] = {
	//position(xyz)				color(rgb)
	 0.0f,   0.5f,   0.0f,		1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,  -0.5f,	 0.0f,		1.0f, 0.0f, 0.0f, 1.0f,
	 0.5f,  -0.5f,	 0.0f,		1.0f, 0.0f, 0.0f, 1.0f,
};

static float VertexData2[] = {
	//position(xy)			   color(rgb)
	 0.2f,   0.5f,	 0.0f,		0.0f, 0.0f, 1.0f, 1.0f,
	-0.3f,  -0.5f,	 0.0f,		0.0f, 0.0f, 1.0f, 1.0f,
	 0.7f,  -0.5f,	 0.0f,		0.0f, 0.0f, 1.0f, 1.0f,
};

class DepthStencilTestWindow : public QRhiWindow {
private:
	QRhiEx::Signal sigInit;
	QRhiEx::Signal sigSubmit;

	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;

	QScopedPointer<QRhiBuffer> mVertexBuffer1;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline1;

	QScopedPointer<QRhiBuffer> mVertexBuffer2;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline2;
public:
	DepthStencilTestWindow(QRhiWindow::InitParams inInitParams) :QRhiWindow(inInitParams) {
		sigInit.request();
		sigSubmit.request();
	}
protected:
	virtual void onRenderTick() override {
		QRhiRenderTarget* currentRenderTarget = mSwapChain->currentFrameRenderTarget();
		QRhiCommandBuffer* currentCmdBuffer = mSwapChain->currentFrameCommandBuffer();

		if (sigInit.receive()) {
			initRhiResource();
		}
		QRhiResourceUpdateBatch* resourceUpdates = nullptr;
		if (sigSubmit.receive()) {
			resourceUpdates = mRhi->nextResourceUpdateBatch();
			submitRhiData(resourceUpdates);
		}

		const QColor clearColor = QColor::fromRgbF(0.2f, 0.2f, 0.2f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		currentCmdBuffer->setGraphicsPipeline(mPipeline1.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings1(mVertexBuffer1.get(), 0);
		currentCmdBuffer->setVertexInput(0, 1, &vertexBindings1);
		currentCmdBuffer->draw(3);

		currentCmdBuffer->setGraphicsPipeline(mPipeline2.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings2(mVertexBuffer2.get(), 0);
		currentCmdBuffer->setVertexInput(0, 1, &vertexBindings2);
		currentCmdBuffer->draw(3);

		currentCmdBuffer->endPass();
	}

	void initRhiResource() {
		mVertexBuffer1.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData1)));
		mVertexBuffer1->create();

		mVertexBuffer2.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData2)));
		mVertexBuffer2->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->create();

		QShader vs = QRhiEx::newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 v_color;
out gl_PerVertex { 
	vec4 gl_Position;
};
void main(){
	v_color = color;
    gl_Position = vec4(position,1.0f);
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiEx::newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 fragColor;
void main(){
    fragColor = v_color;
}
)");
		Q_ASSERT(fs.isValid());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			QRhiVertexInputBinding(7 * sizeof(float))
		});

		inputLayout.setAttributes({
			QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0),
			QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float4, 3 * sizeof(float))
		});

		mPipeline1.reset(mRhi->newGraphicsPipeline());

		mPipeline1->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline1->setSampleCount(mSwapChain->sampleCount());

		mPipeline1->setShaderStages({
			QRhiShaderStage(QRhiShaderStage::Vertex, vs),
			QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});

		mPipeline1->setVertexInputLayout(inputLayout);
		mPipeline1->setShaderResourceBindings(mShaderBindings.get());
		mPipeline1->setRenderPassDescriptor(mSwapChainPassDesc.get());


		mPipeline2.reset(mRhi->newGraphicsPipeline());
		mPipeline2->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline2->setSampleCount(mSwapChain->sampleCount());
		mPipeline2->setShaderStages({
			QRhiShaderStage(QRhiShaderStage::Vertex, vs),
			QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});
	
		mPipeline2->setVertexInputLayout(inputLayout);
		mPipeline2->setShaderResourceBindings(mShaderBindings.get());
		mPipeline2->setRenderPassDescriptor(mSwapChainPassDesc.get());

		mPipeline1->setDepthTest(false);
		mPipeline1->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline1->setDepthWrite(false);
		mPipeline1->setStencilTest(false);

		mPipeline2->setDepthTest(false);
		mPipeline2->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline2->setDepthWrite(false);
		mPipeline2->setStencilTest(false);

		mPipeline1->create();
		mPipeline2->create();
	}

	void submitRhiData(QRhiResourceUpdateBatch* resourceUpdates) {
		resourceUpdates->uploadStaticBuffer(mVertexBuffer1.get(), VertexData1);
		resourceUpdates->uploadStaticBuffer(mVertexBuffer2.get(), VertexData2);
	}

};

#endif // DepthStencilTestWindow_h__
