#ifndef QRhiEx_h__
#define QRhiEx_h__

#include "private/qrhi_p.h"
#include <QWindow>

class QRhiEx: public QRhi {
public:
	static QSharedPointer<QRhiEx> newRhiEx(QRhi::Implementation inBackend = QRhi::Vulkan, QRhi::Flags inFlags = QRhi::Flag(), QWindow* inWindow = nullptr);

	static QShader newShaderFromCode(QShader::Stage stage, const char* code);

	static QShader newShaderFromQSBFile(const char* filename);

	static void globalInitialize();
public:
	class Signal {
	public:
		Signal() {};
		void request();
		bool receive();
		bool peek();
	private:
		bool bDirty = false;
	};
};

#endif // QRhiEx_h__
