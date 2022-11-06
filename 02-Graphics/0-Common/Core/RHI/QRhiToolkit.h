#ifndef QRhiToolkit_h__
#define QRhiToolkit_h__

#include "private/qrhi_p.h"

namespace QRhiToolkit {

class DirtySignal {
public:
	DirtySignal() {};
	void mark();
	bool handle();
private:
	bool bDirty = false;
};

QShader createShaderFromCode(QShader::Stage stage, const char* code);

QShader createShaderFromQSBFile(const char* filename);

}





#endif // QRhiToolkit_h__
