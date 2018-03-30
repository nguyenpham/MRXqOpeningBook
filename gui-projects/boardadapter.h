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
