#ifndef QStaticMeshRenderComponent_h__
#define QStaticMeshRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"

class QStaticSubMesh: public QObject{
	Q_OBJECT
	Q_PROPERTY_DEC(QMatrix4x4, mLocalTransfrom)
	Q_PROPERTY(QRhiGraphicsPipelineBuilder* Pipeline READ getPipeline WRITE setPipeline)
public:
	QRhiGraphicsPipelineBuilder* getPipeline() const { return mPipeline.get(); }
	void setPipeline(QRhiGraphicsPipelineBuilder* ) {}
	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
	};
	using Index = uint32_t;
	QMatrix4x4 mLocalTransfrom;
	QMap<QString, QImage> mMaterialInfo;
	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QRhiGraphicsPipelineBuilder> mPipeline;
};

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
	void recreateResource() override;
	void recreatePipeline() override;
	void uploadResource(QRhiResourceUpdateBatch* batch) override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mStaticMeshPath;
	QVector<QStaticSubMesh*> mSubMeshes;
};

#endif // QStaticMeshRenderComponent_h__
