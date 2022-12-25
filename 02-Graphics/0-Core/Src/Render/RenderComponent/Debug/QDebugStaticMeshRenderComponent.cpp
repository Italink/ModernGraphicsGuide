#include "QDebugStaticMeshRenderComponent.h"
#include "Render/RenderPass/QDebugSceneRenderPass.h"

void QDebugStaticMeshRenderComponent::onRebuildResource() {
	if (mStaticMesh.isNull())
		return;

	mVertexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::VertexBuffer, sizeof(QStaticMesh::Vertex) * mStaticMesh->mVertices.size()));
	mVertexBuffer->create();
	mIndexBuffer.reset(mRhi->newBuffer(QRhiBuffer::Type::Static, QRhiBuffer::IndexBuffer, sizeof(QStaticMesh::Index) * mStaticMesh->mIndices.size()));
	mIndexBuffer->create();

	for (auto& subMesh : mStaticMesh->mSubmeshes) {
		QRhiGraphicsPipelineBuilder* pipeline = new QRhiGraphicsPipelineBuilder(this);
		mPipelines << pipeline;
		pipeline->addUniformBlock(QRhiShaderStage::Vertex, "Transform")
			->addParam("MVP", QGenericMatrix<4, 4, float>())
			->addParam("M", QGenericMatrix<4, 4, float>());

		pipeline->setInputBindings({
			QRhiVertexInputBindingEx(mVertexBuffer.get(),sizeof(QStaticMesh::Vertex))
			});

		pipeline->setInputAttribute({
			QRhiVertexInputAttributeEx("inPosition"	,0, 0, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,position)),
			QRhiVertexInputAttributeEx("inNormal"	,0, 1, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,normal)),
			QRhiVertexInputAttributeEx("inTangent"	,0, 2, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,tangent)),
			QRhiVertexInputAttributeEx("inBitangent",0, 3, QRhiVertexInputAttribute::Float3, offsetof(QStaticMesh::Vertex,bitangent)),
			QRhiVertexInputAttributeEx("inUV"		,0, 4, QRhiVertexInputAttribute::Float2, offsetof(QStaticMesh::Vertex,texCoord))
			});

		pipeline->setShaderMainCode(QRhiShaderStage::Vertex, R"(
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
		QDebugSceneRenderPass* debugPass = qobject_cast<QDebugSceneRenderPass*>(sceneRenderPass());
		bool bHasDiffuse = subMesh.materialInfo.contains("Diffuse");
		if (bHasDiffuse) {
			pipeline->addTexture(QRhiShaderStage::Fragment, "Diffuse", subMesh.materialInfo["Diffuse"]);
		}
		pipeline->setShaderMainCode(QRhiShaderStage::Fragment, QString(R"(
		layout(location = 0) in vec2 vUV;
		layout(location = 1) in vec3 vWorldPosition;
		layout(location = 2) in mat3 vTangentBasis;
		void main(){
			BaseColor = %1;
			DebugID = %2;
		})").arg(bHasDiffuse?"texture(Diffuse,vUV)":"vec4(vTangentBasis[2],1)")
			.arg(debugPass->getIDText(this))
			.toLocal8Bit()
		);
	}
}

