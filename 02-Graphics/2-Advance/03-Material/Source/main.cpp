#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/RenderComponent/ISceneRenderComponent.h"
#include "Core/QMetaDataDefine.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class QTriangleRenderComponent : public ISceneRenderComponent {
	Q_OBJECT
private:
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	Q_PROPERTY_VAR(QRhiGraphicsPipelineBuilder*,mPipelineBuilder);
protected:
	void recreateResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();
	}
	void recreatePipeline() override {
		mPipelineBuilder = new QRhiGraphicsPipelineBuilder;
		mPipelineBuilder->setParent(this);
		mPipelineBuilder->addUniformBlock(QRhiShaderStage::Fragment, "Transform")
			->addVec4("MVP", QVector4D(1,0.0,0,1));

		mPipelineBuilder->addUniformBlock(QRhiShaderStage::Fragment, "Material")
			->addVec4("Color", QVector4D(1, 0.0, 0, 1));

		mPipelineBuilder->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(float) * 2)
		});
		mPipelineBuilder->setInputAttribute({
			QRhiVertexInputAttributeEx("Position",0,0,QRhiVertexInputAttributeEx::Float2,0)
		});
		mPipelineBuilder->setShaderMainCode(QRhiShaderStage::Vertex, R"(
void main(){
	gl_Position = vec4(Position,0.0f,1.0f);
}
)");
		mPipelineBuilder->setShaderMainCode(QRhiShaderStage::Fragment, R"(
void main(){
	FragColor = Material.Color;
}
)");
		mPipelineBuilder->create(this);
	}
	void uploadResource(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override {
		mPipelineBuilder->update(batch);
	}
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
		cmdBuffer->setGraphicsPipeline(mPipelineBuilder->getGraphicsPipeline());
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
	initParams.backend = QRhi::Implementation::D3D11;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
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