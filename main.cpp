// main.cpp
// Курсовой проект сделан студентами Митюшиной Е. В. и Авдеевом Д. И.  М8О-205М-23

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
