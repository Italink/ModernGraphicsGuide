#ifndef ImGuiPainter_h__
#define ImGuiPainter_h__

#include "QObject"
#include "imgui_internal.h"
#include "Render/IPainter.h"

class ImGuiPainter :public QObject ,public IPainter{
	Q_OBJECT
public:
	ImGuiPainter();

	void setupWindow(QWindow* window);

	void setupPaintFunctor(std::function<void()> val) { mPaintFunctor = val; }

	void compile() override;

	void paint(QRhiCommandBuffer* cmdBuffer, QRhiRenderTarget* renderTarget) override;

	void resourceUpdate(QRhiResourceUpdateBatch* batch) override;

protected:
	virtual bool eventFilter(QObject* watched, QEvent* event) override;

protected:
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;
	QScopedPointer<QRhiTexture> mFontTexture;
	QScopedPointer<QRhiSampler> mSampler;

	QWindow* mWindow;
	QImage mFontImage;
	ImGuiContext* mImGuiContext = nullptr;
	double       mTime = 0.0f;
	bool         mMousePressed[3] = { false, false, false };
	float        mMouseWheel = 0;
	float        mMouseWheelH = 0;
	std::function<void()> mPaintFunctor;
};

#endif // ImGuiPainter_h__