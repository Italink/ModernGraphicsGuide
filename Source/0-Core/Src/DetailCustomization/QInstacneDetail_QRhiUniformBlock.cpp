#include "QInstacneDetail_QRhiUniformBlock.h"
#include "RHI/QRhiUniformBlock.h"

QInstacneDetail_QRhiUniformBlock::QInstacneDetail_QRhiUniformBlock() {

}

bool QInstacneDetail_QRhiUniformBlock::Filter(const QSharedPointer<QInstance> inInstance) {
	return inInstance->GetMetaObject()->inherits(&QRhiUniformBlock::staticMetaObject);
}

void QInstacneDetail_QRhiUniformBlock::Build() {
	mUniformBlock = (QRhiUniformBlock*) mInstance->GetPtr();
	for (const QSharedPointer<UniformParamDescBase>& param : mUniformBlock->getParamList()) {
		QPropertyHandler* handler = QPropertyHandler::FindOrCreate(
			mUniformBlock,
			param->mValue.metaType(),
			param->mName,
			[weakParam = param.toWeakRef()]() {
				return weakParam.lock()->mValue;
			},
			[weakParam = param.toWeakRef()](QVariant var) {
				weakParam.lock()->mValue = var;
				weakParam.lock()->sigUpdate.request();
			}
		);
		AddProperty(handler);
	}
}
