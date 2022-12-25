#include "QDebugSkeletalMeshRenderComponent.h"
#include "Render/RenderPass/QDebugSceneRenderPass.h"

void QDebugSkeletalMeshRenderComponent::onRebuildResource() {
	if (mSkeletalMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QSkeletalMesh::Vertex) * mSkeletalMesh->mVertices.size()));
	mVertexBuffer->create();

	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QSkeletalMesh::Index) * mSkeletalMesh->mIndices.size()));
	mIndexBuffer->create();

	mUniformBlock = QSharedPointer<QRhiUniformBlock>::create(QRhiShaderStage::Vertex);
	mUniformBlock->setObjectName("Transform");
	mUniformBlock->addParam("MVP", MathUtils::Mat4())
		->addParam("M", MathUtils::Mat4())
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

		QDebugSceneRenderPass* debugPass = qobject_cast<QDebugSceneRenderPass*>(sceneRenderPass());
		bool bHasDiffuse = mesh.materialInfo.contains("Diffuse");
		if (bHasDiffuse) {
			pipeline->addTexture(QRhiShaderStage::Fragment, "Diffuse", mesh.materialInfo["Diffuse"]);
		}
		pipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			BaseColor = %1;
			DebugID = %2;
		})").arg(bHasDiffuse ? "texture(Diffuse,vUV)" : "vec4(vTangentBasis[2],1)")
			.arg(debugPass->getIDText(this))
			.toLocal8Bit()
		);
	}
}

