#ifndef QCamera_h__
#define QCamera_h__

#include <QWindow>
#include "QQuaternion"
#include "QMatrix4x4"
#include "RHI\QRhiEx.h"

class QCamera :public QObject {
	Q_OBJECT
		Q_PROPERTY(QVector3D Position READ getPosition WRITE setPosition)
		Q_PROPERTY(QVector3D Rotation READ getRotation WRITE setRotation)
		Q_PROPERTY(float MoveSpeed READ getMoveSpeed WRITE setMoveSpeed)
		Q_PROPERTY(float RotationSpeed READ getRotationSpeed WRITE setRotationSpeed)
public:

	QCamera();
	QMatrix4x4 getViewMatrix();

	float getYaw();
	float getPitch();
	float getRoll();

	void setPosition(const QVector3D& newPosition);
	void setRotation(const QVector3D& newRotation);
	QVector3D getPosition() ;
	QVector3D getRotation();
	float getRotationSpeed() const { return mRotationSpeed; }
	void setRotationSpeed(float val) { mRotationSpeed = val; }
	float getMoveSpeed() const { return mMoveSpeed; }
	void setMoveSpeed(float val) { mMoveSpeed = val; }

	void setAspectRatio(float val);

	QMatrix4x4 getMatrixVPWithCorr(QRhiEx* inRhi);
	QMatrix4x4 getMatrixClip();
	QMatrix4x4 getMatrixView();
	void setupWindow(QWindow* window);
private:
	void calculateViewMatrix();
	void calculateClipMatrix();
	void calculateCameraDirection();
	bool eventFilter(QObject* watched, QEvent* event) override;

protected:
	QWindow* mWindow;
	QVector3D mPosition = QVector3D(0, 0, 2);
	QVector3D mRotation = QVector3D(0, M_PI / 2, 0);
private:
	QMatrix4x4 mViewMatrix;
	QMatrix4x4 mClipMatrix;

	float mFov = 45.0f;
	float mAspectRatio = 1.0;
	float mNearPlane = 0.01f;
	float mFarPlane = 10000.0f;

	QVector3D mCameraDirection;
	QVector3D mCameraUp;
	QVector3D mCameraRight;

	QSet<int> mKeySet;					     //记录当前被按下按键的集合
	int64_t mDeltaTimeMs = 0;				 //当前帧与上一帧的时间差
	int64_t mLastFrameTimeMs = 0;			 //上一帧的时间

	float mRotationSpeed = 0.003f;			 //鼠标灵敏度
	float mMoveSpeed = 0.1f;				 //控制移动速度
};

#endif // QCamera_h__
