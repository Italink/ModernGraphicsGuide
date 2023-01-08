#include "QStaticMesh.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"
#include "QDir"
#include "AssetUtils.h"
#include "QQueue"

QSharedPointer<QStaticMesh> QStaticMesh::loadFromFile(const QString& inFilePath) {
	QSharedPointer<QStaticMesh> staticMesh;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inFilePath.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene) {
		return staticMesh;
	}
	staticMesh = QSharedPointer<QStaticMesh>::create();
	const QVector<QMap<QString, QImage>>& matertialInfos = AssetUtils::loadMaterialsInfo(scene, inFilePath);
	QQueue<QPair<aiNode*, aiMatrix4x4>> qNode;
	qNode.push_back({ scene->mRootNode ,aiMatrix4x4() });
	while (!qNode.isEmpty()) {
		QPair<aiNode*, aiMatrix4x4> node = qNode.takeFirst();
		for (unsigned int i = 0; i < node.first->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
			SubMeshInfo meshInfo;
			meshInfo.verticesOffset = staticMesh->mVertices.size();
			meshInfo.verticesRange = mesh->mNumVertices;
			meshInfo.localTransfrom = AssetUtils::converter(node.second);
			meshInfo.materialInfo = matertialInfos[mesh->mMaterialIndex];
			staticMesh->mVertices.resize(meshInfo.verticesOffset + meshInfo.verticesRange);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				QStaticMesh::Vertex& vertex = staticMesh->mVertices[meshInfo.verticesOffset + i];
				vertex.position = AssetUtils::converter(mesh->mVertices[i]);
				if (mesh->mNormals)
					vertex.normal = AssetUtils::converter(mesh->mNormals[i]);
				if (mesh->mTextureCoords[0]) {
					vertex.texCoord.setX(mesh->mTextureCoords[0][i].x);
					vertex.texCoord.setY(mesh->mTextureCoords[0][i].y);
				}
				if (mesh->mTangents)
					vertex.tangent = AssetUtils::converter(mesh->mTangents[i]);
				if (mesh->mBitangents)
					vertex.bitangent = AssetUtils::converter(mesh->mBitangents[i]);
			}
			meshInfo.indicesOffset = staticMesh->mIndices.size();
			meshInfo.indicesRange = 0;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					staticMesh->mIndices.push_back(face.mIndices[j]);
					meshInfo.indicesRange++;
				}
			}
			staticMesh->mSubmeshes << meshInfo;
		}
		for (unsigned int i = 0; i < node.first->mNumChildren; i++) {
			qNode.push_back({ node.first->mChildren[i] ,node.second * node.first->mChildren[i]->mTransformation });
		}
	}
	return staticMesh;
}

