#ifndef QRhiUniformBlock_h__
#define QRhiUniformBlock_h__

#include <QVariant>
#include "RHI/QRhiEx.h"
#include "QList"

class IRenderComponent;

class QRhiUniformBlock : public QObject{
	Q_OBJECT
public:
	QRhiUniformBlock(QRhiShaderStage::Type inStage, QObject* inParent = nullptr);
	QRhiUniformBlock* addFloat(QString name, float var);
	QRhiUniformBlock* addVec2(QString name, QVector2D var);
	QRhiUniformBlock* addVec3(QString name, QVector3D var);
	QRhiUniformBlock* addVec4(QString name, QVector4D var);
	QRhiUniformBlock* addMat4(QString name, QGenericMatrix<4, 4, float> var);
	void setFloat(QString name, float var);
	void setVec2(QString name, QVector2D var);
	void setVec3(QString name, QVector3D var);
	void setVec4(QString name, QVector4D var);
	void setMat4(QString name, QGenericMatrix<4, 4, float> var);
	bool renameParma(const QString& src, const QString& dst);
	void removeParam(const QString& name);
	void create(QRhiEx* inRhi);
	void updateResource(QRhiResourceUpdateBatch* batch);
	struct ParamMemoryDesc{
		QString name;
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
	const QList<QSharedPointer<QRhiUniformBlock::ParamMemoryDesc>>& getParamList() const { return mDataList; }
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
#endif // QRhiUniformBlock_h_