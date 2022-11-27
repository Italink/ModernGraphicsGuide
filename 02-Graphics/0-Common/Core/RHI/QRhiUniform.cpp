#include "QRhiUniform.h"
#include "Render/IRenderComponent.h"
#include "Render/IRenderPass.h"

QRhiUniform::QRhiUniform(QSharedPointer<QRhiEx> inRhi)
	: mRhi(inRhi)
{
}

void QRhiUniform::setDataFloat(QString name, float var) {
	auto param = getOrCreateParam(name, ParamMemoryDesc::Float);
	auto data = param.staticCast<ParamMemoryDesc>();
	data->var = var;
	data->bNeedUpdate.mark();
}

void QRhiUniform::setDataVec2(QString name, QVector2D var) {
	auto param = getOrCreateParam(name, ParamMemoryDesc::Vec2);
	auto data = param.staticCast<ParamMemoryDesc>();
	data->var = var;
	data->bNeedUpdate.mark();
}

void QRhiUniform::setDataVec3(QString name, QVector3D var) {
	auto param = getOrCreateParam(name, ParamMemoryDesc::Vec3);
	auto data = param.staticCast<ParamMemoryDesc>();
	data->var = var;
	data->bNeedUpdate.mark();
}

void QRhiUniform::setDataVec4(QString name, QVector4D var) {
	auto param = getOrCreateParam(name, ParamMemoryDesc::Vec4);
	auto data = param.staticCast<ParamMemoryDesc>();
	data->var = var;
	data->bNeedUpdate.mark();
}

void QRhiUniform::removeParam(const QString& name)
{
	auto iter = mParamNameMap.find(name);
	if (iter != mParamNameMap.end()) {
		mDataList.removeOne(*iter);
		mParamNameMap.remove(name);
		bNeedRecreate.mark();
	}
}

bool QRhiUniform::renameParma(const QString& src, const QString& dst)
{
	if (mParamNameMap.contains(src)) {
		auto param = mParamNameMap.take(src);
		param->name = dst;
		mParamNameMap[dst] = param;
		bNeedRecreate.mark();
		return true;
	}
	return false;
}

QString QRhiUniform::ParamMemoryDesc::getTypeName()
{
	switch (type)
	{
	case QRhiUniform::ParamMemoryDesc::Float:
		return "float";
	case QRhiUniform::ParamMemoryDesc::Vec2:
		return "vec2";
	case QRhiUniform::ParamMemoryDesc::Vec3:
		return "vec3";
	case QRhiUniform::ParamMemoryDesc::Vec4:
		return "vec4";
	case QRhiUniform::ParamMemoryDesc::Mat4:
		return "mat4";
	default:
		break;
	}
	return "";
}

QSharedPointer<QRhiUniform::ParamMemoryDesc> QRhiUniform::getOrCreateParam(QString name, ParamMemoryDesc::Type type) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		return *Iter;
	}
	else {
		QSharedPointer<QRhiUniform::ParamMemoryDesc> param;
		mDataList << QSharedPointer<QRhiUniform::ParamMemoryDesc>::create();
		param = mDataList.back();
		param->type = type;
		param->name = getVaildName(name);
		mParamNameMap[param->name] = param;
		bNeedRecreate.mark();
		return param;
	}
}

QString QRhiUniform::getVaildName(QString name) {
	QString newName = name;
	int index = 0;
	while (mParamNameMap.contains(newName)) {
		newName = name + QString::number(index);
	}
	return newName;
}

int getByteSize(QRhiUniform::ParamMemoryDesc::Type type) {
	switch (type) {
	case QRhiUniform::ParamMemoryDesc::Float:
		return sizeof(float);
	case QRhiUniform::ParamMemoryDesc::Vec2:
		return sizeof(float) * 2;
	case QRhiUniform::ParamMemoryDesc::Vec3:
		return sizeof(float) * 3;
	case QRhiUniform::ParamMemoryDesc::Vec4:
		return sizeof(float) * 4;
	case QRhiUniform::ParamMemoryDesc::Mat4:
		return sizeof(float) * 16;
	}
	return 0;
}

int getAlign(QRhiUniform::ParamMemoryDesc::Type type) {
	switch (type) {
	case QRhiUniform::ParamMemoryDesc::Float:
		return sizeof(float);
	case QRhiUniform::ParamMemoryDesc::Vec2:
		return sizeof(float) * 2;
	case QRhiUniform::ParamMemoryDesc::Vec3:
		return sizeof(float) * 4;
	case QRhiUniform::ParamMemoryDesc::Vec4:
		return sizeof(float) * 4;
	case QRhiUniform::ParamMemoryDesc::Mat4:
		return sizeof(float) * 16;
	}
	return 0;
}

int align(int size, int alignSize) {
	if (size == alignSize)
		return size;
	return (size + alignSize - 1) & ~(alignSize - 1);
}

void QRhiUniform::updateLayout() {
	mDataSize = 0;
	for (int i = 0; i < mDataList.size(); i++) {
		auto& data = mDataList[i];
		data->offsetInByte = align(mDataSize,getAlign(data->type));
		data->sizeInByte = getByteSize(data->type);
		data->sizeInByteAligned = data->sizeInByte;	//16字节对齐
		data->bNeedUpdate.mark();
		mDataSize = data->offsetInByte+data->sizeInByteAligned;
	}
}

void QRhiUniform::create() {
	updateLayout();
	mUniformBlock.reset(mRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer,mDataSize));
	mUniformBlock->create();
	for (auto& dataParam : mDataList) {
		dataParam->bNeedUpdate.mark();
	}
}

void QRhiUniform::updateResource(QRhiResourceUpdateBatch* batch) {
	for (auto& dataParam : mDataList) {
		if (dataParam->bNeedUpdate.handle()) {
			batch->updateDynamicBuffer(mUniformBlock.get(), dataParam->offsetInByte, dataParam->sizeInByte, dataParam->var.data());
		}
	}
}
