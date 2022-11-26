#include <QApplication>
#include "MyFirstTextureWindow.h"

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);

    QRhiWindow::InitParams initParams;
    initParams.backend = QRhi::D3D11;
    MyFirstTextureWindow* window = new MyFirstTextureWindow(initParams);
	window->resize({ 800,600 });
	window->show();

    app.exec();
    delete window;
    return 0;
}