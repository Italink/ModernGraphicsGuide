#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/IRenderComponent.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class QTriangleRenderComponent : public IRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QColor Color READ getColor WRITE setColor)
private:
	QColor mColor = Qt::green;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderBindings;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
public:
	QColor getColor() const { return mColor; }
	void setColor(QColor val) { mColor = val; }
protected:
	void recreateResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();

		mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D)));
		mUniformBuffer->create();
	}
	void recreatePipeline() override {
		mShaderBindings.reset(mRhi->newShaderResourceBindings());
		mShaderBindings->setBindings({
			QRhiShaderResourceBinding::uniformBuffer(0,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
		});
		mShaderBindings->create();

		mPipeline.reset(mRhi->newGraphicsPipeline());

		QRhiGraphicsPipeline::TargetBlend targetBlend;
		targetBlend.enable = false;
		mPipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });

		mPipeline->setSampleCount(sceneRenderPass()->getSampleCount());

		mPipeline->setDepthTest(false);
		mPipeline->setDepthOp(QRhiGraphicsPipeline::Always);
		mPipeline->setDepthWrite(false);

		QShader vs = QRhiEx::newShaderFromCode(QShader::VertexStage, R"(#version 440
layout(location = 0) in vec2 position;
out gl_PerVertex { 
	vec4 gl_Position;
};
void main(){
    gl_Position = vec4(position,0.0f,1.0f);
}
)");
		Q_ASSERT(vs.isValid());

		QShader fs = QRhiEx::newShaderFromCode(QShader::FragmentStage, R"(#version 440
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform UniformBuffer{
	vec4 uColor;
}UBO;
void main(){
    fragColor = UBO.uColor;
}
)");
		Q_ASSERT(fs.isValid());

		mPipeline->setShaderStages({
			QRhiShaderStage(QRhiShaderStage::Vertex, vs),
			QRhiShaderStage(QRhiShaderStage::Fragment, fs)
			});

		QRhiVertexInputLayout inputLayout;
		inputLayout.setBindings({
			QRhiVertexInputBinding(2 * sizeof(float))
			});

		inputLayout.setAttributes({
			QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float2, 0),
			});

		mPipeline->setVertexInputLayout(inputLayout);
		mPipeline->setShaderResourceBindings(mShaderBindings.get());
		mPipeline->setRenderPassDescriptor(sceneRenderPass()->getRenderPassDescriptor());
		mPipeline->create();
	}

	void uploadResource(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override {
		QVector4D vec4(mColor.redF(),mColor.greenF(),mColor.blueF(),mColor.alphaF());
		batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(QVector4D), &vec4);
	}
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
		cmdBuffer->setGraphicsPipeline(mPipeline.get());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings);
		cmdBuffer->draw(3);
	}
};

int main(int argc, char** argv) {
	qputenv("QSG_INFO", "1");
	QApplication app(argc, argv);
	QRhiWindow::InitParams initParams;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->node("Triangle", (new QDefaultSceneRenderPass())
			->addRenderComponent(new QTriangleRenderComponent())
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#include "main.moc"