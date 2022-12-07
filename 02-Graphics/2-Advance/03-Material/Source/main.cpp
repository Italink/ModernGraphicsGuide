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
	Q_PROPERTY_VAR(QRhiGraphicsPipelineBuilder*, Pipeline);
protected:
	void recreateResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();
		Pipeline = new QRhiGraphicsPipelineBuilder(this);

		Pipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addMat4("MVP", QGenericMatrix<4,4,float>());

		Pipeline->addUniformBlock(QRhiShaderStage::Fragment, "Material")
			->addVec4("Color", QVector4D(1, 0.0, 0, 1));

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
	void recreatePipeline() override {
		Pipeline->create(this);
	}
	void uploadResource(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override {
		Pipeline->getUniformBlock("Transform")->setMat4("MVP", calculateMatrixMVP().toGenericMatrix<4, 4>());
		Pipeline->update(batch);
		if (Pipeline->sigRebuild.receive()) {
			sigRecreatePipeline.request();
		}
	}
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
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