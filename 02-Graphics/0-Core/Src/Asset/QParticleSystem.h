#ifndef QParticleSystem_h__
#define QParticleSystem_h__

#include "qvectornd.h"
#include "RHI\QRhiEx.h"

class QParticleSystem {
public:
	void setupRhi(QSharedPointer<QRhiEx> inRhi);
protected:
	inline static const int PARTICLE_MAX_SIZE = 1000000;
	struct Particle {
		QVector4D position;
		QVector4D rotation;
		QVector4D scaling = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
		QVector3D velocity;
		float life = 0.0;
	};
	virtual void recreateResource();
	virtual void updateResourcePrePass(QRhiResourceUpdateBatch* batch) {}
	virtual void onSpawnStage() = 0;
	virtual void onUpdateAndRecyleStage() = 0;
protected:
	QSharedPointer<QRhiEx> mRhi;
	QRhiCommandBuffer* mCurrCmdBuffer = nullptr;
	QScopedPointer<QRhiBuffer> mTransfromBuffer;
};

class QGPUParticleSystem: public QParticleSystem{
protected:
	void recreateResource() override;
	void updateResourcePrePass(QRhiResourceUpdateBatch* batch) override;
	void onSpawnStage() override;
	void onUpdateAndRecyleStage() override;
private:
	struct UpdateContext {
		int counter[2] = { 0,0 };
		int inputSlot = 0;
		int outputSlot = 1;
		float duration = 0.0f;
	}mUpdateContext;

	QScopedPointer<QRhiBuffer> mUpdateContextBuffer;

	QScopedPointer<QRhiBuffer> mParticlesBuffer[2];

	QScopedPointer<QRhiComputePipeline> mSpawnPipeline;
	QScopedPointer<QRhiShaderResourceBindings> mSpawnBindings[2];

	QScopedPointer<QRhiComputePipeline> mUpdatePipeline;
	QScopedPointer<QRhiShaderResourceBindings> mUpdateBindings[2];

	QScopedPointer<QRhiComputePipeline> mTranformComputePipline;
	QScopedPointer<QRhiShaderResourceBindings> mTranformComputeBindings[2];

	float mLastTimeSec = 0;
};



#endif // QParticleSystem_h__