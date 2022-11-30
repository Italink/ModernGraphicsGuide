#include "QRhiEx.h"
#include <QFile>
#include "private\qshaderbaker_p.h"
#include "private\qrhivulkan_p.h"

#ifndef QT_NO_OPENGL
#include <QOffscreenSurface>
#include "private\qrhigles2_p.h"
#endif

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
#include <QtGui/private/qrhimetal_p.h>
#endif

#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>
#include "Customization\QDetailWidgetManager.h"
#include "DetailCustomization\QInstacneDetail_QRhiUniformBlock.h"

void QRhiEx::Signal::request() {
	bDirty = true;
}


bool QRhiEx::Signal::receive()
{
	bool var = bDirty;
	bDirty = false;
	return var;
}

bool QRhiEx::Signal::peek() {
	return bDirty;
}

QSharedPointer<QRhiEx> QRhiEx::newRhiEx(QRhi::Implementation inBackend /*= QRhi::Vulkan*/, QRhi::Flags inFlags /*= QRhi::Flag()*/, QWindow* inWindow /*= nullptr*/) {
	static bool bIsInitialize = false;
	if (!bIsInitialize) {
		globalInitialize();
		bIsInitialize = true;
	}
	
	QSharedPointer<QRhiEx> mRhi;
	if (inBackend == QRhi::Null) {
		QRhiNullInitParams params;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Null, &params, inFlags))));
	}

#ifndef QT_NO_OPENGL
	if (inBackend == QRhi::OpenGLES2) {
		QRhiGles2InitParams params;
		params.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();
		params.window = inWindow;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::OpenGLES2, &params, inFlags))));
	}
#endif

#if QT_CONFIG(vulkan)
	if (inBackend == QRhi::Vulkan) {
		static QVulkanInstance vkInstance;
		if (!vkInstance.isValid()) {
			vkInstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
			vkInstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
			if (!vkInstance.create())
				qFatal("Failed to create Vulkan instance");
		}
		QRhiVulkanInitParams params;
		if (inWindow) {
			inWindow->setVulkanInstance(&vkInstance);
			params.window = inWindow;
		}
		params.inst = &vkInstance;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Vulkan, &params, inFlags))));
	}
#endif

#ifdef Q_OS_WIN
	if (inBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = true;
		//if (framesUntilTdr > 0) {
		//	params.framesUntilKillingDeviceViaTdr = mInitParams.framesUntilTdr;
		//	params.repeatDeviceKill = true;
		//}
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::D3D11, &params, inFlags))));
	}
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
	if (inBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		mRhi.reset(static_cast<QRhiEx*>((QRhi::create(QRhi::Metal, &params, inFlags))));
	}
#endif
	return mRhi;
}

QShader QRhiEx::newShaderFromCode(QShader::Stage stage, const char* code) {
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	baker.setGeneratedShaders({
		QShaderBaker::GeneratedShader{QShader::Source::SpirvShader,QShaderVersion(100)},
		QShaderBaker::GeneratedShader{QShader::Source::GlslShader,QShaderVersion(430)},
		QShaderBaker::GeneratedShader{QShader::Source::MslShader,QShaderVersion(12)},
		QShaderBaker::GeneratedShader{QShader::Source::HlslShader,QShaderVersion(50)},
		});

	baker.setSourceString(code, stage);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		qWarning(code);
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader QRhiEx::newShaderFromQSBFile(const char* filename) {
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

void QRhiEx::globalInitialize() {
	QDetailWidgetManager::instance()->RegisterInstanceFilter<QInstacneDetail_QRhiUniformBlock>();
}

