#include "MainWindow.hpp"
#include <QApplication>


#include "RandomTextColor.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    RandomTextColor c{
        QStringLiteral(R"(C:\Project\Test\randomtextcolor\image.jpg)"),
        QStringLiteral(R"(C:\Project\Test\randomtextcolor\ans.png)") };
    c.convert();

    return app.exec();
}






