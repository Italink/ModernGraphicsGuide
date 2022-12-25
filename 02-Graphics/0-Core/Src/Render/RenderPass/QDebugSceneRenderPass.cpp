#include "QDebugSceneRenderPass.h"
#include "Render/RenderComponent/ISceneRenderComponent.h"
#include "qevent.h"
#include "QApplication"
#include "../Renderer/QWindowRenderer.h"

DebugUIPainter::DebugUIPainter() {
	setupPaintFunctor([this]() {
		auto& io = ImGui::GetIO();
		ImGuizmo::BeginFrame();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		QCamera* camera = mDebugScenePass->getRenderer()->getCamera();
		if (camera) {
			ISceneRenderComponent* CurrComponent = qobject_cast<ISceneRenderComponent*>(mDebugScenePass->getRenderer()->getCurrentObject());
			if (CurrComponent) {
				QMatrix4x4 MAT;
				QMatrix4x4 Model = CurrComponent->calculateMatrixModel();
				QMatrix4x4 View = camera->getMatrixView();
				QMatrix4x4 Clip = camera->getMatrixClip();
				ImGuizmo::Manipulate(View.constData(), Clip.constData(), mOpt, ImGuizmo::LOCAL, Model.data(), NULL, NULL, NULL, NULL);
				CurrComponent->setTransform(Model);
			}
		}
	});
}

void DebugUIPainter::setupDebugRenderPass(QDebugSceneRenderPass* pass) {
	mDebugScenePass = pass;
}

void DebugUIPainter::setupDebugIDTexture(QRhiTexture* texture) {
	mDebugIDTexture = texture;
}

void DebugUIPainter::resourceUpdate(QRhiResourceUpdateBatch* batch) {
	if (!mReadPoint.isNull() && mDebugIDTexture) {
		mReadDesc.setTexture(mDebugIDTexture);
		mReadReult.completed = [this]() {
			if (!mReadPoint.isNull()) {
				const uchar* p = reinterpret_cast<const uchar*>(mReadReult.data.constData());
				int offset = (mReadReult.pixelSize.width() * mReadPoint.y() + mReadPoint.x()) * 4;
				uint32_t ID = p[offset] + p[offset + 1] * 256 + p[offset + 2] * 256 * 256 + p[offset + 3] * 256 * 256 * 256;
				qDebug() << "Picker ID: " << ID;
				mDebugScenePass->getRenderer()->setCurrentObject(mDebugScenePass->GetComponentByID(ID));
				mReadPoint = { 0,0 };
			}
		};
		batch->readBackTexture(mReadDesc, &mReadReult);
		mRhi->finish();
	}
	ISceneRenderComponent* CurrComponent = qobject_cast<ISceneRenderComponent*>(mDebugScenePass->getRenderer()->getCurrentObject());
	QVector4D ID(-1, -1, -1, -1);
	if (CurrComponent) {
		ID = mDebugScenePass->getIDVec4(CurrComponent);
	}
	batch->updateDynamicBuffer(mUniformBuffer.get(), 0, sizeof(QVector4D), &ID);
	ImGuiPainter::resourceUpdate(batch);
}

