
/*
 This file is part of MoonRiver Xiangqi Opening Book, distributed under MIT license.

 Copyright (c) 2018 Nguyen Hong Pham

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */


#include "openingbuilderadapter.h"

void TaskThread::run()
{
    opening::OpBookBuilder opBookBuilder;

    switch (m_task) {
    case 0:
    {
        opBookBuilder.create(m_paramMap, [&](std::string msg) {
            emit reportString(QString(msg.c_str()));
        },
        [&](int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt) {
            emit reportNumbers(fileCnt, gameCnt, nodeCnt, addedNodeCnt);
        }
        );
        break;
    }

    case 1: {
        opBookBuilder.verify(m_paramMap, [&](std::string msg) {
            emit reportString(QString(msg.c_str()));
        },
        [&](int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt) {
            emit reportNumbers(fileCnt, gameCnt, nodeCnt, addedNodeCnt);
        }
        );
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

void OpeningBuilderAdapter::verify(QString openingPath)
{
    std::map<std::string, std::string> paramMap;
    paramMap["out"] = openingPath.toStdString();
    doTask(1, paramMap);
}

void OpeningBuilderAdapter::create(QString folder, QString openingPath, QString info, int sides, int minGameLength, int addToLength, int repeatCnt)
{
    std::map<std::string, std::string> paramMap;

    paramMap["folder"] = folder.toStdString();
    paramMap["out"] = openingPath.isEmpty() ? "./opening.xop" : openingPath.toStdString();
    paramMap["info"] = info.toStdString();

    paramMap["maxply"] = QString("%1").arg(addToLength).toStdString();
    paramMap["minply"] = QString("%1").arg(minGameLength).toStdString();
    paramMap["mingame"] = QString("%1").arg(repeatCnt).toStdString();

    if (sides != 3) {
        if (sides == 1) {
            paramMap["-only-black"] = "1";
        } else
        if (sides == 2) {
            paramMap["-only-white"] = "1";
        }
    }

    doTask(0, paramMap);
}

void OpeningBuilderAdapter::doTask(int task, std::map<std::string, std::string>& paramMap)
{
    m_taskThread = new TaskThread(task, paramMap);

    disconnect(m_taskThread);
    connect(m_taskThread, &TaskThread::finished, this, &OpeningBuilderAdapter::taskDone);
    connect(m_taskThread, &TaskThread::reportString, this, &OpeningBuilderAdapter::reportString);
    connect(m_taskThread, &TaskThread::reportNumbers, this, &OpeningBuilderAdapter::reportNumbers);
    m_taskThread->start();
}
