#ifndef QSkeletalMeshRenderComponent_h__
#define QSkeletalMeshRenderComponent_h__

#include "ISceneRenderComponent.h"
#include "Render/QRhiGraphicsPipelineBuilder.h"
#include "Asset/AssetUtils.h"

struct QSkeleton {
	struct MeshNode {
		QString name;
		QMatrix4x4 localTransform;
		QVector<QSharedPointer<MeshNode>> children;
	};
	struct BoneNode {
		uint16_t index;
		QString name;
		QMatrix4x4 transformOffset;
	};
	void resetPoses();
	QSharedPointer<MeshNode> mMeshRoot;
	QHash<QString, QSharedPointer<BoneNode>> mBoneMap;
	QVector<AssetUtils::Mat4> mBoneOffsetMatrix;
	QVector<AssetUtils::Mat4> mCurrentPosesMatrix;
};

struct QSkeletonAnimation {
	struct AnimNode {
		QMap<double, QVector3D> translation;
		QMap<double, QQuaternion> rotation;
		QMap<double, QVector3D> scaling;
	};
	QMap<QString, AnimNode> mAnimNode;
	double mDuration;
	double mTicksPerSecond;
};

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
	using Index = uint32_t;
	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
		uint32_t boneIndex[4] = { 0,0,0,0 };
		QVector4D boneWeight;
	};

	struct MeshInfo {
		uint32_t verticesOffset;
		uint32_t verticesRange;
		uint32_t indicesOffset;
		uint32_t indicesRange;
		QMap<QString, QImage> materialInfo;
	};
protected:
	void recreateResource() override;
	void recreatePipeline() override;
	void uploadResource(QRhiResourceUpdateBatch* batch) override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) override;
	bool isVaild() override;
protected:
	QString mStaticMeshPath;
	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QVector<MeshInfo> mMeshes;
	QSharedPointer<QSkeleton> mSkeleton;

	QScopedPointer<QRhiBuffer> mVertexBuffer;
	QScopedPointer<QRhiBuffer> mIndexBuffer;
	QSharedPointer<QRhiUniformBlock> mUniformBlock;
	QVector<QRhiGraphicsPipelineBuilder*> mPipelines;
};

#endif // QSkeletalMeshRenderComponent_h__
