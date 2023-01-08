#ifndef QRhiUniformBlock_h__
#define QRhiUniformBlock_h__

#include <QVariant>
#include "RHI/QRhiEx.h"
#include "QList"

class IRenderComponent;

struct UniformParamDescBase {
	virtual const char* typeName() = 0;
	virtual int dataByteSize() = 0;
	virtual int dataAlignSize() = 0;
	virtual void* dataPtr() { return mValue.data(); }
	virtual void setValue(QVariant inValue) {mValue = inValue;}
	virtual QString valueName() { return mName; }
	QString mName;
	QVariant mValue;
	uint32_t mOffsetInByte;
	uint32_t mSizeInByte;
	uint32_t mSizeInByteAligned;
	QRhiEx::Signal sigUpdate;
};

template<typename _Ty>
struct UniformParamDesc : public UniformParamDescBase {
};


class QRhiUniformBlock : public QObject{
	Q_OBJECT
public:
	QRhiUniformBlock(QRhiShaderStage::Type inStage, QObject* inParent = nullptr);
	template<typename _Ty>
	QRhiUniformBlock* addParam(const QString& name, _Ty value) {
		QSharedPointer<UniformParamDesc<_Ty>> paramDesc = QSharedPointer<UniformParamDesc<_Ty>>::create();
		paramDesc->mName = name;
		paramDesc->setValue(QVariant::fromValue<>(value));
		mParamList << paramDesc;
		mParamNameMap[name] = paramDesc;
		paramDesc->sigUpdate.request();
		return this;
	}
	template<typename _Ty, std::enable_if_t<!std::is_same_v<_Ty, QVariant>, bool> = true>
	void setParamValue(const QString& name, _Ty value) {
		setParamValue(name, QVariant::fromValue(value));
	}
	void setParamValue(const QString& mName, QVariant mValue);
	bool renameParma(const QString& src, const QString& dst);
	void removeParam(const QString& mName);
	void create(QRhiEx* inRhi);
	void updateResource(QRhiResourceUpdateBatch* batch);
	QRhiBuffer* getUniformBlock() const { return mUniformBlock.get(); }
	bool isEmpty()const { return mParamList.isEmpty(); }
	const QList<QSharedPointer<UniformParamDescBase>>& getParamList() const { return mParamList; }
protected:
	QString getVaildName(QString mName);
	void updateLayout();
protected:
	QList<QSharedPointer<UniformParamDescBase>> mParamList;
	QHash<QString, QSharedPointer<UniformParamDescBase>> mParamNameMap;
	uint32_t mDataByteSize;
	QScopedPointer<QRhiBuffer> mUniformBlock;
	QRhiShaderStage::Type mStage;
public:
	QRhiEx::Signal sigRecreateBuffer;
};

template<>
struct UniformParamDesc<float> : public UniformParamDescBase {
	const char* typeName() override { return "float"; }
	int dataByteSize() override { return sizeof(float); }
	int dataAlignSize() override { return sizeof(float); }
};

template<>
struct UniformParamDesc<QVector2D> : public UniformParamDescBase {
	const char* typeName() override { return "vec2"; }
	int dataByteSize() override { return sizeof(float) * 2; }
	int dataAlignSize() override { return sizeof(float) * 2; }
};

template<>
struct UniformParamDesc<QVector3D> : public UniformParamDescBase {
	const char* typeName() override { return "vec3"; }
	int dataByteSize() override { return sizeof(float) * 3; }
	int dataAlignSize() override { return sizeof(float) * 4; }
};

template<>
struct UniformParamDesc<QVector4D> : public UniformParamDescBase {
	const char* typeName() override { return "vec4"; }
	int dataByteSize() override { return sizeof(float) * 4; }
	int dataAlignSize() override { return sizeof(float) * 4; }
};

template<>
struct UniformParamDesc<QColor> : public UniformParamDescBase {
	const char* typeName() override { return "vec4"; }
	void setValue(QVariant inValue) override {
		QColor color = inValue.value<QColor >();
		mValue = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
	}
	int dataByteSize() override { return sizeof(float) * 4; }
	int dataAlignSize() override { return sizeof(float) * 4; }
protected:
	QVector4D mColor;
};

template<>
struct UniformParamDesc<QGenericMatrix<4, 4, float>> : public UniformParamDescBase {
	const char* typeName() override { return "mat4"; }
	int dataByteSize() override { return sizeof(float) * 16; }
	int dataAlignSize() override { return sizeof(float) * 16; }
};

template<typename _Ty,size_t _Size>
struct UniformParamDesc<std::array<_Ty,_Size>> : public UniformParamDescBase {
	const char* typeName() override { return mInnerDesc.typeName(); }
	void* dataPtr() override { return mValue.data(); }
	int dataByteSize() override {return _Size * mInnerDesc.dataByteSize();}
	int dataAlignSize() override { return mInnerDesc.dataAlignSize(); }
	QString valueName() override { return QString("%1[%2]").arg(mName).arg(_Size); }
protected:
	UniformParamDesc<_Ty> mInnerDesc;
};

template<typename _Ty>
struct UniformParamDesc<QVector<_Ty>> : public UniformParamDescBase {
	const char* typeName() override { return mInnerDesc.typeName(); }
	void setValue(QVariant inValue) override {
		mVector = inValue.value<QVector<_Ty>>();
		mValue = inValue;
	}
	void* dataPtr() override { return mVector.data(); }
	int dataByteSize() override { return mVector.size() * mInnerDesc.dataByteSize(); }
	int dataAlignSize() override { return mInnerDesc.dataAlignSize(); }
	QString valueName() override { return QString("%1[%2]").arg(mName).arg(mVector.size()); }
protected:
	UniformParamDesc<_Ty> mInnerDesc;
	QVector<_Ty> mVector;
};

#endif // QRhiUniformBlock_h_