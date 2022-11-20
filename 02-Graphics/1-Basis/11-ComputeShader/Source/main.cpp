#include <QApplication>
#include "ComputeShaderWindow.h"

int main(int argc, char **argv)
{
    qputenv("QSG_INFO", "1");
    QApplication app(argc, argv);
    QRhiWindow::InitParams initParams;
    ComputeShaderWindow window(initParams);
	window.resize({ 800,600 });
	window.show();
    app.exec();
    return 0;
}