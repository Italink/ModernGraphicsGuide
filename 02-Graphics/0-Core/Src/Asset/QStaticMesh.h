#ifndef QStaticMesh_h__
#define QStaticMesh_h__

#include "qvectornd.h"
#include "QImage"
#include "QMap"
#include "QMatrix4x4"

class QStaticMesh {
public:
	static QSharedPointer<QStaticMesh> loadFromFile(const QString& inFilePath);

	struct Vertex {
		QVector3D position;
		QVector3D normal;
		QVector3D tangent;
		QVector3D bitangent;
		QVector2D texCoord;
	};

	struct SubMeshInfo {
		uint32_t verticesOffset;
		uint32_t verticesRange;
		uint32_t indicesOffset;
		uint32_t indicesRange;
		QMap<QString, QImage> materialInfo;
		QMatrix4x4 localTransfrom;
	};
	using Index = uint32_t;

	QVector<Vertex> mVertices;
	QVector<Index> mIndices;
	QVector<SubMeshInfo> mSubmeshes;
};

#endif // QStaticMesh_h__
