#ifndef MyFirstTriangleWindow_h__
#define MyFirstTriangleWindow_h__

#include "RHI/QRhiWindow.h"

static float vertexData[] = { 
	//position(xyz)				color(rgba)
	 0.0f,   0.5f,   0.0f,      1.0f, 0.0f, 0.0f, 1.0f,
	-0.5f,  -0.5f,   0.0f,      0.0f, 1.0f, 0.0f, 1.0f,
	 0.5f,  -0.5f,   0.0f,      0.0f, 0.0f, 1.0f, 1.0f
};

class MyFirstTriangleWindow : public QRhiWindow {
public:
	MyFirstTriangleWindow(QRhiWindow::InitParams inInitParams):QRhiWindow(inInitParams) {
		bNeedInit.mark();
	}
protected:
	void initRhiResource() {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(vertexData)));
		mVertexBuffer->create();

		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend()});

		mPipeline->setSampleCount(mSwapChain->sampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

		QShader vs = QRhiToolkit::createShaderFromCode(QShader::VertexStage,R"(#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 v_color;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
    v_color = color;
    gl_Position = vec4(position,1.0f);
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiToolkit::createShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) in vec4 v_color;
layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = v_color;
}
)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			{ QRhiShaderStage::Vertex, vs },
			{ QRhiShaderStage::Fragment, fs }
		});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			{ 7 * sizeof(float) }
			});
		inputLayout.setAttributes({
			{ 0, 0, QRhiVertexInputAttribute::Float3, 0 },
			{ 0, 1, QRhiVertexInputAttribute::Float4, 3 * sizeof(float) }
			});

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->setRenderPassDescriptor(mSwapChainPassDesc.get());
		mPipeline->create();

		bNeedSubmit.mark();
	}

	void submitRhiData(QRhiResourceUpdateBatch* resourceUpdates) {
		resourceUpdates->uploadStaticBuffer(mVertexBuffer.get(), vertexData);
	}

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

		const QColor clearColor = QColor::fromRgbF(0.0f, 0.0f, 1.0f, 1.0f);
		const QRhiDepthStencilClearValue dsClearValue = { 1.0f,0 };

		currentCmdBuffer->beginPass(currentRenderTarget, clearColor, dsClearValue, resourceUpdates);

		currentCmdBuffer->setGraphicsPipeline(mPipeline.get());
		currentCmdBuffer->setViewport(QRhiViewport(0, 0, mSwapChain->currentPixelSize().width(), mSwapChain->currentPixelSize().height()));
		currentCmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get()	, 0);
		currentCmdBuffer->setVertexInput(0, 1, &vertexBindings);
		currentCmdBuffer->draw(3);

		currentCmdBuffer->endPass();
	}
private:
	QRhiToolkit::DirtySignal bNeedInit;
	QRhiToolkit::DirtySignal bNeedSubmit;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
};

#endif // MyFirstTriangleWindow_h__
