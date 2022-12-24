#ifndef QParticlesRenderComponent_h__
#define QParticlesRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Asset/QParticleSystem.h"
#include "Utils/QColor4D.h"

class QParticlesRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QColor4D Color READ getColor WRITE setupColor)
public:
	QParticlesRenderComponent();
	QParticlesRenderComponent* setupColor(QColor4D val);
	QParticlesRenderComponent* setType(QParticleSystem::Type inType);

	QParticleSystem::Type getType();
	QColor4D getColor() const;
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onPreUpdate(QRhiCommandBuffer* cmdBuffer) override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QSharedPointer<QParticleSystem> mParticleSystem;
	QScopedPointer<QRhiBuffer> mIndirectDrawBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mBindings;

	struct IndirectDrawBuffer {
		quint32 vertexCount;
		quint32 instanceCount = 1;
		quint32 firstVertex = 0;
		quint32 firstInstance = 0;
	};
	struct UniformBlock {
		QGenericMatrix<4, 4, float> MVP;
		QGenericMatrix<4, 4, float> M;
		QVector4D color;
	};
	QColor4D mColor = QColor4D(0.1f, 0.5f, 0.9f, 1.0f);
};

#endif // QParticlesRenderComponent_h__
