#ifndef AssetUtils_h__
#define AssetUtils_h__

#include "QGenericMatrix"
#include "assimp\vector3.h"
#include "assimp\matrix4x4.h"
#include "QMatrix4x4"
#include "assimp\scene.h"
#include "QDir"

namespace AssetUtils {

using Mat4 = QGenericMatrix<4, 4, float>;

QMatrix4x4 converter(const aiMatrix4x4& aiMat4);

QVector3D converter(const aiVector3D& aiVec3);

QVector<QMap<QString, QImage>> loadMaterialsInfo(const aiScene* scene, QString modelPath);
}


#endif // AssetUtils_h__
