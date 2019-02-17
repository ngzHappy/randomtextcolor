#pragma once

#include <QtCore/QtCore>
#include <QtGui/QtGui>

template<typename T>
class RandomTextColorPrivate;

class RandomTextColor{
    QString mmmInputFileName;
    QString mmmOutputFileName;
public:
    RandomTextColor(const QString &,const QString &);
public:
    bool convert();
private:
    template<typename T>
    friend class ::RandomTextColorPrivate;
};





























