
#include "RandomTextColor.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QDir varDir{ app.applicationDirPath() };

    RandomTextColor varConvert{
        varDir.absoluteFilePath(QStringLiteral("image")),
        varDir.absoluteFilePath(QStringLiteral("ans.png")) };
    
    if (varConvert.convert()) {
        return 0;
    }

    return -1;
}






