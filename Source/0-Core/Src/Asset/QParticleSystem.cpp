#include "QParticleSystem.h"
#include "QDateTime"
#include "private/qrhivulkan_p.h"
#include "qvulkanfunctions.h"

void QParticleSystem::onInit(QSharedPointer<QRhiEx> inRhi){
	mRhi = inRhi;
	Q_ASSERT(!mRhi.isNull());
	mTransfromBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(float) * 16 * QParticleSystem::PARTICLE_MAX_SIZE));
	mTransfromBuffer->setName("TransfromBuffer");
	mTransfromBuffer->create();
}

void QParticleSystem::onTick(QRhiCommandBuffer* inCmdBuffer)
{
	float currentSecond = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
	if (mLastTimeSec == 0)
		mLastTimeSec = currentSecond;
	mDeltaSec = currentSecond - mLastTimeSec;
	mLastTimeSec = currentSecond;
	onSpawn(inCmdBuffer);
	mRhi->finish();
	onUpdateAndRecyle(inCmdBuffer);
	onCalcAndSubmitTransform(inCmdBuffer);
	mRhi->finish();
}

QGPUParticleSystem::QGPUParticleSystem()
{
	setSpawnShaderCode(R"(#version 450
	#define PARTICLE_MAX_SIZE 1000000
	struct Particle {
		vec3 position;
		vec3 rotation;
		vec3 scaling;
		vec3 velocity;
		float life;
	};
	layout(std140,binding = 0) buffer IndirectBuffer{
		ivec3 dispatch;
	};
	layout(std140,binding = 1) buffer ParticlesBuffer{
		Particle particles[PARTICLE_MAX_SIZE];
	};
	layout(std140,binding = 2) uniform UpdateContextBuffer{
		float deltaSec;
		float lifetimeSec;
		float timestamp;
	}UpdateCtx;

	float rand(float seed, float min, float max){
		return min + (max-min) * fract(sin(dot(vec2(gl_GlobalInvocationID.x * seed * UpdateCtx.deltaSec,UpdateCtx.timestamp) ,vec2(12.9898,78.233))) * 43758.5453);
	}
	#define Particle particles[ID]
	void main(){
		uint ID = atomicAdd(dispatch.x,1);
		Particle.life = 0.0f;
		Particle.scaling = vec3(rand(0.45,0.001,1));
		Particle.position = vec3(rand(0.121,-10,10),rand(0.415,-10,10),rand(0.456,-10,10));
		Particle.velocity = vec3(0.0,0,rand(0.1451,0.0001,0.1));
	})");

	setUpdateShaderCode(R"(#version 450
	#define PARTICLE_MAX_SIZE 1000000
	struct Particle {
		vec3 position;
		vec3 rotation;
		vec3 scaling;
		vec3 velocity;
		float life;
	};
	layout(std140,binding = 0) buffer OutputIndirectBuffer{
		ivec3 outputDispatch;
	};
	layout(std140,binding = 1) buffer InputParticlesBuffer{
		Particle inParticles[PARTICLE_MAX_SIZE];
	};
	layout(std140,binding = 2) buffer OutputParticlesBuffer{
		Particle outParticles[PARTICLE_MAX_SIZE];
	};
	layout(std140,binding = 3) uniform UpdateContextBuffer{
		float deltaSec;
		float lifetimeSec;
		float timestamp;
	}UpdateCtx;

	#define inParticle inParticles[inID]
	#define outParticle outParticles[outID]
	void main(){
		uint inID = gl_GlobalInvocationID.x;
		int isAlive = int(inParticles[inID].life < UpdateCtx.lifetimeSec);
		uint outID = atomicAdd(outputDispatch.x,isAlive);
		outID = max(PARTICLE_MAX_SIZE*(1-isAlive),outID);
		outParticle.life	 = inParticle.life + UpdateCtx.deltaSec;
		outParticle.position = inParticle.position + inParticle.velocity;
		outParticle.velocity = inParticle.velocity;
		outParticle.scaling  = inParticle.scaling;
		outParticle.rotation = inParticle.rotation;
	})");
}

QRhiBuffer* QGPUParticleSystem::getCurrentIndirectBuffer() {
	return mIndirectDispatchBuffer[mInputSlot].get();
}

