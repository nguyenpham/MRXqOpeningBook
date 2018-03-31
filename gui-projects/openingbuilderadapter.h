#ifndef OPENINGBUILDERADAPTER_H
#define OPENINGBUILDERADAPTER_H

#include <QObject>
#include "../source/OpBookBuilder.h"

class OpeningBuilderAdapter : public QObject
{
    Q_OBJECT
public:
    explicit OpeningBuilderAdapter(QObject *parent = nullptr);

    Q_INVOKABLE void create(QString folder, QString openingPath, QString info, int sides,int minGameLength, int addToLength, int repeatCnt);

signals:

public slots:

private:
    opening::OpBookBuilder m_builder;

};

#endif // OPENINGBUILDERADAPTER_H
