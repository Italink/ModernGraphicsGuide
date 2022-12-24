#ifndef QSkeletalMeshRenderComponent_h__
#define QSkeletalMeshRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"
#include "Asset/QSkeletalMesh.h"

class QSkeletalMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QString StaticMeshPath READ getStaticMeshPath WRITE setupStaticMeshPath)
		Q_META_BEGIN(QSkeletalMeshRenderComponent)
		Q_META_P_STRING_AS_FILE_PATH(StaticMeshPath)
		Q_META_END()
public:
	QSkeletalMeshRenderComponent(const QString& inStaticMeshPath = QString());
	QString getStaticMeshPath() const;
	QSkeletalMeshRenderComponent* setupStaticMeshPath(QString inPath);
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mStaticMeshPath;

	QSharedPointer<QSkeletalMesh> mSkeletalMesh;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QRhiUniformBlock> mUniformBlock;
	QVector<QRhiGraphicsPipelineBuilder*> mPipelines;
};

#endif // QSkeletalMeshRenderComponent_h__