void DebugUIPainter::compile() {
	ImGuiPainter::compile();

	mUniformBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D)));
	mUniformBuffer->create();

	mOutlineSampler.reset(mRhi->newSampler(QRhiSampler::Nearest,
		QRhiSampler::Nearest,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	mOutlineSampler->create();
	mOutlinePipeline.reset(mRhi->newGraphicsPipeline());
	QRhiGraphicsPipeline::TargetBlend blendState;
	blendState.enable = true;
	mOutlinePipeline->setTargetBlends({ blendState });
	mOutlinePipeline->setSampleCount(mSampleCount);

	QString vsCode = R"(#version 450
	layout (location = 0 ) out vec2 vUV;
	out gl_PerVertex{
		vec4 gl_Position;
	};
	void main() {
		vUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
		gl_Position = vec4(vUV * 2.0f - 1.0f, 0.0f, 1.0f);
		%1
	}
	)";
	QShader vs = mRhi->newShaderFromCode(QShader::VertexStage, vsCode.arg(mRhi->isYUpInNDC() ? "	vUV.y = 1 - vUV.y;" : "").toLocal8Bit());

	QShader fs = mRhi->newShaderFromCode(QShader::FragmentStage, R"(#version 450
	layout (location = 0) in vec2 vUV;
	layout (location = 0) out vec4 outFragColor;

	layout (binding = 0) uniform sampler2D uDebugId;
	layout (binding = 1) uniform DebugID{
		vec4 ID;
	}Current;

	void main() {
		vec2 texOffset = 1.0 / textureSize(uDebugId, 0);		// gets size of single texel
		
		int count = 0;

		count += (texture(uDebugId,vUV) == Current.ID ? 1 : 0 );		
		count += (texture(uDebugId,vUV+vec2(texOffset.x,0)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV-vec2(texOffset.x,0)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV+vec2(0,texOffset.y)) == Current.ID ? 1 : 0 );
		count += (texture(uDebugId,vUV-vec2(0,texOffset.y)) == Current.ID ? 1 : 0 );

		if(count>0&&count<5){
			outFragColor = vec4(1.0,0.8,0.4,1.0);
		}
		else{
			outFragColor = vec4(0);
		}
	}
	)");
	mOutlinePipeline->setShaderStages({
		{ QRhiShaderStage::Vertex, vs },
		{ QRhiShaderStage::Fragment, fs }
		});
	QRhiVertexInputLayout inputLayout;

	mOutlineBindings.reset(mRhi->newShaderResourceBindings());

	mOutlineBindings->setBindings({
		QRhiShaderResourceBinding::sampledTexture(0,QRhiShaderResourceBinding::FragmentStage,mDebugIDTexture,mOutlineSampler.get()),
		QRhiShaderResourceBinding::uniformBuffer(1,QRhiShaderResourceBinding::FragmentStage,mUniformBuffer.get())
		});

	mOutlineBindings->create();
	mOutlinePipeline->setVertexInputLayout(inputLayout);
	mOutlinePipeline->setShaderResourceBindings(mOutlineBindings.get());
	mOutlinePipeline->setRenderPassDescriptor(mRenderPassDesc);
	mOutlinePipeline->create();
}

void DebugUIPainter::paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) {
	cmdBuffer->setGraphicsPipeline(mOutlinePipeline.get());
	cmdBuffer->setViewport(QRhiViewport(0, 0, renderTarget->pixelSize().width(), renderTarget->pixelSize().height()));
	cmdBuffer->setShaderResources(mOutlineBindings.get());
	cmdBuffer->draw(4);
	ImGuiPainter::paint(cmdBuffer, renderTarget);
}

bool DebugUIPainter::eventFilter(QObject* watched, QEvent* event) {
	static QPoint pressedPos;
	if (watched != nullptr) {
		switch (event->type()) {
		case QEvent::MouseButtonPress:
			pressedPos = QCursor::pos();
			break;
		case QEvent::MouseButtonRelease: {
			QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
			if (pressedPos == QCursor::pos() && mouse->button() == Qt::LeftButton) {
				QPoint pt = mouse->pos() * qApp->devicePixelRatio();
				if (mRhi->isYUpInNDC()) {
					pt.setY(mWindow->height() - pt.y());
				}
				mReadPoint = pt;
			}
			pressedPos = { 0,0 };
			break;
		}
		case QEvent::MouseMove: {
			if (qApp->mouseButtons() & Qt::LeftButton) {
			}
			break;
		}
		case QEvent::KeyPress: {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_W) {
				mOpt = ImGuizmo::OPERATION::TRANSLATE;
			}
			else if (keyEvent->key() == Qt::Key_E) {
				mOpt = ImGuizmo::OPERATION::ROTATE;
			}
			else if (keyEvent->key() == Qt::Key_R) {
				mOpt = ImGuizmo::OPERATION::SCALE;
			}
			break;
		}
		case QEvent::KeyRelease: {
			break;
		}
		}
	}
	return ImGuiPainter::eventFilter(watched, event);
}

QDebugSceneRenderPass::QDebugSceneRenderPass()
	: mDebugUIPainter(new DebugUIPainter)
{
}

IRenderComponent* QDebugSceneRenderPass::GetComponentByID(int ID) {
	return mRenderComponents.value(ID - 1);
}

int QDebugSceneRenderPass::GetID(IRenderComponent* inComponent) {
	return mRenderComponents.indexOf(inComponent) + 1;
}

QVector4D QDebugSceneRenderPass::getIDVec4(IRenderComponent* inComponent) {
	int ID = GetID(inComponent);
	int r = (ID & 0x000000FF) >> 0;
	int g = (ID & 0x0000FF00) >> 8;
	int b = (ID & 0x00FF0000) >> 16;
	int a = (ID & 0xFF000000) >> 24;
	return QVector4D(r, g, b, a) / 255.0f;;
}

