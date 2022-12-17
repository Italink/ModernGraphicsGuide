#ifndef QSkeletalMesh_h__
#define QSkeletalMesh_h__

#include "QSharedPointer"
#include "qvectornd.h"
#include "QImage"
#include "QMap"
#include "QMatrix4x4"
#include "AssetUtils.h"

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
	QSharedPointer<MeshNode> mMeshRoot;
	QHash<QString, QSharedPointer<BoneNode>> mBoneMap;
	QVector<AssetUtils::Mat4> mBoneOffsetMatrix;
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

class QSkeletalMesh {
public:
	static QSharedPointer<QSkeletalMesh> loadFromFile(const QString& inFilePath);

	void resetPoses();
public:
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

	struct SubMeshInfo {
		uint32_t verticesOffset;
		uint32_t verticesRange;
		uint32_t indicesOffset;
		uint32_t indicesRange;
		QMap<QString, QImage> materialInfo;
	};

	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QVector<SubMeshInfo> mSubmeshes;

	QSharedPointer<QSkeleton> mSkeleton;
	QVector<AssetUtils::Mat4> mCurrentPosesMatrix;
};

#endif // QSkeletalMesh_h__