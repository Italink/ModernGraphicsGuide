#ifndef QStaticMeshRenderComponent_h__
#define QStaticMeshRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"
#include "Asset/QStaticMesh.h"

class QStaticMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
	Q_PROPERTY(QString StaticMeshPath READ getStaticMeshPath WRITE setupStaticMeshPath)
		Q_META_BEGIN(QStaticMeshRenderComponent)
			Q_META_P_STRING_AS_FILE_PATH(StaticMeshPath)
		Q_META_END()
public:
	QStaticMeshRenderComponent(const QString& inStaticMeshPath = QString());
	QString getStaticMeshPath() const;
	QStaticMeshRenderComponent* setupStaticMeshPath(QString inPath);
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mStaticMeshPath;
	QSharedPointer<QStaticMesh> mStaticMesh;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QVector<QRhiGraphicsPipelineBuilder*> mPipelines;
};

#endif // QStaticMeshRenderComponent_h__
