#ifndef QStaticMeshRenderComponent_h__
#define QStaticMeshRenderComponent_h__

#include "ISceneRenderComponent.h"

class QStaticMeshRenderComponent :public ISceneRenderComponent {
	Q_OBJECT
public:
public:
	QString getStaticMeshPath() const { return mStaticMeshPath; }
	QStaticMeshRenderComponent* setupStaticMeshPath(QString inPath);
protected:
	void recreateResource() override;
	void recreatePipeline() override;
	void uploadResource(QRhiResourceUpdateBatch* batch) override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QScopedPointer<QRhiGraphicsPipeline> mPipeline;
	QScopedPointer<QRhiBuffer> mUniformBuffer;
	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QScopedPointer<QRhiShaderResourceBindings> mShaderResourceBindings;

	struct UniformMatrix {
		QGenericMatrix<4, 4, float> MVP;
		QGenericMatrix<4, 4, float> M;
	};
	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
	};
	using Index = uint32_t;
	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QString mStaticMeshPath;
};
#endif // QStaticMeshRenderComponent_h__
