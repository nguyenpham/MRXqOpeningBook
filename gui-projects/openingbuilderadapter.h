#ifndef OPENINGBUILDERADAPTER_H
#define OPENINGBUILDERADAPTER_H

#include <QObject>
#include "../source/OpBookBuilder.h"
#include <QThread>

namespace Ui {
class OpeningBuilderAdapter;
}

//class TaskThread;
class TaskThread : public QThread
{
    Q_OBJECT

public:
    TaskThread(int task, const std::map<std::string, std::string>& _paramMap)
        : QThread(0),
          m_task(task),
          m_paramMap(_paramMap)
    {
    }

signals:
    void reportString(const QString& msg);
    void reportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt);

protected:
    void run() override;

private:
    int m_task;
    std::map<std::string, std::string> m_paramMap;
};


///////////////////////////////////////////////////////////////////////////////

class OpeningBuilderAdapter : public QObject
{
    Q_OBJECT
public:
    explicit OpeningBuilderAdapter(QObject *parent = nullptr);

    Q_INVOKABLE void create(QString folder, QString openingPath, QString info, int sides,int minGameLength, int addToLength, int repeatCnt);

signals:
    void taskDone();
    void reportString(const QString& msg);
    void reportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt);

public slots:

private:
    TaskThread* m_taskThread;

//    opening::OpBookBuilder m_builder;

};

#endif // OPENINGBUILDERADAPTER_H
