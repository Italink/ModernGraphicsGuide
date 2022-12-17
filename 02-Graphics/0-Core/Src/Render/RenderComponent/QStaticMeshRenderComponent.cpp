#include "QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/matrix4x4.h"
#include "QPair"
#include "QQueue"
#include "QDir"

QMatrix4x4 converter(const aiMatrix4x4& aiMat4) {
	QMatrix4x4 mat4;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mat4(i, j) = aiMat4[i][j];
		}
	}
	return mat4;
}

QVector3D converter(const aiVector3D& aiVec3) {
	return QVector3D(aiVec3.x, aiVec3.y, aiVec3.z);
}

QStaticMeshRenderComponent::QStaticMeshRenderComponent(const QString& inStaticMeshPath) {
	setupStaticMeshPath(inStaticMeshPath);
}

QString QStaticMeshRenderComponent::getStaticMeshPath() const {
	return mStaticMeshPath;
}

QStaticMeshRenderComponent* QStaticMeshRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	sigRecreateResource.request();
	sigRecreatePipeline.request();
	return this;
}

void QStaticMeshRenderComponent::recreateResource() {
	if (mStaticMeshPath.isEmpty())
		return;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(mStaticMeshPath.toUtf8().constData(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
	if (!scene) {
		return;
	}
	mSubMeshes.clear();
	QVector<QStaticSubMesh::MaterialInfo> matertialInfos;
	static QStringList TextureNameMap = { "None","Diffuse","Specular","Ambient","Emissive","Height","Normals","Shininess","Opacity","Displacement","Lightmap","Reflection",
		"BaseColor","NormalCamera","EmissionColor","Metalness","DiffuseRoughnes","AmbientOcclusion",
		"Unknown","Sheen","Clearcoat","Transmission" };
	QDir dir = QFileInfo(mStaticMeshPath).dir();
	for (uint i = 0; i < scene->mNumMaterials; i++) {
		QStaticSubMesh::MaterialInfo materialInfo;
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
	while (!qNode.isEmpty()) {
		QPair<aiNode*, aiMatrix4x4> node = qNode.takeFirst();
		for (unsigned int i = 0; i < node.first->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node.first->mMeshes[i]];
			QStaticSubMesh* subMesh = new QStaticSubMesh;
			subMesh->setParent(this);
			subMesh->mLocalTransfrom = converter(node.second);
			subMesh->mMaterialInfo = matertialInfos[mesh->mMaterialIndex];
			subMesh->mVertices.resize(mesh->mNumVertices);
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				QStaticSubMesh::Vertex& vertex = subMesh->mVertices[i];
				vertex.position = converter(mesh->mVertices[i]);
				if (mesh->mNormals)
					vertex.normal = converter(mesh->mNormals[i]);
				if (mesh->mTextureCoords[0]) {
					vertex.texCoord.setX(mesh->mTextureCoords[0][i].x);
					vertex.texCoord.setY(mesh->mTextureCoords[0][i].y);
				}
				if (mesh->mTangents)
					vertex.tangent = converter(mesh->mTangents[i]);
				if (mesh->mBitangents)
					vertex.bitangent = converter(mesh->mBitangents[i]);
			}

			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					subMesh->mIndices.push_back(face.mIndices[j]);
				}
			}
			mSubMeshes << subMesh;
		}
		for (unsigned int i = 0; i < node.first->mNumChildren; i++) {
			qNode.push_back({ node.first->mChildren[i] ,node.second * node.first->mChildren[i]->mTransformation });
		}
	}

	for (auto& subMesh : mSubMeshes) {
		subMesh->mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticSubMesh::Vertex) * subMesh->mVertices.size()));
		subMesh->mVertexBuffer->create();
		subMesh->mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticSubMesh::Index) * subMesh->mIndices.size()));
		subMesh->mIndexBuffer->create();

		subMesh->mPipeline.reset(new QRhiGraphicsPipelineBuilder);
		subMesh->mPipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addMat4("MVP", QGenericMatrix<4, 4, float>())
			->addMat4("M", QGenericMatrix<4, 4, float>())
			;

		subMesh->mPipeline->addUniformBlock(QRhiShaderStage::Fragment, "Material")
			->addVec4("Color", QVector4D(1, 0.0, 0, 1));

		subMesh->mPipeline->setInputBindings({
			QRhiVertexInputBindingEx(subMesh->mVertexBuffer.get(),sizeof(QStaticSubMesh::Vertex))
			});

		subMesh->mPipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticSubMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticSubMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticSubMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticSubMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticSubMesh::Vertex,texCoord))
		});

		subMesh->mPipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
layout(location = 0) out vec2 vUV;
layout(location = 1) out vec3 vWorldPosition;
layout(location = 2) out mat3 vTangentBasis;
void main(){
	vUV = inUV;
	vWorldPosition = vec3(Transform.M * vec4(inPosition,1.0f));
	vTangentBasis =  mat3(Transform.M) * mat3(inTangent, inBitangent, inNormal);
	gl_Position = Transform.MVP * vec4(inPosition,1.0f);
}
)");
	if (subMesh->mMaterialInfo.contains("Diffuse")) {
		subMesh->mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			FragColor = vec4(vec3(0.5+dot(vTangentBasis[2],vec3(0,1,0))),1.0f);
		})");
	}
	else {
		subMesh->mPipeline->setShaderMainCode(QRhiShaderStage::Fragment, R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			FragColor = vec4(vTangentBasis[2],1);
		})");
	}
	}
}

void QStaticMeshRenderComponent::recreatePipeline() {
	for (auto& subMesh : mSubMeshes) {
		subMesh->mPipeline->create(this);
	}
}

void QStaticMeshRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
	for (auto& subMesh : mSubMeshes) {
		batch->uploadStaticBuffer(subMesh->mVertexBuffer.get(), subMesh->mVertices.constData());
		batch->uploadStaticBuffer(subMesh->mIndexBuffer.get(), subMesh->mIndices.constData());
	}
}

void QStaticMeshRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
	for (auto& subMesh : mSubMeshes) {
		QMatrix4x4 MVP = calculateMatrixMVP() * subMesh->mLocalTransfrom;
		QMatrix4x4 M = calculateMatrixModel() * subMesh->mLocalTransfrom;

		subMesh->mPipeline->getUniformBlock("Transform")->setMat4("MVP", MVP.toGenericMatrix<4,4>());
		subMesh->mPipeline->getUniformBlock("Transform")->setMat4("M", M.toGenericMatrix<4,4>());
		subMesh->mPipeline->update(batch);
		if (subMesh->mPipeline->sigRebuild.receive()) {
			sigRecreatePipeline.request();
		}
	}
}

void QStaticMeshRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (auto& subMesh : mSubMeshes) {
		cmdBuffer->setGraphicsPipeline(subMesh->mPipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(subMesh->mVertexBuffer.get(), 0);
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, subMesh->mIndexBuffer.get(), 0, QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(subMesh->mIndices.size());
	}
}

bool QStaticMeshRenderComponent::isVaild() {
	return !mSubMeshes.isEmpty();
}
