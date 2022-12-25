#ifndef QSkeletalMeshRenderComponent_h__
#define QSkeletalMeshRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"
#include "Asset/QSkeletalMesh.h"

class QSkeletalMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
		Q_PROPERTY(QString SkeletalMeshPath READ getSkeletalMeshPath WRITE setupSkeletalMeshPath)
		Q_META_BEGIN(QSkeletalMeshRenderComponent)
		Q_META_P_STRING_AS_FILE_PATH(SkeletalMeshPath)
		Q_META_END()
public:
	QSkeletalMeshRenderComponent(const QString& inSkeletalMeshPath = QString());
	QString getSkeletalMeshPath() const;
	QSkeletalMeshRenderComponent* setupSkeletalMeshPath(QString inPath);
protected:
	void onRebuildResource() override;
	void onRebuildPipeline() override;
	void onUpload(QRhiResourceUpdateBatch* batch) override;
	void onUpdate(QRhiResourceUpdateBatch* batch) override;
	void onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mSkeletalMeshPath;

	QSharedPointer<QSkeletalMesh> mSkeletalMesh;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QRhiUniformBlock> mUniformBlock;
	QVector<QRhiGraphicsPipelineBuilder*> mPipelines;
};

#endif // QSkeletalMeshRenderComponent_h__
