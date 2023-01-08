#include "AssetUtils.h"
#include "QImage"

QMatrix4x4 AssetUtils::converter(const aiMatrix4x4& aiMat4) {
	QMatrix4x4 mat4;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mat4(i, j) = aiMat4[i][j];
		}
	}
	return mat4;
}

QVector3D AssetUtils::converter(const aiVector3D& aiVec3) {
	return QVector3D(aiVec3.x, aiVec3.y, aiVec3.z);
}

QVector<QMap<QString, QImage>> AssetUtils::loadMaterialsInfo(const aiScene* scene, QString modelPath) {
	QDir modelDir = QFileInfo(modelPath).dir();
	QVector< QMap<QString, QImage>> matertialInfos;
	static QStringList TextureNameMap = { "None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess","Opacity","Displacement","Lightmap","Reflection",
		"BaseColor","NormalCamera","EmissionColor","Metalness","DiffuseRoughnes","AmbientOcclusion",
		"Unknown","Sheen","Clearcoat","Transmission" };
	for (uint i = 0; i < scene->mNumMaterials; i++) {
		QMap<QString, QImage> materialInfo;
		aiMaterial* rawMaterial = scene->mMaterials[i];
		for (int i = aiTextureType_DIFFUSE; i < AI_TEXTURE_TYPE_MAX; i++) {
			int count = rawMaterial->GetTextureCount(aiTextureType(i));
			for (int j = 0; j < count; j++) {
				aiString path;
				rawMaterial->GetTexture(aiTextureType(i), j, &path);
				QString name = rawMaterial->GetName().C_Str();
				if (name.startsWith('/')) {
					QString newPath = modelDir.filePath(name.mid(1, name.lastIndexOf('/') - 1));
					modelDir.setPath(newPath);
				}
				QString realPath = modelDir.filePath(path.C_Str());

				QImage image;
				if (QFile::exists(realPath)) {
					image.load(realPath);
				}
				else {
					const aiTexture* embTexture = scene->GetEmbeddedTexture(path.C_Str());
					if (embTexture != nullptr) {
						if (embTexture->mHeight == 0) {
							image.loadFromData((uchar*)embTexture->pcData, embTexture->mWidth, embTexture->achFormatHint);
						}
						else {
							image = QImage((uchar*)embTexture->pcData, embTexture->mWidth, embTexture->mHeight, QImage::Format_ARGB32);
						}
					}
				}
				QString textureName = TextureNameMap[i];
				if (j != 0) {
					textureName += QString::number(j);
				}
				materialInfo[textureName] = image;
			}
		}
		matertialInfos << materialInfo;
	}
	return matertialInfos;
}
