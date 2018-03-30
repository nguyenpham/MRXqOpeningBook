import QtQuick 2.9
import QtQuick.Controls 1.4
import QtQuick.Window 2.3

import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0

import TheBoardAdapter 1.0

Rectangle {
    id: board

    property int cellWidth: Math.min(board.width / 10, board.height / 11)
    property int marginWidth: cellWidth / 3

    FontLoader {
        id: xqFont;
        source: "qrc:/fonts/xiangqi.ttf"
    }

    BoardAdapter {
        id: boardAdapter
    }

    XqBoardGrid {
        id: boardFrame
        anchors.fill: parent
        margin: marginWidth
        cellSize: cellWidth
//        bgcolor: parent.color
    }

    // Piece
    Grid {
        id: pieceGrid
        x: marginWidth
        y: marginWidth
        spacing: 0
        rows: 10
        columns: 9

        Repeater {
            id: pieceCells
            model: 90
            Piece {
                width: cellWidth
                height: cellWidth
                letter: boardAdapter.getPieceLetter(index)
                mark: boardAdapter.getMark(index)
            }
        }
    }


    function getBoardAdapter() {
        return boardAdapter
    }

    function getBoard() {
        return boardAdapter.board()
    }

    function newFen(fen) {
        boardAdapter.newGame(fen);
        refresh();
    }

    function updateLine(moves) {
        boardAdapter.updateLine(moves);
        refresh();
    }

    function refresh() {
        for (var i = 0; i < pieceCells.model; ++i) {
            pieceCells.itemAt(i).letter = boardAdapter.getPieceLetter(i)
            pieceCells.itemAt(i).mark = boardAdapter.getMark(i)
        }
    }

}
