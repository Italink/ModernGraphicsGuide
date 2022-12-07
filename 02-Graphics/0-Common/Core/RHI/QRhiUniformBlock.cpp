#include "QRhiUniformBlock.h"
#include "Render/IRenderComponent.h"
#include "Render/IRenderPass.h"

QRhiUniformBlock::QRhiUniformBlock( QRhiShaderStage::Type inStage, QObject* inParent)
	: QObject(inParent)
	, mStage(inStage)
{
}

QRhiUniformBlock* QRhiUniformBlock::addFloat(QString name, float var) {
	addParam(name, ParamMemoryDesc::Float, var);
	return this;
}

QRhiUniformBlock* QRhiUniformBlock::addVec2(QString name, QVector2D var) {
	addParam(name, ParamMemoryDesc::Vec2, var);
	return this;
}

QRhiUniformBlock* QRhiUniformBlock::addVec3(QString name, QVector3D var) {
	addParam(name, ParamMemoryDesc::Vec3, var);
	return this;
}

QRhiUniformBlock* QRhiUniformBlock::addVec4(QString name, QVector4D var) {
	addParam(name, ParamMemoryDesc::Vec4, var);
	return this;
}

QRhiUniformBlock* QRhiUniformBlock::addMat4(QString name, QGenericMatrix<4,4,float> var) {
	addParam(name, ParamMemoryDesc::Mat4, QVariant::fromValue(var));
	return this;
}

void QRhiUniformBlock::addParam(const QString& inName, ParamMemoryDesc::Type inType, QVariant inVar) {
	QSharedPointer<QRhiUniformBlock::ParamMemoryDesc> param = QSharedPointer<QRhiUniformBlock::ParamMemoryDesc>::create();
	param->name = getVaildName(inName);
	param->type = inType;
	param->var = inVar;
	mDataList << param;
	mParamNameMap[inName] = param;
	param->sigUpdateParam.request();
	sigRecreateBuffer.request();
}

void QRhiUniformBlock::setFloat(QString name, float var) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->var = var;
		(*Iter)->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::setVec2(QString name, QVector2D var) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->var = var;
		(*Iter)->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::setVec3(QString name, QVector3D var) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->var = var;
		(*Iter)->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::setVec4(QString name, QVector4D var) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->var = var;
		(*Iter)->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::setMat4(QString name, QGenericMatrix<4, 4,float> var) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->var = QVariant::fromValue(var);
		(*Iter)->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::removeParam(const QString& name)
{
	auto iter = mParamNameMap.find(name);
	if (iter != mParamNameMap.end()) {
		mDataList.removeOne(*iter);
		mParamNameMap.remove(name);
		sigRecreateBuffer.request();
	}
}

bool QRhiUniformBlock::renameParma(const QString& src, const QString& dst)
{
	if (mParamNameMap.contains(src)) {
		auto param = mParamNameMap.take(src);
		param->name = dst;
		mParamNameMap[dst] = param;
		sigRecreateBuffer.request();
		return true;
	}
	return false;
}

QString QRhiUniformBlock::ParamMemoryDesc::getTypeName()
{
	switch (type)
	{
	case QRhiUniformBlock::ParamMemoryDesc::Float:
		return "float";
	case QRhiUniformBlock::ParamMemoryDesc::Vec2:
		return "vec2";
	case QRhiUniformBlock::ParamMemoryDesc::Vec3:
		return "vec3";
	case QRhiUniformBlock::ParamMemoryDesc::Vec4:
		return "vec4";
	case QRhiUniformBlock::ParamMemoryDesc::Mat4:
		return "mat4";
	default:
		break;
	}
	return "";
}

QString QRhiUniformBlock::getVaildName(QString name) {
	QString newName = name;
	int index = 0;
	while (mParamNameMap.contains(newName)) {
		newName = name + QString::number(index);
	}
	return newName;
}

int getByteSize(QRhiUniformBlock::ParamMemoryDesc::Type type) {
	switch (type) {
	case QRhiUniformBlock::ParamMemoryDesc::Float:
		return sizeof(float);
	case QRhiUniformBlock::ParamMemoryDesc::Vec2:
		return sizeof(float) * 2;
	case QRhiUniformBlock::ParamMemoryDesc::Vec3:
		return sizeof(float) * 3;
	case QRhiUniformBlock::ParamMemoryDesc::Vec4:
		return sizeof(float) * 4;
	case QRhiUniformBlock::ParamMemoryDesc::Mat4:
		return sizeof(float) * 16;
	}
	return 0;
}

int getAlign(QRhiUniformBlock::ParamMemoryDesc::Type type) {
	switch (type) {
	case QRhiUniformBlock::ParamMemoryDesc::Float:
		return sizeof(float);
	case QRhiUniformBlock::ParamMemoryDesc::Vec2:
		return sizeof(float) * 2;
	case QRhiUniformBlock::ParamMemoryDesc::Vec3:
		return sizeof(float) * 4;
	case QRhiUniformBlock::ParamMemoryDesc::Vec4:
		return sizeof(float) * 4;
	case QRhiUniformBlock::ParamMemoryDesc::Mat4:
		return sizeof(float) * 16;
	}
	return 0;
}

int align(int size, int alignSize) {
	if (size == alignSize)
		return size;
	return (size + alignSize - 1) & ~(alignSize - 1);
}

void QRhiUniformBlock::updateLayout() {
	mDataSize = 0;
	for (int i = 0; i < mDataList.size(); i++) {
		auto& data = mDataList[i];
		data->offsetInByte = align(mDataSize,getAlign(data->type));
		data->sizeInByte = getByteSize(data->type);
		data->sizeInByteAligned = data->sizeInByte;	//16字节对齐
		data->sigUpdateParam.request();
		mDataSize = data->offsetInByte+data->sizeInByteAligned;
	}
}

void QRhiUniformBlock::create(QRhiEx* inRhi) {
	updateLayout();
	mUniformBlock.reset(inRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, mDataSize));
	mUniformBlock->create();
	for (auto& dataParam : mDataList) {
		dataParam->sigUpdateParam.request();
	}
}

void QRhiUniformBlock::updateResource(QRhiResourceUpdateBatch* batch) {
	for (auto& dataParam : mDataList) {
		if (dataParam->sigUpdateParam.receive()) {
			batch->updateDynamicBuffer(mUniformBlock.get(), dataParam->offsetInByte, dataParam->sizeInByte, dataParam->var.data());
		}
	}
}
