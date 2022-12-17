#include "QSkeletalMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"
#include "QPair"
#include "QQueue"
#include "QDir"

QSkeletalMeshRenderComponent::QSkeletalMeshRenderComponent(const QString& inStaticMeshPath) {
	setupStaticMeshPath(inStaticMeshPath);
}

QString QSkeletalMeshRenderComponent::getStaticMeshPath() const {
	return mStaticMeshPath;
}

QSkeletalMeshRenderComponent* QSkeletalMeshRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	sigRecreateResource.request();
	sigRecreatePipeline.request();
	return this;
}

QSharedPointer<QSkeleton::MeshNode> processSkeletonMeshNode(aiNode* node) {
	QSharedPointer<QSkeleton::MeshNode> boneNode = QSharedPointer<QSkeleton::MeshNode>::create();
	boneNode->name = node->mName.C_Str();
	boneNode->localTransform = AssetUtils::converter(node->mTransformation);
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		boneNode->children << (processSkeletonMeshNode(node->mChildren[i]));
	}
	return boneNode;
}

void QSkeletalMeshRenderComponent::recreateResource() {
	if (mStaticMeshPath.isEmpty())
		return;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(mStaticMeshPath.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene) {
		return;
	}
	QVector<QMap<QString, QImage>> matertialInfos;
	static QStringList TextureNameMap = { "None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess","Opacity","Displacement","Lightmap","Reflection",
		"BaseColor","NormalCamera","EmissionColor","Metalness","DiffuseRoughnes","AmbientOcclusion",
		"Unknown","Sheen","Clearcoat","Transmission" };
	QDir dir = QFileInfo(mStaticMeshPath).dir();
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
					QString newPath = dir.filePath(name.mid(1, name.lastIndexOf('/') - 1));
					dir.setPath(newPath);
				}
				QString realPath = dir.filePath(path.C_Str());

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

	QQueue<QPair<aiNode*, aiMatrix4x4>> qNode;
	qNode.push_back({ scene->mRootNode ,aiMatrix4x4() });
	mVertices.clear();
	mIndices.clear();
	mSkeleton = QSharedPointer<QSkeleton>::create();
	mSkeleton->mMeshRoot = processSkeletonMeshNode(scene->mRootNode);
	while (!qNode.isEmpty()) {
		QPair<aiNode*, aiMatrix4x4> node = qNode.takeFirst();
		for (unsigned int i = 0; i < node.first->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
			MeshInfo meshInfo;
			meshInfo.verticesOffset = mVertices.size();
			meshInfo.verticesRange = mesh->mNumVertices;
			mVertices.resize(meshInfo.verticesOffset + meshInfo.verticesRange);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex& vertex = mVertices[meshInfo.verticesOffset + i];
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
			meshInfo.indicesOffset = mIndices.size();
			meshInfo.indicesRange = 0;
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					mIndices.push_back(face.mIndices[j]);
					meshInfo.indicesRange++;
				}
			}

			for (unsigned int i = 0; i < mesh->mNumBones; i++) {
				for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
					int vertexId = mesh->mBones[i]->mWeights[j].mVertexId;
					aiBone* bone = mesh->mBones[i];
					QSharedPointer<QSkeleton::BoneNode> boneNode;
					if (mSkeleton->mBoneMap.contains(bone->mName.C_Str()))
						boneNode = mSkeleton->mBoneMap[bone->mName.C_Str()];
					else {
						boneNode = QSharedPointer<QSkeleton::BoneNode>::create();
						boneNode->name = bone->mName.C_Str();
						boneNode->transformOffset = AssetUtils::converter(bone->mOffsetMatrix);
						boneNode->index = mSkeleton->mBoneOffsetMatrix.size();
						mSkeleton->mBoneOffsetMatrix << boneNode->transformOffset.toGenericMatrix<4, 4>();
						mSkeleton->mBoneMap[boneNode->name] = boneNode;
					}
					int slot = 0;
					Vertex& vertex = mVertices[meshInfo.verticesOffset + vertexId];
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
			mMeshes << meshInfo;
		}
		for (unsigned int i = 0; i < node.first->mNumChildren; i++) {
			qNode.push_back({ node.first->mChildren[i] ,node.second * node.first->mChildren[i]->mTransformation });
		}
	}
	
	mSkeleton->resetPoses();

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(Vertex) * mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(Index) * mIndices.size()));
	mIndexBuffer->create();

	mUniformBlock = QSharedPointer<QRhiUniformBlock>::create(QRhiShaderStage::Vertex);
	mUniformBlock->setObjectName("Transform");
	mUniformBlock->addParam("MVP", AssetUtils::Mat4())
		->addParam("M", AssetUtils::Mat4())
		->addParam("Bone", mSkeleton->mCurrentPosesMatrix);

	mUniformBlock->create(mRhi.get());
	for (auto& mesh : mMeshes) {
		QRhiGraphicsPipelineBuilder* pipeline = new QRhiGraphicsPipelineBuilder(this);
		mPipelines << pipeline;
		pipeline->addUniformBlock(QRhiShaderStage::Vertex, mUniformBlock);
		pipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(Vertex))
		});
		pipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(Vertex,texCoord)),
			QRhiVertexInputAttributeEx("inBoneIndex",0, 5, QRhiVertexInputAttribute::UInt4, offsetof(Vertex,boneIndex)),
			QRhiVertexInputAttributeEx("inBoneWeight",0, 6, QRhiVertexInputAttribute::Float4, offsetof(Vertex,boneWeight))
			});

		pipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vWorldPosition;
