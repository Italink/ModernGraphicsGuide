#ifndef QInstacneDetail_QRhiUniformBlock_h__
#define QInstacneDetail_QRhiUniformBlock_h__

#include "Customization\Instance\QInstanceDetail.h"

class QRhiUniformBlock;

class QInstacneDetail_QRhiUniformBlock: public QInstanceDetail {
	Q_OBJECT
public:
	QInstacneDetail_QRhiUniformBlock();
	static bool Filter(const QSharedPointer<QInstance> inInstance);
	virtual void Build() override;
private:
	QRhiUniformBlock* mUniformBlock = nullptr;
};

#endif // QInstacneDetail_QRhiUniformBlock_h__
