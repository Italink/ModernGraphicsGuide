#include "QStaticMeshRenderComponent.h"
#include "Render/IRenderPass.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

QStaticMeshRenderComponent::QStaticMeshRenderComponent(const QString& inStaticMeshPath) {
	setStaticMeshPath(inStaticMeshPath);
}

QStaticMeshRenderComponent* QStaticMeshRenderComponent::setStaticMeshPath(QString inPath) {
	mStaticMeshPath = inPath;
	sigRecreateResource.request();
	sigRecreatePipeline.request();
	return this;
}

void QStaticMeshRenderComponent::recreateResource() {
}

void QStaticMeshRenderComponent::recreatePipeline() {
}

void QStaticMeshRenderComponent::uploadResource(QRhiResourceUpdateBatch* batch) {
}

void QStaticMeshRenderComponent::updateResourcePrePass(QRhiResourceUpdateBatch* batch) {
}

void QStaticMeshRenderComponent::renderInPass(QRhiCommandBuffer* cmdBuffer, const QRhiViewport& viewport) {
}

bool QStaticMeshRenderComponent::isVaild() {
	return !mSubMeshes.isEmpty();
}