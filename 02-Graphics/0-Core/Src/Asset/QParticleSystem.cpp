#include "QParticleSystem.h"
#include "QDateTime"

void QParticleSystem::setupRhi(QSharedPointer<QRhiEx> inRhi) {
	mRhi = inRhi;
	Q_ASSERT(!mRhi.isNull());
	recreateResource();
}

void QParticleSystem::recreateResource() {
	mTransfromBuffer.reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(float) * 16 * QParticleSystem::PARTICLE_MAX_SIZE));
	mTransfromBuffer->create();
}

void QGPUParticleSystem::recreateResource() {
	QParticleSystem::recreateResource();

	mParticlesBuffer[0].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(QParticleSystem::Particle) * QParticleSystem::PARTICLE_MAX_SIZE));
	mParticlesBuffer[0]->create();			

	mParticlesBuffer[1].reset(mRhi->newBuffer(QRhiBuffer::Static, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(QParticleSystem::Particle) * QParticleSystem::PARTICLE_MAX_SIZE));
	mParticlesBuffer[1]->create();

	mUpdateContextBuffer.reset(mRhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UsageFlag::VertexBuffer | QRhiBuffer::UsageFlag::StorageBuffer, sizeof(UpdateContext)));
	mUpdateContextBuffer->create();


	QVector<QRhiShaderResourceBinding> spawnBindings;
	spawnBindings << QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	spawnBindings << QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	spawnBindings << QRhiShaderResourceBinding::bufferLoadStore(2, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	mSpawnBindings[0]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[0]->create();

	spawnBindings[0] = QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	spawnBindings[1] = QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	mSpawnBindings[1]->setBindings(spawnBindings.begin(), spawnBindings.end());
	mSpawnBindings[1]->create();
	mSpawnPipeline->setShaderResourceBindings(mSpawnBindings[mUpdateContext.inputSlot].get());

	QShader spawnShader = QRhiEx::newShaderFromCode(QShader::Stage::VertexStage, R"()");
	mSpawnPipeline->setShaderStage({ QRhiShaderStage::Compute,spawnShader });
	mSpawnPipeline->create();

	QVector<QRhiShaderResourceBinding> updateBindings;
	updateBindings << QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	updateBindings << QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings << QRhiShaderResourceBinding::bufferLoadStore(2, QRhiShaderResourceBinding::ComputeStage, mUpdateContextBuffer.get());
	mUpdateBindings[0]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[0]->create();

	updateBindings[0] = QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get());
	updateBindings[1] = QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get());
	mUpdateBindings[1]->setBindings(updateBindings.begin(), updateBindings.end());
	mUpdateBindings[1]->create();
	mUpdatePipeline->setShaderResourceBindings(mUpdateBindings[mUpdateContext.inputSlot].get());

	QShader updateShader = QRhiEx::newShaderFromCode(QShader::Stage::VertexStage, R"()");
	mUpdatePipeline->setShaderStage({ QRhiShaderStage::Compute,updateShader });
	mUpdatePipeline->create();

	mTranformComputePipline.reset(mRhi->newComputePipeline());
	mTranformComputeBindings[0].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[0]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[1].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
	});
	mTranformComputeBindings[0]->create();

	mTranformComputeBindings[1].reset(mRhi->newShaderResourceBindings());
	mTranformComputeBindings[1]->setBindings({
		QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, mParticlesBuffer[0].get()),
		QRhiShaderResourceBinding::bufferStore(1, QRhiShaderResourceBinding::ComputeStage,mTransfromBuffer.get()),
	});
	mTranformComputeBindings[1]->create();

	mTranformComputePipline->setShaderResourceBindings(mTranformComputeBindings[0].get());
	QShader matrixCompute = QRhiEx::newShaderFromCode(QShader::ComputeStage, R"(
		#version 450
		#define LOCAL_SIZE 256
		#define PARTICLE_MAX_SIZE 1000000
		layout (local_size_x = LOCAL_SIZE) in;
		struct Particle {
			vec3 position;
			vec3 rotation;
			vec3 scaling;
			vec3 velocity;
			float life;
		};

		layout(std140,binding = 0) buffer InputParticle{
			Particle intputParticles[PARTICLE_MAX_SIZE];
		};

		layout(std140,binding = 1) buffer OutputParticle{
			mat4 outputMatrix[PARTICLE_MAX_SIZE];
		};

		void main(){
			const uint index = gl_GlobalInvocationID.x ;      //根据工作单元的位置换算出内存上的索引
			vec3 position = intputParticles[index].position;
			vec3 rotation  = intputParticles[index].rotation;
			vec3 scale    = intputParticles[index].scaling;

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
			outputMatrix[index] = matTranslate * matScale * matRotation;
		}
	)");
	mTranformComputePipline->setShaderStage({ QRhiShaderStage::Compute,matrixCompute });
	mTranformComputePipline->create();
}

void QGPUParticleSystem::onSpawnStage() {
	float currentSecond = QTime::currentTime().msecsSinceStartOfDay() / 1000.0;
	if (mLastTimeSec == 0)
		mLastTimeSec = currentSecond;
	mUpdateContext.duration = currentSecond - mLastTimeSec;
	mLastTimeSec = currentSecond;
	QRhiResourceUpdateBatch* batch = mRhi->nextResourceUpdateBatch();
	batch->updateDynamicBuffer(mUpdateContextBuffer.get(), 0, sizeof(UpdateContext), &mUpdateContext);

	mCurrCmdBuffer->beginComputePass(batch);
	mCurrCmdBuffer->setComputePipeline(mSpawnPipeline.get());
	mCurrCmdBuffer->setShaderResources(mSpawnBindings[mUpdateContext.inputSlot].get());
	mCurrCmdBuffer->dispatch(1, 1, 1);
	mCurrCmdBuffer->endComputePass();
}

void QGPUParticleSystem::onUpdateAndRecyleStage() {

	mCurrCmdBuffer->beginComputePass();
	mCurrCmdBuffer->setComputePipeline(mUpdatePipeline.get());
	mCurrCmdBuffer->setShaderResources(mUpdateBindings[mUpdateContext.inputSlot].get());
	mCurrCmdBuffer->dispatch(1, 1, 1);
	mCurrCmdBuffer->endComputePass();

	mCurrCmdBuffer->beginComputePass();
	mCurrCmdBuffer->setComputePipeline(mTranformComputePipline.get());
	mCurrCmdBuffer->setShaderResources(mTranformComputeBindings[mUpdateContext.inputSlot].get());
	mCurrCmdBuffer->dispatch(1, 1, 1);
	mCurrCmdBuffer->endComputePass();

	qSwap(mUpdateContext.inputSlot, mUpdateContext.outputSlot);
}

