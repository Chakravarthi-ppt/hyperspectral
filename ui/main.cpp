#include <QApplication>
#include "MainWindow.h"
#include "hsi/RasterIO.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("HyperspectralSuite");
    QApplication::setOrganizationName("HyperspectralSuite");

    hsi::RasterIO::init();

    MainWindow window;
    window.showMaximized();

    return app.exec();
}
