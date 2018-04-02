
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


#include <QVariant>
#include <QDebug>

#include "boardadapter.h"


BoardAdapter::BoardAdapter(QObject* t_parent)
    : QObject(t_parent)
{
    m_board.setFen("");
    m_board.show("BoardAdapter::BoardAdapter");
}


void BoardAdapter::newGame(QString fen)
{
    m_board.newGame(fen.toStdString());
    m_board.show("BoardAdapter::newGame");
}

void BoardAdapter::updateLine(const QVariantList& moveList)
{
    m_board.newGame("");

    for(auto && val : moveList) {
        int m = val.toInt();
        int from = m & 0xff;
        int dest = m >> 8;

        m_board.make(from, dest);
    }
}

QString BoardAdapter::getPieceLetter(int pos) const
{
    auto piece = m_board.getPiece(pos);
    if (piece.isEmpty()) {
        return "";
    }

    char ch = opening::pieceTypeName[static_cast<int>(piece.type)];

    if (piece.side == opening::Side::black) {
        ch -= 'a' - 'A';
    }

    return QString(ch);
}

int BoardAdapter::getMark(int pos) const
{
    int mark = 0;
    if (!m_board.getHistList().empty()) {
        auto lastMove = m_board.getHistList().back().move;
        if (lastMove.from == pos) {
            mark = 1;
        } else if (lastMove.dest == pos) {
            mark = 2;
        }
    }
    return mark;
}
