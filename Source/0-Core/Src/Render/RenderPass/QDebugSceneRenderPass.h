#ifndef QDebugSceneRenderPass_h__
#define QDebugSceneRenderPass_h__

#include "Render/IRenderPass.h"
#include "Render/Painter/ImGuiPainter.h"
#include "ImGuizmo.h"

class QDebugSceneRenderPass;

class DebugUIPainter :public ImGuiPainter {
	Q_OBJECT
public:
	DebugUIPainter();
	void setupDebugRenderPass(QDebugSceneRenderPass* pass);
	void setupDebugIDTexture(QRhiTexture* texture);
	void resourceUpdate(QRhiResourceUpdateBatch* batch) override;
	void compile() override;
	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;
protected:
	bool eventFilter(QObject* watched, QEvent* event) override;
private:
	QRhiTexture* mDebugIDTexture = nullptr;
	QDebugSceneRenderPass* mDebugScenePass = nullptr;
	ImGuizmo::OPERATION mOpt = ImGuizmo::OPERATION::TRANSLATE;
	QRhiReadbackResult mReadReult;
	QRhiReadbackDescription mReadDesc;
	QPoint mReadPoint;

	QScopedPointer<QRhiGraphicsPipeline> mOutlinePipeline;
	QScopedPointer<QRhiSampler> mOutlineSampler;
	QScopedPointer<QRhiShaderResourceBindings> mOutlineBindings;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
};

class QDebugSceneRenderPass : public ISceneRenderPass {
	Q_OBJECT
	friend class DebugUIPainter;
public:
	enum  OutSlot {
		BaseColor,
		DebugID,
		DebugUI
	};
	QDebugSceneRenderPass();
	IRenderComponent* GetComponentByID(int ID);
	int GetID(IRenderComponent* inComponent);
	QString getIDText(IRenderComponent* inComponent);
	QVector4D getIDVec4(IRenderComponent* inComponent);
protected:
	struct RT {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiTexture> atDebugID;
		QScopedPointer<QRhiRenderBuffer> atDepthStencil;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};
	struct DebugUIRT {
		QScopedPointer<QRhiTexture> atBaseColor;
		QScopedPointer<QRhiRenderBuffer> atDepthStencil;
		QScopedPointer<QRhiTextureRenderTarget> renderTarget;
		QScopedPointer<QRhiRenderPassDescriptor> renderPassDesc;
	};

	int getSampleCount() override;
	QList<QPair<QRhiTexture::Format, QString>> getRenderTargetSlots();;
	QRhiRenderPassDescriptor* getRenderPassDescriptor() override;
	QRhiRenderTarget* getRenderTarget() override;

	void resizeAndLink(const QSize& size, const TextureLinker& linker) override;
	void compile() override;
	void render(QRhiCommandBuffer* cmdBuffer) override;
protected:
	RT mRT;
	DebugUIRT mDebugUILayer;
	QScopedPointer<DebugUIPainter> mDebugUIPainter;
};

#endif // QDebugSceneRenderPass_h__