void QGPUParticleSystem::onInit(QSharedPointer<QRhiEx> inRhi) {
	QParticleSystem::onInit(inRhi);
	mParticlesBuffer[0].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(QParticleSystem::Particle) * QParticleSystem::PARTICLE_MAX_SIZE));
	mParticlesBuffer[0]->setName("ParticlesBuffer0");
	mParticlesBuffer[0]->create();
	mParticlesBuffer[1].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(QParticleSystem::Particle) * QParticleSystem::PARTICLE_MAX_SIZE));
	mParticlesBuffer[1]->setName("ParticlesBuffer1");
	mParticlesBuffer[1]->create();
	mUpdateContextBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::UniformBuffer, sizeof(UpdateContextBuffer)));
	mUpdateContextBuffer->setName("UpdateContextBuffer");
	mUpdateContextBuffer->create();
	mIndirectDispatchBuffer[0].reset(mRhi->newVkBuffer(QRhiBuffer::Immutable, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(float) * 3));
	mIndirectDispatchBuffer[0]->setName("IndirectDispatchBuffer0");
	mIndirectDispatchBuffer[0]->create();
	mIndirectDispatchBuffer[1].reset(mRhi->newVkBuffer(QRhiBuffer::Immutable, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(float) * 3));
	mIndirectDispatchBuffer[1]->setName("IndirectDispatchBuffer1");
	mIndirectDispatchBuffer[1]->create();

	QVector<QRhiShaderResourceBinding> spawnBindings;
	spawnBindings << QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[0].get());
	spawnBindings << QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	spawnBindings << QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	mSpawnBindings[0].reset(mRhi->newShaderResourceBindings());
	mSpawnBindings[0]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[0]->create();
	spawnBindings[0] = QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[1].get());
	spawnBindings[1] = QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	mSpawnBindings[1].reset(mRhi->newShaderResourceBindings());
	mSpawnBindings[1]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[1]->create();

	QVector<QRhiShaderResourceBinding> updateBindings;
	updateBindings << QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[1].get());
	updateBindings << QRhiShaderResourceBinding::bufferLoad(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	updateBindings << QRhiShaderResourceBinding::bufferStore(2, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings << QRhiShaderResourceBinding::uniformBuffer(3, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	mUpdateBindings[0].reset(mRhi->newShaderResourceBindings());
	mUpdateBindings[0]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[0]->create();

	updateBindings[0] = QRhiShaderResourceBinding::bufferLoadStore(0, QRhiShaderResourceBinding::ComputeStage, mIndirectDispatchBuffer[0].get());
	updateBindings[1] = QRhiShaderResourceBinding::bufferLoad(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings[2] = QRhiShaderResourceBinding::bufferStore(2, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	mUpdateBindings[1].reset(mRhi->newShaderResourceBindings());
	mUpdateBindings[1]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[1]->create();

	mTranformComputePipline.reset(mRhi->newComputePipeline());
	mTranformComputeBindings[0].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[0]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
		});
	mTranformComputeBindings[0]->create();
	mTranformComputeBindings[1].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[1]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
		});
	mTranformComputeBindings[1]->create();

	mTranformComputePipline.reset(mRhi->newComputePipeline());
	mTranformComputePipline->setShaderResourceBindings(mTranformComputeBindings[0].get());
	QShader matrixCompute = QRhiEx::newShaderFromCode(QShader::ComputeStage, R"(
		#version 450
		#define PARTICLE_MAX_SIZE 1000000
		struct Particle {
			vec3 position;
			vec3 rotation;
			vec3 scaling;
			vec3 velocity;
			float life;
		};
		layout(std140,binding = 0) buffer InputParticle{
			Particle inParticles[PARTICLE_MAX_SIZE];
		};

		layout(std140,binding = 1) buffer OutputParticle{
			mat4 outMatrix[PARTICLE_MAX_SIZE];
		};

		void main(){
			const uint index = gl_GlobalInvocationID.x ;      //根据工作单元的位置换算出内存上的索引
			vec3 position = inParticles[index].position;
			vec3 rotation  = inParticles[index].rotation;
			vec3 scale    = inParticles[index].scaling;

			mat4 matScale = mat4(scale.x,0,0,0,
								 0,scale.y,0,0,
								 0,0,scale.z,0,
								 0,0,0,1);

	        vec2 X=vec2(sin(rotation.x),cos(rotation.x));
            vec2 Y=vec2(sin(rotation.y),cos(rotation.y));
            vec2 Z=vec2(sin(rotation.z),cos(rotation.z));

			mat4 matRotation=mat4(Z.y*Y.y,Z.y*Y.x*X.x-Z.x*X.y,Z.y*Y.x*X.y-Z.x*X.x,0,
                                  Z.x*Y.y,Z.x*Y.x*X.x+Z.y*X.y,Z.x*Y.x*X.y-Z.y*X.x,0,
                                  -Y.x,Y.y*X.x,Y.y*X.y,0,
                                  0,0,0,1
                                  );
			mat4 matTranslate=mat4(1,0,0,0,
								0,1,0,0,
								0,0,1,0,
								position.x,position.y,position.z,1);
			outMatrix[index] = matTranslate * matScale * matRotation;
		}
	)");
	mTranformComputePipline->setShaderStage({ QRhiShaderStage::Compute,matrixCompute });
	mTranformComputePipline->create();

	onRecreateComputePipeline();
}

