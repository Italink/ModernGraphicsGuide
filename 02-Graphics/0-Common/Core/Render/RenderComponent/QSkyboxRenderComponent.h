#ifndef QSkyboxRenderComponent_h__
#define QSkyboxRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "QDetailWidgetMacros.h"

class QSkyboxRenderComponent : public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QImage SkyBoxImage READ getSkyBoxImage WRITE setupSkyBoxImage);
public:
	QSkyboxRenderComponent();
	QSkyboxRenderComponent* setupSkyBoxImage(QImage inImage);
	QImage getSkyBoxImage() const;
protected:
	void recreateResource() override;
	void recreatePipeline() override;
	void uploadResource(QRhiResourceUpdateBatch* batch) override;
	void updatePrePass(QRhiCommandBuffer* cmdBuffer) override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QScopedPointer<QRhiTexture> mTexture;
	QScopedPointer<QRhiSampler> mSampler;
	QScopedPointer<QRhiShaderResourceBindings> mShaderResourceBindings;
	QImage mSkyBoxImage;
	std::array<QImage, 6> mSubImageList;
};

#endif // QSkyboxRenderComponent_h__