#include <QApplication>
#include "Render/QRendererWidget.h"
#include "Render/RenderPass/QDefaultSceneRenderPass.h"
#include "Render/RenderComponent/ISceneRenderComponent.h"
#include "Core/QMetaDataDefine.h"

static float VertexData[] = {
	//position(xy)	
	 0.0f,   0.5f,
	-0.5f,  -0.5f,
	 0.5f,  -0.5f,
};

class QTriangleRenderComponent : public ISceneRenderComponent {
	Q_OBJECT
private:
	Q_PROPERTY_VAR(QColor, Color) = Qt::green;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiGraphicsPipelineEx> mMaterial;
protected:
	void recreateResource() override {
		mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, sizeof(VertexData)));
		mVertexBuffer->create();
	}
	void recreatePipeline() override {
		mMaterial.reset(newGraphicsPipeline());
		mMaterial->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(float) * 2)
		});
		mMaterial->setInputAttribute({
			QRhiVertexInputAttributeEx("Position",0,0,QRhiVertexInputAttributeEx::Float2,0)
		});
		mMaterial->setShaderMainCode(QRhiShaderStage::Vertex, R"(
			void main(){
				gl_Position = UBO.Transform * vec4(Postion,0.0f,1.0f);
			}
		)");

		mMaterial->setShaderMainCode(QRhiShaderStage::Fragment, R"(
			void main(){
				outFragColor = vec4(1);
			}
		)");
		mMaterial->create();
	}

	void uploadResource(QRhiResourceUpdateBatch* batch) override {
		batch->uploadStaticBuffer(mVertexBuffer.get(), VertexData);
	}
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override {
		//QMatrix4x4 mat = calculateMatrixMVP();
		//batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(float) * 16, &mat);
		//QVector4D vec4(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
		//batch->updateDynamicBuffer(mUniformBuffer.get(), sizeof(float) * 16, sizeof(QVector4D), &vec4);
	}
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override {
		cmdBuffer->setGraphicsPipeline(mMaterial->getGraphicsPipeline());
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