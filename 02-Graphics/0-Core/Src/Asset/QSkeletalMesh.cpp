#include "QSkeletalMesh.h"
#include "QDir"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"
#include "QQueue"

QSharedPointer<QSkeleton::MeshNode> processSkeletonMeshNode(aiNode* node) {
	QSharedPointer<QSkeleton::MeshNode> boneNode = QSharedPointer<QSkeleton::MeshNode>::create();
	boneNode->name = node->mName.C_Str();
	boneNode->localTransform = AssetUtils::converter(node->mTransformation);
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		boneNode->children << (processSkeletonMeshNode(node->mChildren[i]));
	}
	return boneNode;
}

QSharedPointer<QSkeletalMesh> QSkeletalMesh::loadFromFile(const QString& inFilePath) {
	QSharedPointer<QSkeletalMesh> skeletalMesh;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(inFilePath.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene) {
		return skeletalMesh;
	}
	skeletalMesh = QSharedPointer<QSkeletalMesh>::create();
	const QVector<QMap<QString, QImage>>& matertialInfos = AssetUtils::loadMaterialsInfo(scene, inFilePath);
	QQueue<QPair<aiNode*, aiMatrix4x4>> qNode;
	qNode.push_back({ scene->mRootNode ,aiMatrix4x4() });
	skeletalMesh->mSkeleton = QSharedPointer<QSkeleton>::create();
	skeletalMesh->mSkeleton->mMeshRoot = processSkeletonMeshNode(scene->mRootNode);
	while (!qNode.isEmpty()) {
		QPair<aiNode*, aiMatrix4x4> node = qNode.takeFirst();
		for (unsigned int i = 0; i < node.first->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
			SubMeshInfo meshInfo;
			meshInfo.verticesOffset = skeletalMesh->mVertices.size();
			meshInfo.verticesRange = mesh->mNumVertices;
			skeletalMesh->mVertices.resize(meshInfo.verticesOffset + meshInfo.verticesRange);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex& vertex = skeletalMesh->mVertices[meshInfo.verticesOffset + i];
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
			meshInfo.indicesOffset = skeletalMesh->mIndices.size();
			meshInfo.indicesRange = 0;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					skeletalMesh->mIndices.push_back(face.mIndices[j]);
					meshInfo.indicesRange++;
				}
			}

			for (unsigned int i = 0; i < mesh->mNumBones; i++) {
				for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
					int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
					aiBone* bone = mesh->mBones[i];
					QSharedPointer<QSkeleton::BoneNode> boneNode;
					if (skeletalMesh->mSkeleton->mBoneMap.contains(bone->mName.C_Str()))
						boneNode = skeletalMesh->mSkeleton->mBoneMap[bone->mName.C_Str()];
					else {
						boneNode = QSharedPointer<QSkeleton::BoneNode>::create();
						boneNode->name = bone->mName.C_Str();
						boneNode->transformOffset = AssetUtils::converter(bone->mOffsetMatrix);
						boneNode->index = skeletalMesh->mSkeleton->mBoneOffsetMatrix.size();
						skeletalMesh->mSkeleton->mBoneOffsetMatrix << boneNode->transformOffset.toGenericMatrix<4, 4>();
						skeletalMesh->mSkeleton->mBoneMap[boneNode->name] = boneNode;
					}
					int slot = 0;
					Vertex& vertex = skeletalMesh->mVertices[meshInfo.verticesOffset + vertexId];
					while (slot < std::size(vertex.boneIndex) && (vertex.boneWeight[slot] > 0.000001)) {
						slot++;
					}
					if (slot < std::size(vertex.boneIndex)) {
						vertex.boneIndex[slot] = boneNode->index;
						vertex.boneWeight[slot] = mesh->mBones[i]->mWeights[j].mWeight;;
					}
					else {
						qWarning("Lack of slot");
					}
				}
			}
			meshInfo.materialInfo = matertialInfos[mesh->mMaterialIndex];
			skeletalMesh->mSubmeshes << meshInfo;
		}
		for (unsigned int i = 0; i < node.first->mNumChildren; i++) {
			qNode.push_back({ node.first->mChildren[i] ,node.second * node.first->mChildren[i]->mTransformation });
		}
	}
	skeletalMesh->resetPoses();
	return skeletalMesh;
}

void QSkeletalMesh::resetPoses() {
	mCurrentPosesMatrix.resize(mSkeleton->mBoneOffsetMatrix.size());
	QQueue<QPair<QSharedPointer<QSkeleton::MeshNode>, QMatrix4x4>> qNode;
	qNode.push_back({ mSkeleton->mMeshRoot ,QMatrix4x4() });
	while (!qNode.isEmpty()) {
		QPair<QSharedPointer<QSkeleton::MeshNode>, QMatrix4x4> node = qNode.takeFirst();
		QMatrix4x4 nodeMat = node.first->localTransform;
		QMatrix4x4 globalMatrix = node.second * nodeMat;
		auto boneIter = mSkeleton->mBoneMap.find(node.first->name);
		if (boneIter != mSkeleton->mBoneMap.end()) {
			const int& index = (*boneIter)->index;
			mCurrentPosesMatrix[index] = (globalMatrix * QMatrix4x4(mSkeleton->mBoneOffsetMatrix[index])).toGenericMatrix<4, 4>();
		}
		for (unsigned int i = 0; i < node.first->children.size(); i++) {
			qNode.push_back({ node.first->children[i] ,globalMatrix });
		}
	}
}

