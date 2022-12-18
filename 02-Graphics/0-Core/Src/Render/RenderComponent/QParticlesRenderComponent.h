#ifndef QParticlesRenderComponent_h__
#define QParticlesRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"
#include "Asset/QStaticMesh.h"

class QParticlesRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QString StaticMeshPath READ getStaticMeshPath WRITE setupStaticMeshPath)
		Q_META_BEGIN(QParticlesRenderComponent)
			Q_META_P_STRING_AS_FILE_PATH(StaticMeshPath)
		Q_META_END()
public:
	QParticlesRenderComponent(const QString& inStaticMeshPath = QString());
	QString getStaticMeshPath() const;
	QParticlesRenderComponent* setupStaticMeshPath(QString inPath);
protected:
	void recreateResource() override;
	void recreatePipeline() override;
	void uploadResource(QRhiResourceUpdateBatch* batch) override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mStaticMeshPath;
	QSharedPointer<QStaticMesh> mStaticMesh;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QVector<QRhiGraphicsPipelineBuilder*> mPipelines;
};

#endif // QParticlesRenderComponent_h__
