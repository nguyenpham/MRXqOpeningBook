import QtQuick 2.0

Item {
    id: boardGrid
    property string bgcolor: "#eeeeee"
    property real margin: 10
    property real cellSize: 10

    Canvas {
        id: mycanvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            drawXqBoardGrid(ctx)
        }
    }


    function drawXqBoardGrid(ctx)
    {
        ctx.beginPath()

        ctx.fillStyle = bgcolor;
        ctx.fillRect(0, 0, cellSize * 9 + margin * 2, cellSize * 10 + margin * 2);

        var x1 = margin + cellSize / 2
        var y1 = x1
        var x2 = x1 + cellSize * 8
        var y2 = y1 + cellSize * 9

        ctx.strokeStyle = 'black'
        ctx.lineWidth = 2

        ctx.strokeRect(x1 - 3, y1 - 3, cellSize * 8 + 6, cellSize * 9 + 6);

        ctx.lineWidth = 1

        // Horizontal lines
        for (var i = 0; i < 10; i++) {
            ctx.moveTo(x1, y1 + i * cellSize);
            ctx.lineTo(x2, y1 + i * cellSize);
        }

        // 2 Vertical lines of Border
        ctx.moveTo(x1, y1);
        ctx.lineTo(x1, y2);

        ctx.moveTo(x2, y1);
        ctx.lineTo(x2, y2);

        // Vertical lines
        for (var i = 1; i < 8; i++) {
            ctx.moveTo(x1 + i * cellSize, y1);
            ctx.lineTo(x1 + i * cellSize, y1 + 4 * cellSize);

            ctx.moveTo(x1 + i * cellSize, y1 + 5 * cellSize);
            ctx.lineTo(x1 + i * cellSize, y2);
        }

        // King palace 1
        ctx.moveTo(x1 + 3 * cellSize, y1);
        ctx.lineTo(x1 + 5 * cellSize, y1 + 2 * cellSize);

        ctx.moveTo(x1 + 5 * cellSize, y1);
        ctx.lineTo(x1 + 3 * cellSize, y1 + 2 * cellSize);

        // King palace 2
        ctx.moveTo(x1 + 3 * cellSize, y1 + 7 * cellSize);
        ctx.lineTo(x1 + 5 * cellSize, y1 + 9 * cellSize);

        ctx.moveTo(x1 + 5 * cellSize, y1 + 7 * cellSize);
        ctx.lineTo(x1 + 3 * cellSize, y1 + 9 * cellSize);

//        auto sx = cellSize / 5;
//        auto sy = -cellSize / 5;
//        // Pawn positions
//        for (int j=3; j<=6; j+=3) {
//            for (int i=2; i<=6; i+=2) {
//                drawXqBoardGridMark(painter, (x1 + i * cellSize), (y1 + j * cellSize), sx, sy, 3);
//            }
//        }

//        drawXqBoardGridMark(painter, x1,  (y1 + 3 * cellSize), sx, sy, 2);
//        drawXqBoardGridMark(painter, x1, (y1 + 6 * cellSize), sx, sy, 2);
//        drawXqBoardGridMark(painter, x1 + 8 * cellSize, (y1 + 3 * cellSize), sx, sy, 1);
//        drawXqBoardGridMark(painter, x1 + 8 * cellSize, (y1 + 6 * cellSize), sx, sy, 1);
//
//        // Cannon positions
//        drawXqBoardGridMark(painter, x1 + 1 * cellSize, (y1 + 2 * cellSize), sx, sy, 3);
//        drawXqBoardGridMark(painter, x1 + 7 * cellSize, (y1 + 2 * cellSize), sx, sy, 3);
//        drawXqBoardGridMark(painter, x1 + 1 * cellSize, (y1 + 7 * cellSize), sx, sy, 3);
//        drawXqBoardGridMark(painter, x1 + 7 * cellSize, (y1 + 7 * cellSize), sx, sy, 3);

        ctx.stroke()
    }


//    function drawXqBoardGridMark(QPainter* painter, x, y, sx, sy, mode) {
//        const int d = 12;

//        if ((mode & 1) != 0) {
//            auto p0 = QPoint(x - d - sx, y - d);
//            auto p1 = QPoint(x - d, y - d);
//            auto p2 = QPoint(x - d, y - d + sy);

//            auto p3 = QPoint(x - d - sx, y + d);
//            auto p4 = QPoint(x - d, y + d);
//            auto p5 = QPoint(x - d, y + d - sy);

//            ctx.drawLine(p0, p1);
//            ctx.drawLine(p2, p1);

//            ctx.drawLine(p3, p4);
//            ctx.drawLine(p5, p4);
//        }

//        if ((mode & 2) != 0) {
//            auto p0 = QPoint(x + d + sx, y - d);
//            auto p1 = QPoint(x + d, y - d);
//            auto p2 = QPoint(x + d, y - d + sy);
//            auto p3 = QPoint(x + d + sx, y + d);
//            auto p4 = QPoint(x + d, y + d);
//            auto p5 = QPoint(x + d, y + d - sy);

//            ctx.drawLine(p0, p1);
//            ctx.drawLine(p2, p1);

//            ctx.drawLine(p3, p4);
//            ctx.drawLine(p5, p4);
//        }
//    }

}
