#ifndef QParticleSystem_h__
#define QParticleSystem_h__

#include "qvectornd.h"
#include "RHI\QRhiEx.h"

class QParticleSystem {
public:
	enum class Type {
		CPU,
		GPU
	};
	virtual ~QParticleSystem() = default;
	virtual Type type() = 0;
	virtual void onInit(QSharedPointer<QRhiEx> inRhi);
	virtual void onTick(QRhiCommandBuffer* inCmdBuffer);

	int getNumOfSpawnPerFrame() const { return mNumOfSpawnPerFrame; }
	void setNumOfSpawnPerFrame(int val) { mNumOfSpawnPerFrame = val; }
	float getParticleLifetime() const { return mParticleLifetime; }
	void setParticleLifetime(float val) { mParticleLifetime = val; }
	bool isPrintCurrentNumOfParticle() const { return bPrintCurrentNumOfParticle; }
	void setPrintCurrentNumOfParticle(bool val) { bPrintCurrentNumOfParticle = val; }
	QRhiBuffer* getTransformBuffer() { return mTransfromBuffer.get(); }
protected:
	inline static const int PARTICLE_MAX_SIZE = 1000000;
	struct Particle {
		QVector4D position;
		QVector4D rotation;
		QVector4D scaling = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
		QVector3D velocity;
		float life = 0.0;
	};
	virtual void onSpawn(QRhiCommandBuffer* inCmdBuffer) = 0;
	virtual void onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer) = 0;
	virtual void onCalcAndSubmitTransform(QRhiCommandBuffer* inCmdBuffer) = 0;
protected:
	QSharedPointer<QRhiEx> mRhi;
	QScopedPointer<QRhiBuffer> mTransfromBuffer;

	bool bPrintCurrentNumOfParticle = true;
	int mNumOfSpawnPerFrame = 1000;
	float mParticleLifetime = 2.0f;

	float mDeltaSec = 0.0f;
	float mLastTimeSec = 0;
};

class QGPUParticleSystem: public QParticleSystem{
public:
	QGPUParticleSystem();
	void setSpawnShaderCode(QByteArray val) { mSpawnShaderCode = val; }
	void setUpdateShaderCode(QByteArray val) { mUpdateShaderCode = val; }
	QRhiBuffer* getCurrentIndirectBuffer();
	QRhiEx::Signal sigInitIndirectBuffer;
protected:
	virtual Type type() { return QParticleSystem::Type::GPU; }
	void onInit(QSharedPointer<QRhiEx> inRhi) override;
	void onRecreateComputePipeline();
	void onSpawn(QRhiCommandBuffer* inCmdBuffer) override;
	void onUpdateAndRecyle(QRhiCommandBuffer* inCmdBuffer) override;
	void onCalcAndSubmitTransform(QRhiCommandBuffer* inCmdBuffer) override;
private:
	struct IndirectDispatchBuffer {
		int x, y, z;
	};
	struct UpdateContextBuffer {
		float deltaSec;
		float lifetimeSec;
		float timestamp;
	};
	QScopedPointer<QRhiBuffer> mUpdateContextBuffer;

	QScopedPointer<QRhiBuffer> mIndirectDispatchBuffer[2];

	QScopedPointer<QRhiBuffer> mParticlesBuffer[2];
	QScopedPointer<QRhiComputePipeline> mSpawnPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mSpawnBindings[2];

	QScopedPointer<QRhiComputePipeline> mUpdatePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mUpdateBindings[2];

	QScopedPointer<QRhiComputePipeline> mTranformComputePipline;
	QScopedPointer<QRhiShaderResourceBindings> mTranformComputeBindings[2];

	int mInputSlot = 0;

	int mOutputSlot = 1;

	QByteArray mSpawnShaderCode;

	QByteArray mUpdateShaderCode;
};

#endif // QParticleSystem_h__