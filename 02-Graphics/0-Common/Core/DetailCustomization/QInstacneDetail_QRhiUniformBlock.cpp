#include "QInstacneDetail_QRhiUniformBlock.h"
#include "RHI/QRhiUniformBlock.h"

QInstacneDetail_QRhiUniformBlock::QInstacneDetail_QRhiUniformBlock() {

}

bool QInstacneDetail_QRhiUniformBlock::Filter(const QSharedPointer<QInstance> inInstance) {
	return inInstance->GetMetaObject()->inherits(&QRhiUniformBlock::staticMetaObject);
}

void QInstacneDetail_QRhiUniformBlock::Build() {
	mUniformBlock = (QRhiUniformBlock*) mInstance->GetPtr();
	for (const QSharedPointer<QRhiUniformBlock::ParamMemoryDesc>& param : mUniformBlock->getParamList()) {
		QPropertyHandler* handler = QPropertyHandler::FindOrCreate(
			mUniformBlock,
			param->var.metaType(),
			param->name,
			[weakParam = param.toWeakRef()]() {
				return weakParam.lock()->var;
			},
			[weakParam = param.toWeakRef()](QVariant var) {
				weakParam.lock()->var = var;
				weakParam.lock()->sigUpdateParam.request();
			}
		);
		AddProperty(handler);
	}
}
