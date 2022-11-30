#ifndef QInstacneDetail_QRhiUniformBlock_h__
#define QInstacneDetail_QRhiUniformBlock_h__

#include "Customization\Instance\QInstanceDetail.h"

class QRhiUniform;

class QInstacneDetail_QRhiUniformBlock: public QInstanceDetail {
	Q_OBJECT
public:
	QInstacneDetail_QRhiUniformBlock();
	static bool Filter(const QSharedPointer<QInstance> inInstance);
	virtual void Build() override;
private:
	QRhiUniform* mUniformBlock = nullptr;
};

#endif // QInstacneDetail_QRhiUniformBlock_h__
