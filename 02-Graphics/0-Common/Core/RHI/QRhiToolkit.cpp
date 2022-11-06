#include "QRhiToolkit.h"
#include <QFile>
#include "private\qshaderbaker_p.h"

bool QRhiToolkit::DirtySignal::handle() 
{
	bool var = bDirty;
	bDirty = false;
	return var;
}

void QRhiToolkit::DirtySignal::mark() {
	bDirty = true;
}

QShader QRhiToolkit::createShaderFromCode(QShader::Stage stage, const char* code) {
	QShaderBaker baker;
	baker.setGeneratedShaderVariants({ QShader::StandardShader });
	baker.setGeneratedShaders({
		QShaderBaker::GeneratedShader{QShader::Source::SpirvShader,QShaderVersion(100)},
		QShaderBaker::GeneratedShader{QShader::Source::GlslShader,QShaderVersion(430)},
		QShaderBaker::GeneratedShader{QShader::Source::MslShader,QShaderVersion(12)},
		QShaderBaker::GeneratedShader{QShader::Source::HlslShader,QShaderVersion(60)},
		});

	baker.setSourceString(code, stage);
	QShader shader = baker.bake();
	if (!shader.isValid()) {
		qWarning(code);
		qWarning(baker.errorMessage().toLocal8Bit());
	}
	return shader;
}

QShader QRhiToolkit::createShaderFromQSBFile(const char* filename) {
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

