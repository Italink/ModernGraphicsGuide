#include "QInstacneDetail_QRhiUniformBlock.h"
#include "RHI/QRhiUniform.h"

QInstacneDetail_QRhiUniformBlock::QInstacneDetail_QRhiUniformBlock() {

}

bool QInstacneDetail_QRhiUniformBlock::Filter(const QSharedPointer<QInstance> inInstance) {
	return inInstance->GetMetaObject()->inherits(&QRhiUniform::staticMetaObject);
}

void QInstacneDetail_QRhiUniformBlock::Build() {
	mUniformBlock = (QRhiUniform*) mInstance->GetPtr();
	for (const QSharedPointer<QRhiUniform::ParamMemoryDesc>& param : mUniformBlock->getParamList()) {
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
