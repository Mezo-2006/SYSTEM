#include <QApplication>
#include <QIcon>
#include <Qt>
#include "gui/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Set application properties
    QApplication::setApplicationName("Switch-Case Compiler");
    QApplication::setApplicationVersion("1.0");
    QApplication::setOrganizationName("Educational Compiler Project");
    QApplication::setWindowIcon(QIcon(":/icons/cpp_icon.svg"));
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.setWindowState((mainWindow.windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    mainWindow.showNormal();
    mainWindow.raise();
    mainWindow.activateWindow();
    
    return app.exec();
}