void QGPUParticleSystem::onRecreateComputePipeline()
{
	mSpawnPipeline.reset(mRhi->newComputePipeline());
	mSpawnPipeline->setShaderResourceBindings(mSpawnBindings[mInputSlot].get());
	QShader spawnShader = QRhiEx::newShaderFromCode(QShader::Stage::ComputeStage, mSpawnShaderCode);
	mSpawnPipeline->setShaderStage({ QRhiShaderStage::Compute,spawnShader });
	mSpawnPipeline->create();

	mUpdatePipeline.reset(mRhi->newComputePipeline());
	mUpdatePipeline->setShaderResourceBindings(mUpdateBindings[mInputSlot].get());
	QShader updateShader = QRhiEx::newShaderFromCode(QShader::Stage::ComputeStage, mUpdateShaderCode);
	mUpdatePipeline->setShaderStage({ QRhiShaderStage::Compute,updateShader });
	mUpdatePipeline->create();

	sigInitIndirectBuffer.request();
}

void QGPUParticleSystem::onSpawn(QRhiCommandBuffer* inCmdBuffer) {
	if (sigInitIndirectBuffer.receive()) {
		IndirectDispatchBuffer dispatch = {0,1,1};
		QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
		batch->uploadStaticBuffer(mIndirectDispatchBuffer[0].get(),&dispatch);
		batch->uploadStaticBuffer(mIndirectDispatchBuffer[1].get(),&dispatch);
		inCmdBuffer->resourceUpdate(batch);
		mRhi->finish();
	}
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	UpdateContextBuffer updateCtx;
	updateCtx.deltaSec = mDeltaSec;
	updateCtx.lifetimeSec = mParticleLifetime;
	updateCtx.timestamp = QTime::currentTime().msecsSinceStartOfDay() / 1000000.0f;
	batch->updateDynamicBuffer(mUpdateContextBuffer.get(), 0, sizeof(UpdateContextBuffer), &updateCtx);
	inCmdBuffer->resourceUpdate(batch);

	inCmdBuffer->beginComputePass(nullptr, QRhiCommandBuffer::ExternalContent);
	inCmdBuffer->setComputePipeline(mSpawnPipeline.get());
	inCmdBuffer->setShaderResources(mSpawnBindings[mInputSlot].get());
	QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
	auto buffer = mIndirectDispatchBuffer[mInputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
	QVulkanInstance* vkInstance = mRhi->getVkInstance();
	vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDispatch(vkCmdBufferHandle->commandBuffer, mNumOfSpawnPerFrame, 1, 1);
	inCmdBuffer->endComputePass();

	//if (bPrintCurrentNumOfParticle) {
	//	static QRhiBufferReadbackResult mCtxReader;
	//	mCtxReader.completed = [this]() {
	//		Particle part[100];
	//		memcpy(&part, mCtxReader.data.constData(), mCtxReader.data.size());
	//		qDebug() << part[0].life;
	//	};
	//	batch = mRhi->nextResourceUpdateBatch();
	//	batch->readBackBuffer(mParticlesBuffer[mInputSlot].get(), 0, sizeof(Particle)*100, &mCtxReader);
	//	inCmdBuffer->resourceUpdate(batch);
	//	//batch = mRhi->nextResourceUpdateBatch();
	//	//batch->readBackBuffer(mIndirectDispatchBuffer[1].get(), 0, sizeof(float), &mCtxReader);
	//	//inCmdBuffer->resourceUpdate(batch);
	//}
	
}

void QGPUParticleSystem::onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer) {
	IndirectDispatchBuffer dispatch = { 0,1,1 };
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->uploadStaticBuffer(mIndirectDispatchBuffer[mOutputSlot].get(), &dispatch);
	inCmdBuffer->resourceUpdate(batch);

	inCmdBuffer->beginComputePass(nullptr, QRhiCommandBuffer::ExternalContent);
	inCmdBuffer->setComputePipeline(mUpdatePipeline.get());
	inCmdBuffer->setShaderResources(mUpdateBindings[mInputSlot].get());
	QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
	auto buffer = mIndirectDispatchBuffer[mInputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
	QVulkanInstance* vkInstance = mRhi->getVkInstance();
	vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDispatchIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0);
	inCmdBuffer->endComputePass();
}

void QGPUParticleSystem::onCalcAndSubmitTransform(QRhiCommandBuffer* inCmdBuffer)
{
	inCmdBuffer->beginComputePass(nullptr, QRhiCommandBuffer::ExternalContent);
	inCmdBuffer->setComputePipeline(mTranformComputePipline.get());
	inCmdBuffer->setShaderResources(mTranformComputeBindings[mOutputSlot].get());
	QRhiVulkanCommandBufferNativeHandles* vkCmdBufferHandle = (QRhiVulkanCommandBufferNativeHandles*)inCmdBuffer->nativeHandles();
	QRhiVulkanNativeHandles* vkHandles = (QRhiVulkanNativeHandles*)mRhi->nativeHandles();
	auto buffer = mIndirectDispatchBuffer[mOutputSlot]->nativeBuffer();
	VkBuffer vkBuffer = *(VkBuffer*)buffer.objects[0];
	QVulkanInstance* vkInstance = mRhi->getVkInstance();
	vkInstance->deviceFunctions(vkHandles->dev)->vkCmdDispatchIndirect(vkCmdBufferHandle->commandBuffer, vkBuffer, 0);
	inCmdBuffer->endComputePass();
	qSwap(mInputSlot, mOutputSlot);
}