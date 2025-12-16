#include "bankmainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    BankMainWindow w;
    w.show();
    return a.exec();
}