layout(location = 2) out mat3 vTangentBasis;
void main(){
	mat4 BoneTransform =  Transform.Bone[inBoneIndex[0]] * inBoneWeight[0];
			BoneTransform += Transform.Bone[inBoneIndex[1]] * inBoneWeight[1];
			BoneTransform += Transform.Bone[inBoneIndex[2]] * inBoneWeight[2];
			BoneTransform += Transform.Bone[inBoneIndex[3]] * inBoneWeight[3];
	vUV = inUV;
	vec4 pos = BoneTransform * vec4(inPosition,1.0f);
	vWorldPosition = vec3(Transform.M * pos);
	vTangentBasis =  mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
	gl_Position = Transform.MVP * pos;
}
)");
		if (mesh.materialInfo.contains("Diffuse")) {
			pipeline->addTexture(QRhiShaderStage::Fragment, "Diffuse", mesh.materialInfo["Diffuse"]);
			pipeline->setShaderMainCode(QRhiShaderStage::Fragment, R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			FragColor = texture(Diffuse,vUV);
		})");
		}
		else {
			pipeline->setShaderMainCode(QRhiShaderStage::Fragment, R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			FragColor = vec4(vTangentBasis[2],1);
		})");
		}
	}
}

void QSkeletalMeshRenderComponent::recreatePipeline() {
	for (auto pipeline : mPipelines) {
		pipeline->create(this);
	}
}

void QSkeletalMeshRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mIndices.constData());
}

void QSkeletalMeshRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
	for (auto pipeline : mPipelines) {
		QMatrix4x4 MVP = calculateMatrixMVP();
		QMatrix4x4 M = calculateMatrixModel();
		pipeline->getUniformBlock("Transform")->setParamValue("MVP", MVP.toGenericMatrix<4,4>());
		pipeline->getUniformBlock("Transform")->setParamValue("M", M.toGenericMatrix<4, 4>());
		pipeline->update(batch);
		if (pipeline->sigRebuild.receive()) {
			sigRecreatePipeline.request();
		}
	}
}

void QSkeletalMeshRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i];
		const MeshInfo& meshInfo = mMeshes[i];
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset, QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}

bool QSkeletalMeshRenderComponent::isVaild() {
	return !mMeshes.isEmpty();
}

void QSkeleton::resetPoses() {
	QVector<AssetUtils::Mat4> matrix(mBoneOffsetMatrix.size());
	QQueue<QPair<QSharedPointer<MeshNode>, QMatrix4x4>> qNode;
	qNode.push_back({ mMeshRoot ,QMatrix4x4() });
	while (!qNode.isEmpty()) {
		QPair<QSharedPointer<MeshNode>, QMatrix4x4> node = qNode.takeFirst();
		QMatrix4x4 nodeMat = node.first->localTransform;
		QMatrix4x4 globalMatrix = node.second * nodeMat;
		auto boneIter = mBoneMap.find(node.first->name);
		if (boneIter != mBoneMap.end()) {
			const int& index = (*boneIter)->index;
			matrix[index] = (globalMatrix * QMatrix4x4(mBoneOffsetMatrix[index])).toGenericMatrix<4, 4>();
		}
		for (unsigned int i = 0; i < node.first->children.size(); i++) {
			qNode.push_back({ node.first->children[i] ,globalMatrix });
		}
	}
	mCurrentPosesMatrix = matrix;
}
