#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/RenderPass/QSceneOutputRenderPass.h"
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
	QRhiGraphicsPipelineBuilder* Pipeline;
protected:
	void onRebuildResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();
		Pipeline = new QRhiGraphicsPipelineBuilder(this);
		Pipeline->setParent(this);
		Pipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addParam("MVP", QGenericMatrix<4,4,float>());

		Pipeline->addUniformBlock(QRhiShaderStage::Fragment, "Material")
			->addParam("Color", QVector4D(1, 0.0, 0, 1));

		Pipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(float) * 2)
			});

		Pipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("Position",0,0,QRhiVertexInputAttributeEx::Float2,0)
			});
		Pipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
void main(){
	gl_Position = Transform.MVP * vec4(Position,0.0f,1.0f);
}
)");
		Pipeline->setShaderMainCode(QRhiShaderStage::Fragment, R"(
void main(){
	FragColor = Material.Color;
}
)");
	}
	void onRebuildPipeline() override {
		Pipeline->create(this);
	}
	void onUpload(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void onUpdate(QRhiResourceUpdateBatch* batch) override {
		Pipeline->getUniformBlock("Transform")->setParamValue("MVP", calculateMatrixMVP().toGenericMatrix<4, 4>());
		Pipeline->update(batch);
		if (Pipeline->sigRebuild.receive()) {
			sigonRebuildPipeline.request();
		}
	}
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
		cmdBuffer->setGraphicsPipeline(Pipeline->getGraphicsPipeline());
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
	initParams.backend = QRhi::Implementation::Vulkan;
	QRendererWidget widget(initParams);
	widget.setupDetailWidget();
	widget.setupCamera();
	widget.setFrameGraph(
		QFrameGraphBuilder::begin()
		->addPass("Triangle", (new QSceneOutputRenderPass())
			->addRenderComponent(new QTriangleRenderComponent())
		)
		->end()
	);
	widget.resize({ 800,600 });
	widget.show();
	return app.exec();
}

#include "main.moc"