QString QDebugSceneRenderPass::getIDText(IRenderComponent* inComponent) {
	QVector4D ID = getIDVec4(inComponent);
	return QString("vec4(%1,%2,%3,%4)").arg(ID.x()).arg(ID.y()).arg(ID.z()).arg(ID.w());
}

int QDebugSceneRenderPass::getSampleCount() {
	return 1;
}

QList<QPair<QRhiTexture::Format, QString>> QDebugSceneRenderPass::getRenderTargetSlots() {
	return { {QRhiTexture::RGBA32F,"BaseColor"},{QRhiTexture::RGBA8,"DebugID"} };
}

QRhiRenderPassDescriptor* QDebugSceneRenderPass::getRenderPassDescriptor() {
	return mRT.renderPassDesc.get();
}

QRhiRenderTarget* QDebugSceneRenderPass::getRenderTarget() {
	return mRT.renderTarget.get();
}

void QDebugSceneRenderPass::resizeAndLink(const QSize& size, const TextureLinker& linker) {
	mRT.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA32F, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atBaseColor->create();
	mRT.atDebugID.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mRT.atDebugID->create();
	mRT.atDepthStencil.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, size, 1));
	mRT.atDepthStencil->create();
	QRhiTextureRenderTargetDescription RTDesc;
	RTDesc.setColorAttachments({ 
		QRhiColorAttachment(mRT.atBaseColor.get()),
		QRhiColorAttachment(mRT.atDebugID.get()),
	});
	RTDesc.setDepthStencilBuffer(mRT.atDepthStencil.get());
	mRT.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
	mRT.renderPassDesc.reset(mRT.renderTarget->newCompatibleRenderPassDescriptor());
	mRT.renderTarget->setRenderPassDescriptor(mRT.renderPassDesc.get());
	mRT.renderTarget->create();

	mDebugUILayer.atBaseColor.reset(mRhi->newTexture(QRhiTexture::RGBA8, size, 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource));
	mDebugUILayer.atBaseColor->create();
	mDebugUILayer.atDepthStencil.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::Type::DepthStencil, size, 1));
	mDebugUILayer.atDepthStencil->create();
	RTDesc.setColorAttachments({
		QRhiColorAttachment(mDebugUILayer.atBaseColor.get()),
	});
	RTDesc.setDepthStencilBuffer(mDebugUILayer.atDepthStencil.get());
	mDebugUILayer.renderTarget.reset(mRhi->newTextureRenderTarget(RTDesc));
	mDebugUILayer.renderPassDesc.reset(mDebugUILayer.renderTarget->newCompatibleRenderPassDescriptor());
	mDebugUILayer.renderTarget->setRenderPassDescriptor(mDebugUILayer.renderPassDesc.get());
	mDebugUILayer.renderTarget->create();

	mDebugUIPainter->setupRhi(mRhi);
	if (QWindowRenderer* renderer = qobject_cast<QWindowRenderer*>(getRenderer())) {
		mDebugUIPainter->setupWindow(renderer->getWindow());
	}
	mDebugUIPainter->setupDebugRenderPass(this);
	mDebugUIPainter->setupDebugIDTexture(mRT.atDebugID.get());
	mDebugUIPainter->setupRenderPassDesc(mDebugUILayer.renderPassDesc.get());
	mDebugUIPainter->setupSampleCount(mDebugUILayer.renderTarget->sampleCount());
	mDebugUIPainter->compile();

	linker.setOutputTexture(0, "BaseColor", mRT.atBaseColor.get());
	linker.setOutputTexture(1, "DebugID", mRT.atDebugID.get());
	linker.setOutputTexture(2, "DebugUI", mDebugUILayer.atBaseColor.get());
}

void QDebugSceneRenderPass::compile() {
}

void QDebugSceneRenderPass::render(QRhiCommandBuffer* cmdBuffer) {
	ISceneRenderPass::render(cmdBuffer);
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	mDebugUIPainter->resourceUpdate(batch);
	cmdBuffer->beginPass(mDebugUILayer.renderTarget.get(), QColor::fromRgbF(0.0f, 0.0f, 0.0f, 0.0f), { 1.0f, 0 }, batch, QRhiCommandBuffer::ExternalContent);
	mDebugUIPainter->paint(cmdBuffer, mDebugUILayer.renderTarget.get());
	cmdBuffer->endPass();
}
