#include "openingbuilderadapter.h"

void TaskThread::run()
{
    switch (m_task) {
    case 0:
    {
        opening::OpBookBuilder opBookBuilder;
        opBookBuilder.create(m_paramMap, [&](std::string path) {
            emit reportString(QString(path.c_str()));
        },
        [&](int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt) {
            emit reportNumbers(fileCnt, gameCnt, nodeCnt, addedNodeCnt);
        }
        );
        break;
    }

    case 1: {
        auto openingPath = m_paramMap["out"];
        opening::OpBook opBook;
        opBook.load(openingPath);
        opBook.verifyData();
        break;
    }

    default:
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////
OpeningBuilderAdapter::OpeningBuilderAdapter(QObject *parent) : QObject(parent)
{

}


void OpeningBuilderAdapter::create(QString folder, QString openingPath, QString info, int sides, int minGameLength, int addToLength, int repeatCnt)
{
    std::map<std::string, std::string> paramMap;

    paramMap["folder"] = folder.toStdString();
    paramMap["out"] = openingPath.isEmpty() ? "./opening.xop" : openingPath.toStdString();
    paramMap["info"] = info.toStdString();

    paramMap["maxply"] = QString(addToLength).toStdString();
    paramMap["minply"] = QString(minGameLength).toStdString();
    paramMap["mingame"] = QString(repeatCnt).toStdString();

    if (sides != 3) {
        if (sides == 1) {
            paramMap["-only-black"] = "1";
        } else
        if (sides == 2) {
            paramMap["-only-white"] = "1";
        }
    }

//    m_builder.create(paramMap);

    int task = 0;
    m_taskThread = new TaskThread(task, paramMap);

//    connect(m_taskThread, &QThread::finished, [=]() {
//        m_taskThread->deleteLater();
//        m_taskThread = nullptr;
//    });

    connect(m_taskThread, &TaskThread::finished, this, &OpeningBuilderAdapter::taskDone);
    connect(m_taskThread, &TaskThread::reportString, this, &OpeningBuilderAdapter::reportString);
    connect(m_taskThread, &TaskThread::reportNumbers, this, &OpeningBuilderAdapter::reportNumbers);
//    connect(m_taskThread, SIGNAL(reportString), this, reportString);
//    connect(m_taskThread, &TaskThread::reportString, this, [this](const QString& msg) {
//        emit reportString(msg);
//    });

//    connect(m_taskThread, &TaskThread::reportNumbers, this, [this](int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt) {
////        QString str = QString("Added nodes: %1 / %2, games: %3, files: %4").arg(addedNodeCnt).arg(nodeCnt).arg(gameCnt).arg(fileCnt);
////        ui->m_infoNumLabel->setText(str);
//        emit reportString(fileCnt, gameCnt, nodeCnt, addedNodeCnt);
//    });

    m_taskThread->start();

}
