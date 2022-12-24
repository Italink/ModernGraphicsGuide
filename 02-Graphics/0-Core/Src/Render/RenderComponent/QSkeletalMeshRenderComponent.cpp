#include "QSkeletalMeshRenderComponent.h"
#include "Render/IRenderPass.h"

QSkeletalMeshRenderComponent::QSkeletalMeshRenderComponent(const QString& inStaticMeshPath) {
	setupStaticMeshPath(inStaticMeshPath);
}

QString QSkeletalMeshRenderComponent::getStaticMeshPath() const {
	return mStaticMeshPath;
}

QSkeletalMeshRenderComponent* QSkeletalMeshRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	mSkeletalMesh = QSkeletalMesh::loadFromFile(inPath);
	sigonRebuildResource.request();
	sigonRebuildPipeline.request();
	return this;
}

void QSkeletalMeshRenderComponent::onRebuildResource() {
	if (mSkeletalMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QSkeletalMesh::Vertex) * mSkeletalMesh->mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QSkeletalMesh::Index) * mSkeletalMesh->mIndices.size()));
	mIndexBuffer->create();

	mUniformBlock = QSharedPointer<QRhiUniformBlock>::create(QRhiShaderStage::Vertex);
	mUniformBlock->setObjectName("Transform");
	mUniformBlock->addParam("MVP", AssetUtils::Mat4())
		->addParam("M", AssetUtils::Mat4())
		->addParam("Bone", mSkeletalMesh->mCurrentPosesMatrix);

	mUniformBlock->create(mRhi.get());
	for (auto& mesh : mSkeletalMesh->mSubmeshes) {
		QRhiGraphicsPipelineBuilder* pipeline = new QRhiGraphicsPipelineBuilder(this);
		mPipelines << pipeline;
		pipeline->addUniformBlock(QRhiShaderStage::Vertex, mUniformBlock);
		pipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QSkeletalMesh::Vertex))
		});
		pipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QSkeletalMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QSkeletalMesh::Vertex,texCoord)),
			QRhiVertexInputAttributeEx("inBoneIndex",0, 5, QRhiVertexInputAttribute::UInt4, offsetof(QSkeletalMesh::Vertex,boneIndex)),
			QRhiVertexInputAttributeEx("inBoneWeight",0, 6, QRhiVertexInputAttribute::Float4, offsetof(QSkeletalMesh::Vertex,boneWeight))
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

void QSkeletalMeshRenderComponent::onRebuildPipeline() {
	for (auto pipeline : mPipelines) {
		pipeline->create(this);
	}
}

void QSkeletalMeshRenderComponent::onUpload(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mSkeletalMesh->mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mSkeletalMesh->mIndices.constData());
}

void QSkeletalMeshRenderComponent::onUpdate(QRhiResourceUpdateBatch* batch) {
	for (auto pipeline : mPipelines) {
		QMatrix4x4 MVP = calculateMatrixMVP();
		QMatrix4x4 M = calculateMatrixModel();
		pipeline->getUniformBlock("Transform")->setParamValue("MVP", MVP.toGenericMatrix<4,4>());
		pipeline->getUniformBlock("Transform")->setParamValue("M", M.toGenericMatrix<4, 4>());
		pipeline->getUniformBlock("Transform")->setParamValue("Bone", mSkeletalMesh->mCurrentPosesMatrix);
		pipeline->update(batch);
		if (pipeline->sigRebuild.receive()) {
			sigonRebuildPipeline.request();
		}
	}
}

void QSkeletalMeshRenderComponent::onRender(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i];
		const QSkeletalMesh::SubMeshInfo& meshInfo = mSkeletalMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset * sizeof(QSkeletalMesh::Vertex));
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset * sizeof(QSkeletalMesh::Index), QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}

bool QSkeletalMeshRenderComponent::isVaild() {
	return !mSkeletalMesh.isNull();
}
