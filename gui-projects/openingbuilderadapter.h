#ifndef OPENINGBUILDERADAPTER_H
#define OPENINGBUILDERADAPTER_H

#include <QObject>

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
    Q_INVOKABLE void verify(QString openingPath);

signals:
    void taskDone();
    void reportString(const QString& msg);
    void reportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt);

public slots:

private:
    void doTask(int task, std::map<std::string, std::string>& paramMap);

    TaskThread* m_taskThread;

//    opening::OpBookBuilder m_builder;

};

#endif // OPENINGBUILDERADAPTER_H
