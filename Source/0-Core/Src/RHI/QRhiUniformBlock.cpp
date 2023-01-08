#include "QRhiUniformBlock.h"
#include "Render/IRenderComponent.h"
#include "Render/IRenderPass.h"

QRhiUniformBlock::QRhiUniformBlock( QRhiShaderStage::Type inStage, QObject* inParent)
	: QObject(inParent)
	, mStage(inStage)
{
}

void QRhiUniformBlock::setParamValue(const QString& name, QVariant value) {
	auto Iter = mParamNameMap.find(name);
	if (Iter != mParamNameMap.end()) {
		(*Iter)->setValue(value);
		(*Iter)->sigUpdate.request();
	}
}

void QRhiUniformBlock::removeParam(const QString& name)
{
	auto iter = mParamNameMap.find(name);
	if (iter != mParamNameMap.end()) {
		mParamList.removeOne(*iter);
		mParamNameMap.remove(name);
		sigRecreateBuffer.request();
	}
}

bool QRhiUniformBlock::renameParma(const QString& src, const QString& dst)
{
	if (mParamNameMap.contains(src)) {
		auto param = mParamNameMap.take(src);
		param->mName = dst;
		mParamNameMap[dst] = param;
		sigRecreateBuffer.request();
		return true;
	}
	return false;
}


QString QRhiUniformBlock::getVaildName(QString name) {
	QString newName = name;
	int index = 0;
	while (mParamNameMap.contains(newName)) {
		newName = name + QString::number(index);
	}
	return newName;
}

int align(int size, int alignSize) {
	if (size == alignSize)
		return size;
	return (size + alignSize - 1) & ~(alignSize - 1);
}

void QRhiUniformBlock::updateLayout() {
	mDataByteSize = 0;
	for (int i = 0; i < mParamList.size(); i++) {
		auto& paramDesc = mParamList[i];
		paramDesc->mOffsetInByte = align(mDataByteSize, paramDesc->dataAlignSize());
		paramDesc->mSizeInByte = paramDesc->dataByteSize();
		paramDesc->mSizeInByteAligned = paramDesc->mSizeInByte;	//16字节对齐
		paramDesc->sigUpdate.request();
		mDataByteSize = paramDesc->mOffsetInByte + paramDesc->mSizeInByteAligned;
	}
}

void QRhiUniformBlock::create(QRhiEx* inRhi) {
	updateLayout();
	mUniformBlock.reset(inRhi->newBuffer(QRhiBuffer::Type::Dynamic, QRhiBuffer::UniformBuffer, mDataByteSize));
	mUniformBlock->create();
	for (auto& dataParam : mParamList) {
		dataParam->sigUpdate.request();
	}
}

void QRhiUniformBlock::updateResource(QRhiResourceUpdateBatch* batch) {
	for (auto& dataParam : mParamList) {
		if (dataParam->sigUpdate.receive()) {
			batch->updateDynamicBuffer(mUniformBlock.get(), dataParam->mOffsetInByte, dataParam->mSizeInByte, dataParam->dataPtr());
		}
	}
}
