#ifndef QRhiUniform_h__
#define QRhiUniform_h__

#include <QVariant>
#include "RHI/QRhiEx.h"
#include "QList"

class IRenderComponent;

class QRhiUniform : public QObject{
	Q_OBJECT
public:
	QRhiUniform(QRhiShaderStage::Type inStage);
	QRhiUniform* addFloat(QString name, float var);
	QRhiUniform* addVec2(QString name, QVector2D var);
	QRhiUniform* addVec3(QString name, QVector3D var);
	QRhiUniform* addVec4(QString name, QVector4D var);
	void setFloat(QString name, float var);
	void setVec2(QString name, QVector2D var);
	void setVec3(QString name, QVector3D var);
	void setVec4(QString name, QVector4D var);
	bool renameParma(const QString& src, const QString& dst);
	void removeParam(const QString& name);
	void create(QRhiEx* inRhi);
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
		QRhiEx::Signal sigUpdateParam;
	};
	QRhiBuffer* getUniformBlock() const { return mUniformBlock.get(); }
	bool isEmpty()const { return mDataList.isEmpty(); }
	const QList<QSharedPointer<QRhiUniform::ParamMemoryDesc>>& getParamList() const { return mDataList; }
protected:
	void addParam(const QString& inName, ParamMemoryDesc::Type inType , QVariant inVar);
	QString getVaildName(QString name);
	void updateLayout();
protected:
	QList<QSharedPointer<ParamMemoryDesc>> mDataList;
	QHash<QString, QSharedPointer<ParamMemoryDesc>> mParamNameMap;
	uint32_t mDataSize;
	QScopedPointer<QRhiBuffer> mUniformBlock;
	QRhiShaderStage::Type mStage;
public:
	QRhiEx::Signal sigRecreateBuffer;
};
#endif // QRhiUniform_h_