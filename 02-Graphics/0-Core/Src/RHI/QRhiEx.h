#ifndef QRhiEx_h__
#define QRhiEx_h__

#include "private/qrhi_p.h"
#include "private/qrhi_p_p.h"
#include "private/qrhivulkan_p_p.h"
#include <QWindow>

class QRhiEx: public QRhi {
public:
	static void globalInitialize();

	static QVulkanInstance* getVkInstance();

	static QSharedPointer<QRhiEx> newRhiEx(QRhi::Implementation inBackend = QRhi::Vulkan, QRhi::Flags inFlags = QRhi::Flag(), QWindow* inWindow = nullptr);

	static QShader newShaderFromCode(QShader::Stage stage, const char* code);

	static QShader newShaderFromQSBFile(const char* filename);

	QRhiBuffer* newVkBuffer(QRhiBuffer::Type type, VkBufferUsageFlags flags, int size);

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
