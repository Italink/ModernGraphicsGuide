#ifndef QRhiUniform_h__
#define QRhiUniform_h__

#include <QVariant>
#include "RHI/QRhiEx.h"

class IRenderComponent;

class QRhiUniform{
public:
	QRhiUniform(QSharedPointer<QRhiEx> inRhi);
	void setDataFloat(QString name, float var);
	void setDataVec2(QString name, QVector2D var);
	void setDataVec3(QString name, QVector3D var);
	void setDataVec4(QString name, QVector4D var);
	bool renameParma(const QString& src, const QString& dst);
	void removeParam(const QString& name);
	void create();
	void updateResource(QRhiResourceUpdateBatch* batch);
	struct ParamMemoryDesc{
		QString name;
		QRhiShaderResourceBinding::StageFlag stage = QRhiShaderResourceBinding::StageFlag::FragmentStage;
		QVariant var;
		uint32_t offsetInByte;
		uint32_t sizeInByte;
		uint32_t sizeInByteAligned;
		enum Type {
			Float = 0,
			Vec2,
			Vec3,
			Vec4,
			Mat4,
		}type;
		QString getTypeName();
		QRhiEx::DirtySignal bNeedUpdate;
	};
protected:
	Q_DISABLE_COPY(QRhiUniform)
	QSharedPointer<ParamMemoryDesc> getOrCreateParam(QString name, ParamMemoryDesc::Type type);
	QString getVaildName(QString name);
	void updateLayout();
protected:
	QSharedPointer<QRhiEx> mRhi;
	QList<QSharedPointer<ParamMemoryDesc>> mDataList;
	QHash<QString, QSharedPointer<ParamMemoryDesc>> mParamNameMap;
	uint32_t mDataSize;
	QScopedPointer<QRhiBuffer> mUniformBlock;
public:
	QRhiEx::DirtySignal bNeedRecreate;
};
#endif // QRhiUniform_h_