#include "QParticlesRenderComponent.h"
#include "Render/IRenderPass.h"

QParticlesRenderComponent::QParticlesRenderComponent(const QString& inStaticMeshPath) {
	setupStaticMeshPath(inStaticMeshPath);
}

QString QParticlesRenderComponent::getStaticMeshPath() const {
	return mStaticMeshPath;
}

QParticlesRenderComponent* QParticlesRenderComponent::setupStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	mStaticMesh = QStaticMesh::loadFromFile(inPath);
	sigRecreateResource.request();
	sigRecreatePipeline.request();
	return this;
}

void QParticlesRenderComponent::recreateResource() {
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
	if (subMesh.materialInfo.contains("Diffuse")) {
		pipeline->addTexture(QRhiShaderStage::Fragment, "Diffuse", subMesh.materialInfo["Diffuse"]);
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

void QParticlesRenderComponent::recreatePipeline() {
	for (auto& pipeline : mPipelines) {
		pipeline->create(this);
	}
}

void QParticlesRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
	batch->uploadStaticBuffer(mVertexBuffer.get(), mStaticMesh->mVertices.constData());
	batch->uploadStaticBuffer(mIndexBuffer.get(), mStaticMesh->mIndices.constData());
	
}

void QParticlesRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i];
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		QMatrix4x4 MVP = calculateMatrixMVP() * meshInfo.localTransfrom;
		QMatrix4x4 M = calculateMatrixModel() * meshInfo.localTransfrom;
		pipeline->getUniformBlock("Transform")->setParamValue("MVP", QVariant::fromValue(MVP.toGenericMatrix<4, 4>()));
		pipeline->getUniformBlock("Transform")->setParamValue("M", QVariant::fromValue(M.toGenericMatrix<4, 4>()));
		pipeline->update(batch);
		if (pipeline->sigRebuild.receive()) {
			sigRecreatePipeline.request();
		}
	}
}

void QParticlesRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
	for (int i = 0; i < mPipelines.size(); i++) {
		QRhiGraphicsPipelineBuilder* pipeline = mPipelines[i];
		const QStaticMesh::SubMeshInfo& meshInfo = mStaticMesh->mSubmeshes[i];
		cmdBuffer->setGraphicsPipeline(pipeline->getGraphicsPipeline());
		cmdBuffer->setViewport(viewport);
		cmdBuffer->setShaderResources();
		const QRhiCommandBuffer::VertexInput vertexBindings(mVertexBuffer.get(), meshInfo.verticesOffset * sizeof (QStaticMesh::Vertex));
		cmdBuffer->setVertexInput(0, 1, &vertexBindings, mIndexBuffer.get(), meshInfo.indicesOffset * sizeof(QStaticMesh::Index), QRhiCommandBuffer::IndexUInt32);
		cmdBuffer->drawIndexed(meshInfo.indicesRange);
	}
}

bool QParticlesRenderComponent::isVaild() {
	return !mStaticMesh.isNull();
}
