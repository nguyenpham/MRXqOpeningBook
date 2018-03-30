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
