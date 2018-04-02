
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


#ifndef BOARDADAPTER_H
#define BOARDADAPTER_H

#include <QObject>

#include "../source/OpBoard.h"

class BoardAdapter : public QObject
{
    Q_OBJECT

public:
    explicit BoardAdapter(QObject* t_parent = nullptr);

    Q_INVOKABLE const opening::OpeningBoard& board() const { return m_board; }
    Q_INVOKABLE opening::OpeningBoard& board() { return m_board; }
    Q_INVOKABLE void newGame(QString fen = "");

    Q_INVOKABLE QString getPieceLetter(int pos) const;
    Q_INVOKABLE int getMark(int pos) const;
    Q_INVOKABLE void updateLine(const QVariantList& moveList);

signals:
    void boardChanged();

public slots:

private:
    opening::OpeningBoard m_board;
};

#endif // BOARDADAPTER_H
