#ifndef QWindowRenderer_h__
#define QWindowRenderer_h__

#include "Render/IRenderer.h"

class QRhiWindow;

class QWindowRenderer : public IRenderer {
	Q_OBJECT
public:
	QWindowRenderer(QRhiWindow* inWindow);

	virtual void render() override;
	virtual QRhiRenderTarget* renderTaget() override;
	virtual int sampleCount()override;
private:
	QRhiWindow* mWindow;
};


#endif // QWindowRenderer_h__
