#ifndef QWindowRenderer_h__
#define QWindowRenderer_h__

#include "Render/IRenderer.h"

class QRhiWindow;

class QWindowRenderer : public IRenderer {
	Q_OBJECT
public:
	friend class QRendererWidget;
	QWindowRenderer(QRhiWindow* inWindow);
	QWindow* getWindow() const;

protected:
	void render() override;
	QRhiRenderTarget* renderTaget() override;
	int sampleCount()override;
private:
	QRhiWindow* mWindow;
};


#endif // QWindowRenderer_h__
