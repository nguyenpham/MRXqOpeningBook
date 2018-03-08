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

#include "OpBoard.h"
#include <assert.h>

namespace opening {

    extern const char* pieceTypeName;
    extern const int egtbPieceListStartIdxByType[7];
    extern const PieceType egtbPieceListIdxToType[16];

    const int egtbPieceListStartIdxByType[7] = { 0, 1, 3, 5, 7, 9, 11 };

    // king, advisor, elephant, rook, cannon, horse, pawn, empty
    const char* pieceTypeName = "kaerchp.";

    const PieceType egtbPieceListIdxToType[16] = {
        PieceType::king,
        PieceType::advisor, PieceType::advisor,
        PieceType::elephant, PieceType::elephant,
        PieceType::rook, PieceType::rook,
        PieceType::cannon, PieceType::cannon,
        PieceType::horse, PieceType::horse,
        PieceType::pawn, PieceType::pawn, PieceType::pawn, PieceType::pawn, PieceType::pawn
    };

}

using namespace opening;

extern const u64 hashTable[];
static const std::string originalFen = "rneakaenr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNEAKAENR w - - 0 1";

void OpeningBoard::show(const char* msg) const {
    if (msg) {
        std::cout << msg << std::endl;
    }
    std::cout << toString() << std::endl;
}

std::string OpeningBoard::toString() const {
    std::ostringstream stringStream;

    stringStream << getFen(side) << std::endl;

    for (int i = 0; i < 90; i++) {
        auto pieceType = getPiece(i).type;
        auto side = getSide(i);

        stringStream << Piece::toString(pieceType, side) << " ";

        if (i > 0 && i % 9 == 8) {
            int row = 9 - i / 9;
            stringStream << " " << row << "\n";
        }
    }

    stringStream << "a b c d e f g h i  ";

    if (side != Side::none) {
        stringStream << (side == Side::white ? "w turns" : "b turns") << "\n";
    }
    return stringStream.str();
}

bool OpeningBoard::setup(const std::vector<Piece> pieceVec, Side _side) {
    pieceList_reset((int8_t *)pieceList);
    for (auto && p : pieces) {
        p.setEmpty();
    }

    side = _side;
    for (auto && p : pieceVec) {
        if (!isEmpty(p.pos)) {
            return false;
        }
        set(p.pos, p.type, p.side);
    }

    initHashKey();
    return true;
}

void OpeningBoard::setResult(const std::string& resultString) {
    if (resultString == "1-0") result = Result::win;
    else if (resultString == "0-1") result = Result::loss;
    else if (resultString == "1/2-1/2" || resultString == "0.5-0.5") result = Result::draw;
    else result = Result::noresult;
}

void OpeningBoard::setFen(const std::string& fen) {
    pieceList_reset((int8_t *)pieceList);
    for (auto && p : pieces) {
        p.setEmpty();
    }

    std::string thefen = fen;
    if (fen.empty()) {
        thefen = originalFen;
    }

    bool last = false;
    side = Side::white;

    for (int i=0, pos=0; i < (int)thefen.length(); i++) {
        char ch = thefen.at(i);

        if (ch==' ') {
            last = true;
            continue;
        }

        if (last) {
            if (ch=='w' || ch=='W') {
                side = Side::white;
            } else if (ch=='b' || ch=='B') {
                side = Side::black;
            }

            continue;
        }

        if (ch=='/') {
            continue;
        }

        if (ch>='0' && ch <= '9') {
            int num = ch - '0';
            pos += num;
            continue;
        }

        Side side = Side::black;
        if (ch >= 'A' && ch < 'Z') {
            side = Side::white;
            ch += 'a' - 'A';
        }

        auto pieceType = PieceType::empty;
        const char* p = strchr(pieceTypeName, ch);
        if (p==NULL) {
            if (ch=='n') {
                pieceType = PieceType::horse;
            } else if (ch=='b') {
                pieceType = PieceType::elephant;
            }
        } else {
            int k = (int)(p - pieceTypeName);
            pieceType = static_cast<PieceType>(k);

        }
        if (pieceType != PieceType::empty) {
            set(pos, pieceType, side);
        }
        pos ++;
    }

    initHashKey();
}

std::string OpeningBoard::getFen(Side side, int halfCount, int fullMoveCount) const {
    std::ostringstream stringStream;

    int e=0;
    for (int i=0; i < 90; i++) {
        auto piece = getPiece(i);
        if (piece.isEmpty()) {
            e += 1;
        } else {
            if (e) {
                stringStream << e;
                e = 0;
            }
            stringStream << piece.toString();
        }

        if (i % 9 == 8) {
            if (e) {
                stringStream << e;
            }
            if (i < 89) {
                stringStream << "/";
            }
            e = 0;
        }
    }

    stringStream << (side == Side::white ? " w " : " b ") << halfCount << " " << fullMoveCount;

    return stringStream.str();
}

static const int8_t legalPosBits [7] = {
    1, 2, 4, 0, 0, 0, 8
};

static const int8_t legalPositions [90] = {
    0, 0, 4, 3, 1, 3, 4, 0, 0,
    0, 0, 0, 1, 3, 1, 0, 0, 0,
    4, 0, 0, 3, 5, 3, 0, 0, 4,
    8, 0, 8, 0, 8, 0, 8, 0, 8,
    8, 0,12, 0, 8, 0,12, 0, 8,

    8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8,
};

bool OpeningBoard::isValid() const
{
    int pieceCout[2][7] = {{0,0,0,0,0,0,0}, {0,0,0,0,0,0,0}};

    for(int i = 0; i < 90; i++) {
        auto p = pieces[i];
        if (p.isEmpty()) {
            continue;
        }
        auto sd = static_cast<int>(p.side);
        auto type = static_cast<int>(p.type);
        pieceCout[sd][type]++;

        auto flag = legalPosBits [type];
        if (flag) {
            auto k = p.side == Side::white ? (89 - i) : i;
            if ((flag & legalPositions [k]) == 0) {
//                printf("flag=%d, k=%d, legalPositions[k]=%d\n", flag, k, legalPositions [k]);
//                printOut("Wrong board");
                return false;
            }
        }
    }

    return
        pieceCout[0][0] == 1 && pieceCout[1][0] == 1 &&
        pieceCout[0][1] <= 2 && pieceCout[1][1] <= 2 &&
        pieceCout[0][2] <= 2 && pieceCout[1][2] <= 2 &&
        pieceCout[0][3] <= 2 && pieceCout[1][3] <= 2 &&
        pieceCout[0][4] <= 2 && pieceCout[1][4] <= 2 &&
        pieceCout[0][5] <= 2 && pieceCout[1][5] <= 2 &&
        pieceCout[0][6] <= 5 && pieceCout[1][6] <= 5;
}

void OpeningBoard::pieceList_reset(int8_t *pieceList) {
    for(int i = 0; i < 16; i++) {
        pieceList[i] = pieceList[16 + i] = -1;
    }
}

bool OpeningBoard::pieceList_set(int8_t *pieceList, int pos, PieceType type, Side side) {
    int d = side == Side::white ? 16 : 0;
    for (int t = egtbPieceListStartIdxByType[static_cast<int>(type)]; ; t++) {
        if (t >= 16) {
            break;
        }
        if (pieceList[d + t] < 0 || pieceList[d + t] == pos) {
            pieceList[d + t] = pos;
            return true;
        }
    }
    return false;
}

bool OpeningBoard::pieceList_setEmpty(int8_t *pieceList, int pos, PieceType type, Side side) {
    int d = side == Side::white ? 16 : 0;
    for (int t = egtbPieceListStartIdxByType[static_cast<int>(type)]; ; t++) {
        if (t >= 16) {
            break;
        }
        if (pieceList[d + t] == pos) {
            pieceList[d + t] = -1;
            return true;
        }
    }
    return false;
}

bool OpeningBoard::pieceList_setEmpty(int8_t *pieceList, int pos) {
    for (int sd = 0; sd < 2; sd ++) {
        if (pieceList_setEmpty(pieceList, pos, sd)) {
            return true;
        }
    }
    return false;
}

bool OpeningBoard::pieceList_setEmpty(int8_t *pieceList, int pos, int sd) {
    int d = sd == 0 ? 0 : 16;
    for(int i = 0; i < 16; i++) {
        if (pieceList[i + d] == pos) {
            pieceList[i + d] = -1;
            return true;
        }
    }

    return false;
}

bool OpeningBoard::pieceList_isThereAttacker(const int8_t *pieceList) {
    for(int i = 5; i < 16; i ++) {
        if (pieceList[i] >= 0 || pieceList[i + 16] >= 0) return true;
    }
    return false;
}

bool OpeningBoard::pieceList_make(const Hist& hist) {
    if (!hist.cap.isEmpty()) {
        bool ok = false;
        for (int t = egtbPieceListStartIdxByType[static_cast<int>(hist.cap.type)], sd = static_cast<int>(hist.cap.side); ; t++) {
            if (t >= 16) {
                break;
            }
            if (pieceList[sd][t] == hist.move.dest) {
                pieceList[sd][t] = -1;
                ok = true;
                break;
            }
        }
        if (!ok) {
            return false;
        }
    }
    for (int t = egtbPieceListStartIdxByType[static_cast<int>(hist.move.type)], sd = static_cast<int>(hist.move.side); ; t++) {
        if (t >= 16) {
            break;
        }
        if (pieceList[sd][t] == hist.move.from) {
            pieceList[sd][t] = hist.move.dest;
            return true;
        }
    }
    return false;
}

bool OpeningBoard::pieceList_takeback(const Hist& hist) {
    bool ok = false;
    for (int t = egtbPieceListStartIdxByType[static_cast<int>(hist.move.type)], sd = static_cast<int>(hist.move.side); ; t++) {
        if (t >= 16) {
            break;
        }
        if (pieceList[sd][t] == hist.move.dest) {
            pieceList[sd][t] = hist.move.from;
            ok = true;
            break;
        }
    }
    if (!ok) {
        return false;
    }
    if (hist.cap.isEmpty()) {
        return true;
    }
    for (int t = egtbPieceListStartIdxByType[static_cast<int>(hist.cap.type)], sd = static_cast<int>(hist.cap.side); ; t++) {
        if (t >= 16) {
            break;
        }
        if (pieceList[sd][t] < 0) {
            pieceList[sd][t] = hist.move.dest;
            return true;
        }
    }
    return false;
}

bool OpeningBoard::pieceList_setupBoard(const int8_t *thePieceList) {
    reset();

    if (thePieceList == nullptr) {
        thePieceList = (const int8_t *) pieceList;
    } else {
        pieceList_reset((int8_t *)pieceList);
    }

    for (int sd = 0, d = 0; sd < 2; sd++, d = 16) {
        Side side = static_cast<Side>(sd);
        for(int i = 0; i < 16; i++) {
            auto pos = thePieceList[d + i];
            if (pos >= 0) {
                if (!isEmpty(pos)) {
                    return false;
                }
                auto type = egtbPieceListIdxToType[i];
                set(pos, type, side);
            }
        }
    }

    return true;
}

static const int flip_h[90] = {
    8, 7, 6, 5, 4, 3, 2, 1, 0,
    17,16,15,14,13,12,11,10, 9,
    26,25,24,23,22,21,20,19,18,
    35,34,33,32,31,30,29,28,27,
    44,43,42,41,40,39,38,37,36,
    53,52,51,50,49,48,47,46,45,
    62,61,60,59,58,57,56,55,54,
    71,70,69,68,67,66,65,64,63,
    80,79,78,77,76,75,74,73,72,
    89,88,87,86,85,84,83,82,81
};

static const int flip_v[90] = {
    81, 82,83,84,85,86,87,88,89,
    72, 73,74,75,76,77,78,79,80,
    63, 64,65,66,67,68,69,70,71,
    54, 55,56,57,58,59,60,61,62,
    45, 46,47,48,49,50,51,52,53,

    36, 37,38,39,40,41,42,43,44,
    27, 28,29,30,31,32,33,34,35,
    18, 19,20,21,22,23,24,25,26,
    9, 10,11,12,13,14,15,16,17,
    0,  1, 2, 3, 4, 5, 6, 7, 8
};


int OpeningBoard::flip(int pos, FlipMode flipMode) {
    switch (flipMode) {
        case FlipMode::none: return pos;
        case FlipMode::horizontal: return flip_h[pos];
        case FlipMode::vertical: return flip_v[pos];
        case FlipMode::rotate: return 89 - pos;    // around
        default:
            break;
    }
    return 0;
}

static const FlipMode flipflip_h[] = { FlipMode::horizontal, FlipMode::none, FlipMode::rotate, FlipMode::vertical };
static const FlipMode flipflip_v[] = { FlipMode::vertical, FlipMode::rotate, FlipMode::none, FlipMode::horizontal };
static const FlipMode flipflip_r[] = { FlipMode::rotate, FlipMode::vertical, FlipMode::horizontal, FlipMode::none };

FlipMode OpeningBoard::flip(FlipMode oMode, FlipMode flipMode) {
    switch (flipMode) {
        case FlipMode::none:
            break;
        case FlipMode::horizontal:
            return flipflip_h[static_cast<int>(oMode)];

        case FlipMode::vertical:
            return flipflip_v[static_cast<int>(oMode)];
        case FlipMode::rotate:
            return flipflip_r[static_cast<int>(oMode)];
    }
    return oMode;
}

void OpeningBoard::flip(FlipMode flipMode) {
    if (flipMode == FlipMode::none) {
        return;
    }
    int8_t bkPieceList[2][16];
    memcpy(bkPieceList, pieceList, sizeof(bkPieceList));

    reset();
    pieceList_reset((int8_t *)pieceList);

    for(int sd = 0; sd < 2; sd++) {
        Side side = static_cast<Side>(sd);

        if (flipMode == FlipMode::vertical || flipMode == FlipMode::rotate) {
            side = getXSide(side);
        }
        for(int i = 0; i < 16; i++) {
            int pos = bkPieceList[sd][i];
            if (pos < 0) {
                continue;
            }
            int8_t newpos = flip(pos, flipMode);
            PieceType type = egtbPieceListIdxToType[i];
            set(newpos, type, side);
        }
    }

    initHashKey();
}


void OpeningBoard::xorHashKey(int pos) {
    assert(pos >= 0 && pos < 90);
    assert(!pieces[pos].isEmpty());
    int sd = static_cast<int>(pieces[pos].side);
    int p = static_cast<int>(pieces[pos].type);
    int h = sd * 7 * 90 + p * 90 + pos;
    hashKey ^= hashTable[h];
}

void OpeningBoard::initHashKey() {
    hashKey = 0;
    for(int i = 0; i < 90; i++) {
        if (!pieces[i].isEmpty()) {
            xorHashKey(i);
        }
    }
}

void OpeningBoard::make(const Move& move, Hist& hist) {
    auto hk = hashKey; initHashKey();
    assert(hk == hashKey);


    hist.move = move;
    hist.cap = pieces[move.dest];
    hist.hashKey = hashKey;

    xorHashKey(move.from);
    if (!hist.cap.isEmpty()) {
        xorHashKey(move.dest);
    }

    pieces[move.dest] = pieces[move.from];
    pieces[move.from].setEmpty();

    xorHashKey(move.dest);

    pieceList_make(hist);

    auto hk1 = hashKey; initHashKey();
    assert(hk1 == hashKey);
}

void OpeningBoard::takeBack(const Hist& hist) {
    hashKey = hist.hashKey;
    pieces[hist.move.from] = pieces[hist.move.dest];
    pieces[hist.move.dest] = hist.cap;

    pieceList_takeback(hist);
}

void OpeningBoard::make(const Move& move)
{
    Hist hist;
    make(move, hist);

    side = getXSide(side);
    histList.push_back(hist);
}

void OpeningBoard::takeBack()
{
    side = getXSide(side);
    auto hist = histList.back();
    takeBack(hist);
    histList.pop_back();
}

int OpeningBoard::findKing(Side side) const
{
    auto sd = static_cast<int>(side);
    auto kingpos = pieceList[sd][0];
    return kingpos;
}

void OpeningBoard::genLegal(MoveList& moves, Side side) {

    MoveList moveList;
    gen(moveList, side, false);

    Hist hist;
    for (int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        make(move, hist);
        if (!isIncheck(side)) {
            moves.add(move);
        }
        takeBack(hist);
    }
}

void OpeningBoard::gen_addMove(MoveList& moves, int from, int dest, bool captureOnly) const {
    auto toSide = pieces[dest].side;

    if (pieces[from].side != toSide && (!captureOnly || toSide != Side::none)) {
        moves.add(pieces[from].type, pieces[from].side, from, dest);
    }
}

void OpeningBoard::gen(MoveList& moves, Side side, bool captureOnly) const {
    moves.reset();
    for (int l = 0; l < 16; l++) {
        auto pos = pieceList[static_cast<int>(side)][l];
        if (pos < 0) {
            continue;
        }
        auto piece = pieces[pos];

        switch (piece.type) {
            case PieceType::king:
            {
                int col = pos % 9;
                if (col != 3) { // go left
                    gen_addMove(moves, pos, pos - 1, captureOnly);
                }
                if (col != 5) { // right
                    gen_addMove(moves, pos, pos + 1, captureOnly);
                }
                if (pos > 72 || (pos > 8 && pos < 27)) { // up
                    gen_addMove(moves, pos, pos - 9, captureOnly);
                }
                if (pos < 18 || (pos > 63 && pos < 81)) { // down
                    gen_addMove(moves, pos, pos + 9, captureOnly);
                }
                break;
            }

            case PieceType::advisor:
            {
                int y = pos - 10;   /* go left up */
                if (y == 3 || y == 13 || y == 66 || y == 76) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos - 8;        /* go right up */
                if (y == 5 || y == 13 || y == 68 || y == 76) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 8;        /* go left down */
                if (y == 13 || y == 21 || y == 84 || y == 76) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 10;       /* go right down */
                if (y == 13 || y == 23 || y == 76 || y == 86) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                break;
            }

            case PieceType::elephant:
            {
                int y = pos - 20; /* go left up */
                if ((y == 2 || y == 6 || y == 18 || y == 22 || y == 47 || y == 51 || y == 63 || y == 67) && isEmpty(pos - 10)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos - 16; /* go right up */
                if ((y == 2 || y == 6 || y == 22 || y == 26 || y == 47 || y == 51 || y == 67 || y == 71) && isEmpty(pos - 8)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 16; /* go left down */
                if ((y == 18 || y == 22 || y == 38 || y == 42 || y == 63 || y == 67 || y == 83 || y == 87) && isEmpty(pos + 8)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 20; /* go right up */
                if ((y == 22 || y == 26 || y == 38 || y == 42 || y == 67 || y == 71 || y == 83 || y == 87) && isEmpty(pos + 10)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                break;
            }

            case PieceType::cannon: {
                int col = pos % 9;
                /*
                 * go left
                 */
                int f = 0;

                for (int y=pos - 1; y >= pos - col; y--) {
                    if (isEmpty(y)) {
                        if (f == 0 && !captureOnly) {
                            gen_addMove(moves, pos, y, captureOnly);
                        }
                        continue;
                    }
                    f++;
                    if (f == 2) {
                        gen_addMove(moves, pos, y, captureOnly);
                        break;
                    }
                }
                /*
                 * go right
                 */
                f = 0;
                for (int y=pos + 1; y < pos - col + 9; y++) {
                    if (isEmpty(y)) {
                        if (f == 0 && !captureOnly) {
                            gen_addMove(moves, pos, y, captureOnly);
                        }
                        continue;
                    }
                    f++;
                    if (f == 2) {
                        gen_addMove(moves, pos, y, captureOnly);
                        break;
                    }
                }

                f = 0;
                for (int y=pos - 9; y >= 0; y -= 9) { /* go up */
                    if (isEmpty(y)) {
                        if (f == 0 && !captureOnly) {
                            gen_addMove(moves, pos, y, captureOnly);
                        }
                        continue;
                    }
                    f += 1 ;
                    if (f == 2) {
                        gen_addMove(moves, pos, y, captureOnly);
                        break;
                    }
                }

                f = 0;
                for (int y=pos + 9; y < 90; y += 9) { /* go down */
                    if (isEmpty(y)) {
                        if (f == 0 && !captureOnly) {
                            gen_addMove(moves, pos, y, captureOnly);
                        }
                        continue;
                    }
                    f += 1 ;
                    if (f == 2) {
                        gen_addMove(moves, pos, y, captureOnly);
                        break;
                    }
                }

                break;
            }

            case PieceType::rook:
            {
                int col = pos % 9;
                for (int y=pos - 1; y >= pos - col; y--) { /* go left */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }

                for (int y=pos + 1; y < pos - col + 9; y++) { /* go right */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }
                }

                for (int y=pos - 9; y >= 0; y -= 9) { /* go up */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }

                }

                for (int y=pos + 9; y < 90; y += 9) { /* go down */
                    gen_addMove(moves, pos, y, captureOnly);
                    if (!isEmpty(y)) {
                        break;
                    }

                }
                break;
            }

            case PieceType::horse:
            {
                int col = pos % 9;
                int y = pos - 11;
                int z = pos - 1;
                if (y >= 0 && col > 1 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos - 19;
                z = pos - 9;
                if (y >= 0 && col > 0 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos - 17;
                z = pos - 9;
                if (y >= 0 && col < 8 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos - 7;
                z = pos + 1;
                if (y >= 0 && col < 7 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }

                y = pos + 7;
                z = pos - 1;
                if (y < 90 && col > 1 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 17;
                z = pos + 9;
                if (y < 90 && col > 0 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 19;
                z = pos + 9;
                if (y < 90 && col < 8 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                y = pos + 11;
                z = pos + 1;
                if (y < 90 && col < 7 && isEmpty(z)) {
                    gen_addMove(moves, pos, y, captureOnly);
                }
                break;
            }

            case PieceType::pawn:
            {
                if ((side == Side::black && pos > 44) || (side == Side::white && pos < 45)) {
                    int col = pos % 9;
                    /* go left */
                    if (col > 0) {
                        gen_addMove(moves, pos, pos - 1, captureOnly);
                    }
                    /* go right */
                    if (col < 8) {
                        gen_addMove(moves, pos, pos + 1, captureOnly);
                    }
                }

                if (side == Side::black) {
                    /* go down */
                    if (pos < 81) {
                        gen_addMove(moves, pos, pos + 9, captureOnly);
                    }
                } else {
                    /* go up */
                    if (pos > 8) {
                        gen_addMove(moves, pos, pos - 9, captureOnly);
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

bool OpeningBoard::isIncheck(Side beingAttackedSide) const
{
    int kingPos = findKing(beingAttackedSide);

    Side attackerSide = getXSide(beingAttackedSide);

    /*
     * Check horizontal and vertical lines for attacking of Rook, Cannon and
     * King face
     */

    /* go down */
    int y = kingPos + 9;
    if (y < 90) {
        int f = 0;
        auto p = pieces[y];
        if (!p.isEmpty()) {
            f = 1;
            if (p.side == attackerSide && (p.type == PieceType::rook || (p.type == PieceType::pawn && attackerSide == Side::white))) {
                return true;
            }
        }

        for (int yy = y+9; yy < 90; yy+=9) {
            auto p = pieces[yy];
            if (p.isEmpty()) {
                continue;
            }
            f++;
            if (p.side == attackerSide) {
                if ((f == 1 && (p.type == PieceType::rook || p.type == PieceType::king)) || (f == 2 && p.type == PieceType::cannon)) {
                    return true;
                }
            }
            if (f == 2) {
                break;
            }
        }
    }

    /* go left */
    y = kingPos - 1;
    int f = 0;

    auto p = pieces[y];
    if (!p.isEmpty()) {
        f = 1;
        if (p.side == attackerSide && (p.type == PieceType::rook || p.type == PieceType::pawn)) {
            return true;
        }
    }

    int col = kingPos % 9;

    for (int yy = y-1; yy >= kingPos - col; yy--) {
        auto p = pieces[yy];
        if (p.isEmpty()) {
            continue;
        }
        f++;
        if (p.side == attackerSide) {
            if ((f == 1 && p.type == PieceType::rook) || (f == 2 && p.type == PieceType::cannon)) {
                return true;
            }
        }
        if (f == 2) {
            break;
        }
    }

    /* go right */
    y = kingPos + 1;
    f = 0;
    p = pieces[y];
    if (!p.isEmpty()) {
        f = 1;
        if (p.side == attackerSide && (p.type == PieceType::rook || p.type == PieceType::pawn)) {
            return true;
        }
    }

    for (int yy = y+1; yy < kingPos - col + 9; yy++) {
        auto p = pieces[yy];
        if (p.isEmpty()) {
            continue;
        }
        f++;
        if (p.side == attackerSide) {
            if ((f == 1 && p.type == PieceType::rook) || (f == 2 && p.type == PieceType::cannon)) {
                return true;
            }
        }
        if (f == 2) {
            break;
        }
    }

    /* go up */
    y = kingPos - 9;
    if (y >= 0) {
        f = 0;
        p = pieces[y];
        if (!p.isEmpty()) {
            f = 1;
            if (p.side == attackerSide && (p.type == PieceType::rook || (p.type == PieceType::pawn && attackerSide == Side::black))) {
                return true;
            }
        }

        for (int yy = y-9; yy >= 0; yy-=9) {
            auto p = pieces[yy];
            if (p.isEmpty()) {
                continue;
            }
            f++;
            if (p.side == attackerSide) {
                if ((f == 1 && (p.type == PieceType::rook || p.type == PieceType::king)) || (f == 2 && p.type == PieceType::cannon)) {
                    return true;
                }
            }
            if (f == 2) {
                break;
            }
        }
    }

    /* Check attacking of Knight */
    if (kingPos > 9 && isPiece(kingPos - 11, PieceType::horse, attackerSide) && isEmpty(kingPos - 10)) {
        return true;
    }
    if (kingPos > 18 && isPiece(kingPos - 19, PieceType::horse, attackerSide) && isEmpty(kingPos - 10)) {
        return true;
    }
    if (kingPos > 18 && isPiece(kingPos - 17, PieceType::horse, attackerSide) && isEmpty(kingPos - 8)) {
        return true;
    }
    if (kingPos > 9 && isPiece(kingPos - 7, PieceType::horse, attackerSide) && isEmpty(kingPos - 8)) {
        return true;
    }
    if (kingPos < 81 && isPiece(kingPos + 7, PieceType::horse, attackerSide) && isEmpty(kingPos + 8)) {
        return true;
    }
    if (kingPos < 72 && isPiece(kingPos + 17, PieceType::horse, attackerSide) && isEmpty(kingPos + 8)) {
        return true;
    }
    if (kingPos < 72 && isPiece(kingPos + 19, PieceType::horse, attackerSide) && isEmpty(kingPos + 10)) {
        return true;
    }
    if (kingPos < 81 && isPiece(kingPos + 11, PieceType::horse, attackerSide) && isEmpty(kingPos + 10)) {
        return true;
    }

    return false;
}

namespace opening {

const u64 originHashKey = 6665246470371296167ULL;

const u64 hashTable[] = {
    0x1b10fc65ff28f1e, 0x64d79b552a559d7f, 0x44a572665a6ee240, 0xeb2bf6dc3d72135c, 0xe3836981f9f82ea0, 0x43a38212350ee392, 0xce77502bffcacf8b, 0x5d8a82d90126f0e7, 0xc0510c6f402c1e3c,
    0x48d895bf8b69f77b, 0x8d9fbb371f1de07f, 0x1126b97be8c91ce2, 0xf05e1c9dc2674be2, 0xe4d5327a12874c1e, 0x7c1951ea43a7500d, 0xbba2bbfbecbc239a, 0xc5704350b17f0215,
    0x823a67c5f88337e7, 0x9fbe3cfcd1f08059, 0xdc29309412e352b9, 0x5a0ff7908b1b3c57, 0x46f39cb43b126c55, 0x9648168491f3b126, 0xdd3e72538fd39a1c, 0x348399b7d2b8428,
    0xcf36d95eae514f52, 0x7b9231d5308d7534, 0xb225e28cfc5aa663, 0xa833f6d5c72448a4, 0xdaa565f5815de899, 0x4b051d1e4cc78eb8, 0xef6a48be001729c7, 0xfdc570ba2fe979fb,
    0xb57697121dfdfe93, 0x96524e209b767c29, 0xb313b667a4d999d6, 0x7c7fa1bd6fd7dea, 0xee9f4c15c57e92a, 0xc5fb71b8f4bf5f56, 0xa251f93a4b335492, 0x933792382b0218a3,
    0x7d497d2f7a15eaf8, 0xb2f0624214f522a2, 0x22d91b683f251121, 0x3462898a2ae7b001, 0x468bc3a10a34890c, 0x84ff6ce56552b185, 0xed95ff232c511188, 0x4869be47a8137c83,
    0x934606951e6fcd81, 0x1ab5e8e453bde710, 0xd870d281b293af3d, 0xc3a5f903a836fafd, 0x88bd6a24d49cd77, 0x4e38ddc2719162a5, 0xf48286b4f22cad94, 0x7450f7229f336762,
    0xb75b43fb4c81784a, 0x562f36fae610a2e1, 0xe0e413e555bd736, 0xd452549efe08402d, 0x74ac5663c6c4f45b, 0xd84c9a356858060, 0x19d5b3643bc029b6, 0xdd8131e97ffc842,
    0xf98c6d63ff48a16e, 0x83490ef054537f7e, 0xe071f833e55ebfe6, 0xba15364649384016, 0xdae6e879b8eab74a, 0xe4a41f17e70d3e0a, 0x56e10c00dd580f70, 0xbc1abca3fbeeb2f3,
    0x8f760d9228900a4, 0xda11a24c514b9dc7, 0x41c11098f6123861, 0x5e37299d89089ba4, 0xa1f735eb8235b32f, 0x2289d719e7b146ee, 0x1c9c9d0284d96719, 0x317e34c009a07a39,
    0x5f45053666f3f84f, 0x63e7074f03c73d92, 0x22080cf23288e895, 0x9643b3707db2cfb5, 0x98e2db6c665e7178, 0x36e7ac11d1f3a617, 0x508f0acb609bd756, 0x6f42d435193a1ac2,
    0x2df2cab9d65e0b00, 0x4584c1fde5f1ad55, 0x602c500bdc8317c2, 0xc80d5b04f6337337, 0x5d33cf557e6c4475, 0x5b5a78be74ccd40, 0x3ec2cce5290785f4, 0x2eef1e9c4b36828b,
    0x3c4cb919b35c221c, 0x7ff345b4eb7f3639, 0xa15f9c2826cb34db, 0x64822b68adefa772, 0xa5d049ab1061dba3, 0xc1cdaa5f9ca79b19, 0x35b04c42565ebe2, 0x5094cafab11cbc3a,
    0x94d40a57481346b5, 0x126a70b84f779acc, 0x6564e506d11e9de1, 0xd493e410b68b6c6, 0x8c656651dbb3ed84, 0x2819237e5e8cb03a, 0x6cd436fd5f7c1e73, 0xf03b845f2a584931,
    0x8847b9f1f2d55b7a, 0x91e060fb399ecf2c, 0x5748fbea12dd2869, 0xa35e9dfa5a64f56a, 0x5e650b9a3cb34308, 0xe482d24e5b239833, 0xb0648475f2b4d63f, 0x332f7ce3e077ecce,
    0x1ead372c068ebb04, 0x5b057c64bda94a33, 0x11e1388f59653c66, 0xe974ffd57219e5e6, 0xe7819b2cb44db4c0, 0x6750ecdf35c8471b, 0x269e0cf307467534, 0x2c6983c911c958af,
    0xb2fd7c07ae0bfa60, 0x3220a56d67450e80, 0x7385883420eb9f69, 0x167cc7f46a511504, 0x1194815038360d85, 0x8f287f1a3be6e458, 0x9f003e8e0e9c6c0, 0x9ef3b8df89ea4c29,
    0x9ba96cb53c24408b, 0x501337e1a7f1e218, 0x589b8eaea60d54a4, 0x68ccb8c6cd7ec24d, 0x8f1a8c1b84b3ba62, 0xdd7222eaee54e524, 0xf69ffb64131633ae, 0x11753aea03bb0ecd,
    0xcb729235e6876ee, 0x94c1190da321d470, 0x321e26bf321fb60b, 0xc9202ac8ba71c873, 0x284d02d7552a3e90, 0xf7bd5a988596c1bb, 0xccf835db6a10d2df, 0xa6e86f7ba2779a5c,
    0xd798bd6d52ad26da, 0x218f6912af38b818, 0x8f675048b7b012e5, 0xe5e469aac68eaf1d, 0x341774636bdc8f41, 0x4c2fb1c45b749ced, 0xcc4e27afa81bc8a8, 0x38fa9cb2ce72bc16,
    0x45de92e997e2915c, 0xcd9b08d4dcda27ba, 0x1f7495a5f36c34ce, 0x423ae115ec99d154, 0x517fc1107eaa6a83, 0xc5967cdf353aeac6, 0xab1b908b97dc911b, 0xf3d84c286f22611c,
    0x59926f3401f437d4, 0xfb49ab0635d66c3a, 0xe2e6bf8cb6e29717, 0x316558d69efc8f6b, 0x2125a2be5bf67eb3, 0xfa9f65d2b4848b12, 0x2b92665a263a5091, 0xa879920d28c0d54c,
    0x4171d5edd5468876, 0xfc24806650c823bd, 0xae43f70f181d9371, 0x978499ba4193b333, 0x99bb4bf79b0a46c1, 0x1bc1741ce05c4cce, 0x939ed8b0d7e91f87, 0xc8f129ec69ce7811,
    0x7ecd448787517eba, 0xbf2f4f23a6c92295, 0x8b7aad8ffea1b991, 0x2fda11cd74177e6b, 0x7890bc68793bb959, 0x2a060f45a1719347, 0xd7c4fd7921707400, 0xbfb945ef1cf94d1d,
    0x6679c449ac22edc0, 0x2578b45bc6f34cab, 0x1b192f0be8dc77a7, 0xe1e4c2924f35192e, 0x4b7519cb9028ac83, 0xda2ffb6a863c850b, 0xe38239260b584150, 0x1528080b61f54198,
    0x1617db48eb6640d0, 0x37056e05b4724179, 0x2fc96b31dccafd9f, 0xe9fb2e3998d16a25, 0xd5340e98fde806de, 0xca4625581bf9dbe1, 0x48a89c5424b4cdb, 0x5510b9cd8306839f,
    0xc1f2f613ee3c2e21, 0x3a9fa27258257e53, 0x9cb572b041015355, 0x21b7616ced869ef1, 0x8f23aff44d352f03, 0xf1bea47e2c68a4c, 0x9fa0178362df447b, 0xc11c63febf83598a,
    0x80cd4f9f23fcafdd, 0x451717a061972d1f, 0x3cbae97a12632fbe, 0xded6960c0be007a8, 0x289de4875bda262d, 0x99f5a15f635296a6, 0x1a32471bd26acd7a, 0x6ab23720409790a7,
    0xac7c9f339a822344, 0xf9724a82872bd619, 0xfde48d87e844ec93, 0xf2ebc493b909d8c7, 0x13834d3d0e43ab73, 0xfe0b1d7e4f0ef297, 0x563de834c4e56a46, 0xd4680470b790968,
    0x9ec02036a6688a24, 0x8c9a454af9e09984, 0x686cfe3ea1889281, 0x763451110c00291b, 0xb894e0ae40a9c87f, 0xda7211029d59f04e, 0x57833aa39ef75a04, 0x2f5725cf5fc583ae,
    0xad29cce061ba3934, 0x1119e4c257ccf7fb, 0xd2e58dfb1a8e3866, 0x60de1f1050684297, 0xa808b477fd65fb4a, 0x3f143cd0bc243fbe, 0x43e20045bcb06b0e, 0x45d80e6bd330d613,
    0xf2a0dd678f92e0a5, 0x55199357cdc55491, 0x1a5ff3356d77d43b, 0xacbc2b8d1fb16ea8, 0xc087a2116f52d0fc, 0x2df8fbe8993f8c9f, 0x9163a77a53e361b8, 0x23c0723e123a899,
    0xcc6245a26564a399, 0xff4d956ffea8dea4, 0x77c8da91b5313675, 0x8d8325e82c4fdcdc, 0xb8b4ec771c41de8d, 0xb643a5be44efc242, 0x2d813db4b75c2ba3, 0x7f091b8c8e38de5b,
    0xbed325d9f9110749, 0xdbfd0251e39f75a1, 0xb361c40e95545795, 0x304dc11e60777da0, 0xd6289522aae19fdf, 0x8cad789f53948c3e, 0x3a321e3dadbf9e95, 0xb0368ec353973891,
    0xc2bc97dd4ca20950, 0xfb2c63179fe43fd7, 0x16c3269e96bb6bf5, 0x18e883239eac0617, 0x623efdfbd13bdb21, 0x73546837f894d0ca, 0x40dc52653efbccef, 0x7fa459494d559385,
    0x6de01919243ae5bb, 0xd12fa9e5d03723d1, 0xf66dbe12e01b8550, 0x3005b6b521106660, 0x7533904c075f66ee, 0xac1e7ecfae282b1d, 0x20134ff91431aec4, 0xbc5de6e4be75aece,
    0xf6cb3a49ed882e3f, 0xb6d432f6eb4ef80a, 0xd8f92f55d92198f8, 0x6fedea0b61190b9e, 0x4a4eec2c400cfdf6, 0x4f4675ef1285062, 0x7778b73e852e4d15, 0x9110c768339a9e41,
    0x62e1aa08e295e3e9, 0xe6d8713a75a6b5ab, 0xc00d8a3297364ec9, 0xabd7363aa145610, 0xab157fde82da45dc, 0x2f2228a01b8d0af9, 0xb193700fe9937383, 0x99ffa7e03de2876b,
    0x9f762dbfc2636735, 0xa33e0e71eee35602, 0x617864f443ee06a2, 0x54ea15dbd9dac4b2, 0xf71744143a9f1bf7, 0xa1210472d9abf2e6, 0x64e32987db997537, 0xcb9ff59da5101e29,
    0xfbd29f4b17438163, 0xb0880848c6f0ec2d, 0xeaf702e9c8f09c99, 0xb7215a02ed403166, 0x13fe2344d1b464ef, 0x6350032d7bcf4b4b, 0x5cf1942a1d22ec7, 0xe028694c06b610be,
    0x4b76a4e0c78915c9, 0x55a0a60d77bc7827, 0xc6a8c29b036a0730, 0xb5cfec5dfbcfc55b, 0xbfd4b617b5c35210, 0xaced1547c8c265c, 0xc509ee0da78f0100, 0x49656d78e56d3a8c,
    0x6f9c7605867795d2, 0x62b140fd317c156c, 0x2a21ad591bae819b, 0x63d6678fcca141e1, 0xed189cc0bc77c7a1, 0xbd4eb34eeafea8ef, 0xa4fb58de76c1ab4c, 0x30d9c2be1d428a2a,
    0x504af3dd67922522, 0xd1ac3000700c4b06, 0x4ff1d166ddad31bc, 0xc62be72b065f76db, 0x54b2d31700f88be2, 0xf9f9ee1a87c0244b, 0xa39a0b0fafe21a79, 0xb1058b188e517af0,
    0xc9e18bfefc688db5, 0x6b43e0e0f4663132, 0xa7a8944bca742f16, 0xaff5d9ae39262a25, 0x2d148c7e42121b0b, 0x699ebe7735f00c1c, 0x17645a5c713dad56, 0xcaeb707fbf778434,
    0x2e5acf8acd8e2524, 0xab646c19c1eed822, 0xd2a77d0263a1580b, 0x39846bce19845617, 0x5e516b5f43dc0082, 0xc10a56539405c997, 0x3c6c6515e626b095, 0x13bf57be6a2c43fb,
    0x500015453ba6177c, 0x90d5c93d80b81993, 0xd09c2326cc77ee1c, 0x63a2de8d0b64eab1, 0x569e425df8a7a02d, 0x1eb61d0c7a0b470e, 0xdd7cc68bb21c92e4, 0xec32155b844684cc,
    0x761c932d41ebe627, 0xfd73b26c0483e111, 0xdff5997136988f45, 0xcf901e304708c5a4, 0xb13db1062b25ecca, 0xdfd94982c2edcc14, 0xf5bb892719b1929f, 0x4219bf39ed86bd9,
    0x12ae9403548451f5, 0x45aeae9c28adc992, 0xfc0851d924e44e77, 0xac51ca985eb2b5e8, 0x65c2e43689a6d3bf, 0x9b7c78cc82a4564a, 0x12e29d61fa2df99a, 0x3d8ae088a0410552,
    0x86f2ad4bb400687c, 0x9921b356a217e365, 0x7b74cd841c609c38, 0x5ed1b272241d7966, 0xd72bc9d2957e5d24, 0x96289e543d287a0d, 0x5a64d34ac704f820, 0x8415a2d0a8d33d52,
    0xc219602af1f212d8, 0x80c62c714d94aa18, 0xfd71f7aa682fbfb, 0xeac8a0e90da113ea, 0x45036c3cd223f408, 0x8288b66680b40560, 0xc5c7b4ef48033849, 0x37d4ddc0cf3a05ab,
    0xc860cec36ce71664, 0x155b7524c44511cd, 0x4e06ab3bc4f080bf, 0xd4eddfe1ed5f5ab3, 0x14864cfccf8d73c8, 0x3be026325cf0c3d5, 0x2e624af6e7a45a1a, 0x81291fcdfa4e7924,
    0x80a88c71cc4bca2, 0x403852663ce10e8, 0xb41c46f454bf90d4, 0x85c649807d379682, 0x4b8afdd619ebf8db, 0x25a4d43826c54a0f, 0x2776e9b20138335e, 0xf46b78547ca77524,
    0xf43c6a2130ae7c88, 0x8d63faf7206df440, 0x2eff5b479ce2c011, 0xfe110bb08b44a041, 0x89d17649f4aa4338, 0xb36d31cc474b5f63, 0x16955cfdd268c3a5, 0x9003f0e2363bf869,
    0x480bbbee895d0fc0, 0xb795ebd239f14aed, 0x9af3aa2f0c32cac2, 0x30fa0ff335cc16a9, 0x6d83ffd9de86d72d, 0x7626b8d5ad908ccf, 0xd1cb7fe8ab9af3ca, 0x5be6b947811f633a,
    0x406703ffe936b27b, 0xed222c1176c3e959, 0x59af5618587fba7e, 0xfc784c49019dbbff, 0x2920f3a3b5f6d82c, 0x9f24b09e4858c9e1, 0x2af25a08cc217ef8, 0x55e156d36b7c11c9,
    0xd522f641e9eadfa8, 0x895d588451d31d40, 0xca4f01c7098fe4f7, 0x6fcb1190d140ee2, 0x19ccc4ed8af662dd, 0xdc04c61a9cfdd546, 0x4295b7a3145e2754, 0xa76c97d743394adc,
    0xa10b1ba432a94d7c, 0x5fb6e7f97cc37b1b, 0x5adb56d7fbdd6d5c, 0x1c8d6b641a7f7df5, 0x7bdec429aa1b5f6b, 0x66fd97d5315b8b02, 0x5332daf749334156, 0x4ec7142b652f7f94,
    0xe94d9f0b994612fa, 0x768e18a6544faad2, 0x19014180cf7d74ae, 0xf56b42802032a068, 0x2d7d2eaa48051d58, 0x254a596da0c8ef2a, 0xa0eadadb80af82eb, 0xcd8b1dde61cd426e,
    0x8ae76d95922d5d4c, 0x31ce7529803bbcb4, 0x4bbc0f56adefe3c5, 0xbd0f195c3982b975, 0xd7f293545626fa76, 0x2e4079c063293c9d, 0xb64e48256279c91f, 0xdd29612d111ec442,
    0x7cb10385ba574bf0, 0xeec5158c669109d6, 0x1dafebadd97034f, 0x1a2df99bf887e0c1, 0xef6cfab4eabca428, 0x6963950b6db80058, 0xe49128b9bbf79cf5, 0xd2e629e72428de59,
    0xd22eef96042510e0, 0x8eae2ab0b5374830, 0x660fb743d3b6607c, 0x122904648eea3490, 0x8608266609dfab9b, 0xa35870a0661897e8, 0x36516b5c556925, 0x7f3b16c693e08e62,
    0x14ff10bda44fc13c, 0xeced9c3cadb4717d, 0xeff85368de3aa141, 0xdf2ec75b67adaeba, 0x964dfe421f82d11b, 0x6576bb968c6a6cfa, 0xc2c7cf7c91934149, 0x4f53628a20e15c6d,
    0x2ad1a8c30408950d, 0x7a596aafd2463f84, 0x969ee394afae91d3, 0x9d13c3a577ab2ae4, 0x403ca1a5979f2822, 0xb7a712fc9ed1fdac, 0x5be7e5a57f934cfe, 0x3efd48503f47564a,
    0x54cdec188d6a7eef, 0xb8d2dea52bc9021f, 0xf785d70316ea28da, 0x2149665c40b48913, 0x1af77e4337e6b7be, 0xa84fab43e6084473, 0x3597a5009a7fca2f, 0x8d395226360c8eb3,
    0x8e1bffb76cf8e68e, 0x941e1fca928bcc27, 0x8605b3bf5442558b, 0x364527c96cead815, 0xd75ca8f09f5dd857, 0x33611ac5e1a2cd90, 0x57797788fb194ca3, 0x5938185c981c72e1,
    0x1218f60865c6cc6d, 0xad8e92693d4d6fb8, 0x3acaed72f61e4dc0, 0xf02e3a2a6cfab325, 0xe0bb1016d2df01c1, 0xdbd35a7ec753ec8f, 0x554dd6c5d20bfb6f, 0xd2e9cf54cd9cb796,
    0x6647815864d92ab, 0x3f4973c5b29a90bc, 0x131d35132e51517e, 0xc2b8048e1c9bfc00, 0xc4e3bb232bf10663, 0xf49b7883015f6e35, 0x7c3ec1602992cbba, 0x3e23a0edaa7865c5,
    0xbac241cbabb4885f, 0x7acbca4586293c7, 0x165a0f05f5161ad2, 0x987b4163e18c3446, 0xc5e3763b5840b7e7, 0x6e2fc59962e8306b, 0x690166e21fd52595, 0xc9bb76f40694ef49,
    0xde027de704d2592a, 0xd7dd0d892842847f, 0xe3f3ea756c9f144f, 0xf36fb7854afa624, 0xb8a78d9fcdd58fe5, 0x392fe65f94477963, 0xd5be774f4bce58c3, 0xdb798bb9db2fd66e,
    0x4965a21fc68e777c, 0x314a31e6f672e695, 0xa3b02141822b28e5, 0x30afa0305deb454c, 0x92983dad0ffdc006, 0xed3e1af073fd774f, 0xee5e32ffc4cb197f, 0x2ef0bf56fb429fb1,
    0x5c5155e6ac6a1f05, 0x918bb5aed878960a, 0xf22852b9631d249e, 0x42e9cf956246deae, 0x76813f1fbf71a82d, 0xe5fd64f73cc47670, 0x9943ed2326efcf8f, 0x6a2e8e63b60af7af,
    0x13d98add5e88e1d4, 0x3a7cfa298b4e65f9, 0xa403a934236e7df, 0x651acbf7e71d73a2, 0x724066c2154ffb44, 0xa418c21042c961ce, 0x18feed9270cf7437, 0x1c0a304ddfc0578f,
    0xecffdb80c767ffe5, 0x8f78bee843c5f927, 0xf481860125833fa6, 0x8282da067a9f7f47, 0xc99c187d8897b42, 0xa4be5150d798eadb, 0x1706a1520e9b6214, 0xffef02667ad27ed6,
    0x3c1422233680a4d6, 0x6f936ee6751cbafd, 0x6d12385a6a3526d4, 0xc5b9b859fb823f48, 0xf319d35773f3f1ea, 0x45c765944ce3707, 0x6d35ced186af3c33, 0xb994ea7bd176e248,
    0x49cece15a02a05ce, 0x32291e6b29b8d4eb, 0x4ec03d33af84b436, 0x119a2a33d02c9b6d, 0xa96bb9d05ee83341, 0x2a2a14aac1d24300, 0x2ad87dbd22b30f9b, 0x8073e358cb4deaf7,
    0x8a1d4661ee8c1dfd, 0x9b1c909129b98c36, 0x9727fbd1ecea89d, 0x472e5eb3de7aa533, 0x6b0ba0ce726bc62b, 0xcb2e539cf8cd5a91, 0xec95c12df641167b, 0xfb1e58032ac2d485,
    0x4c94fec521c56eda, 0x6c455ab535a5898f, 0x22e6be20e36a2a7a, 0x9a5d9ac2b0b8e3ea, 0x403701399e96d59d, 0xb1d21cc0bf85f46c, 0x8f45826a7e1f83fc, 0x8920d897154adfb,
    0x75f605f4a696864a, 0x72b7d1a137a313d9, 0xc480279394c1824b, 0xb9d5f7084e18b71d, 0x38c11cd9f7e310df, 0xb3663472607e01b3, 0x77684df14a925b73, 0x7cd1874f89253a17,
    0x43264eef6d41948e, 0x4cc2eb9efae4f15e, 0x6ab96c87eba7a0fe, 0x94f46625a918e7db, 0x98f7538686cc6ede, 0x1198bbf50ee3be2d, 0x9170501dd1e86f4d, 0x81e4954c54d1d3a8,
    0x7a3dcd206d3e7543, 0x172dae21663bfbb2, 0xb4dc1e8a51a481f, 0x6aa4a0e62d52f51a, 0x8de99a8a9c8365d7, 0xea691440004f4ed0, 0x366a00f7943995e2, 0x351d7b0d75998a4b,
    0x59d3a90952b3027d, 0xf0de579afc51bbf9, 0x4047241eb1c1ab06, 0x7180b01eff8a4ff7, 0xd80b9183cf8609bc, 0x717edba2d210504d, 0x20f77da212ea9ae, 0x457243c20b88b1a3,
    0xa55c446f1577ac69, 0x9fe1dcbff0e587f2, 0x5f5c5faab0bbf5d5, 0x3e9dac4ed951b2c4, 0x2876a62cbcfb45b6, 0x2221699aa48b7e2c, 0x4ff89a1b7af8597d, 0xf252338420340b01,
    0xe246d16ea6a5643d, 0x930885e4ec8ff23d, 0xb8a85dacdb63b029, 0x90eea1b94709f1d2, 0xfdcb5f70e1a004a1, 0x6ea17b7c88a22ec7, 0xa9d3a47dd61c7683, 0xc426edc9d924f0b6,
    0xb1bfc70971d0c4d2, 0x707660064f16427b, 0xd6f523e984d05372, 0x1d777ab54c994f28, 0x6a0764150d09c9c, 0x8e751407a2fced38, 0x4a39dade6f9f932b, 0xc71495c381485567,
    0x632373c951a8df5f, 0x297f15cca89b9789, 0x35a1d34260adea95, 0xbbab3b904b86ded4, 0x8acca025da45a452, 0x145cd3a4306dde70, 0x2822a51846363b70, 0x7249b8bf26697211,
    0x8044183cae9e5e22, 0x68254e7c6b5baf10, 0xea2f2313c9482508, 0xd4d3277593cf2f20, 0x39dcdf97f9270a9, 0xfa33749892bee327, 0xbaa016b69f2e6378, 0x7fee6eab705bda,
    0xa66559e53f9cfcfe, 0xc382acce661e25a8, 0xab773355d9ef86dd, 0xa2bcc962624fc438, 0xb88053f278cb3eca, 0x748e13cf3f72b6ca, 0xa7aae8c684b2fdd1, 0x574462a90bbcc1f5,
    0xa7e7289621e6148c, 0x407e1f2ccbfdb0cf, 0x7b4a04bdec90ad39, 0xea34cc91df5da292, 0x35bc61c2ea9af764, 0x48d5fc868a958933, 0x96fee5a312f2ca1a, 0x83c62aa0527275,
    0x257f8adeb2d408b6, 0x957fa4d6d70f205c, 0xf83f55a706ffb460, 0xe5c3decc8cd83224, 0x618ad36093fcd581, 0x210f9d6dd5a75c45, 0xfc586ad659192151, 0xbb0fe00e573cb46c,
    0xb6cb8f5e52b0175f, 0x6afa25982a6fe5ac, 0x17dbeb7f6034b1b8, 0xf861035436213d0b, 0x46b68a76917b748c, 0x663d3d8e7328b73a, 0x138e3b198143564b, 0xa3382fe8cd5ee0f0,
    0x8c28f809fd1c81ed, 0x52a588cb9fe12639, 0x260a3236800386fd, 0x38e6e7042403db5e, 0x8bfad06f229dcad8, 0xa91a8c214817e50e, 0x1fcbd86938dea017, 0x91557a77b0022cfb,
    0x9df05b009c5bc24, 0xdf20aab7a88adb1f, 0x5c754e6c02e3c23e, 0x3655c91747cb9a1c, 0xdc284e3af8133fac, 0x72fbe660aff88d3c, 0xc968d67a0aace334, 0xcf8786c8c24be295,
    0x59f6acc3592486c0, 0x756767cd143d042b, 0x3ff36d7711d7e5eb, 0xd7047934ec5de5ff, 0x4e1cd43113efb9a8, 0x589f3ed46f627d03, 0xccbc9be6d132ed7b, 0x75e39cbd50c87d78,
    0x3e2f8e2e55aacfd1, 0x65be7726b459d927, 0x4ab79c9368ce6bef, 0x8a9d5c4b5e34fbc1, 0x70bbbe491ce518b6, 0xee3afa3f1389dd96, 0x1f5e19c2eb0d9b6b, 0xa98647f505285ceb,
    0x49c14147e60693cb, 0xfb4a572600815767, 0x19d702b2ad5543e9, 0xac85dcd62c7224b7, 0x430f0ba27a6c3a1f, 0xfee270615ecf51de, 0x826dad475c7c8b37, 0x896461b49df3e33f,
    0x787ea865e3638465, 0x8b06d7e9deb49656, 0x1900d0dfbcb2b15b, 0xafcefe2cabc3aae, 0xa42145ba17911d62, 0x208096ac489e2cfe, 0xfce4b0673b3c1084, 0x45c6f3e732224729,
    0xc37edc32190d4c2d, 0xc73170bf3f0f3785, 0x5f804e5cebaab8f0, 0x8e69cbed0210a471, 0x56c61f72c7f0f90, 0x83cac30fd0011e8c, 0xa3ed2738130da661, 0xe47ba8e89e018f3b,
    0x7c06447feb2b4c87, 0xc4db79044420acda, 0x4a831b74f5fabb08, 0x489c78a0a4e22168, 0x5cb485e1bf36fe14, 0xefeeabcfd3681802, 0x4dee62b77208f89b, 0xfbdc386719712727,
    0x1521e54e889213b, 0x3190b95fc7fd157c, 0xfebbbca26a344ea8, 0x297a5db0bde550d2, 0xcb74364e7b9e2f37, 0xd89d173ab2b5196f, 0x527538b80a85c2, 0x25a8b8bd6f175b34,
    0x37b33e83da08a699, 0x68c9e519bad7bfc2, 0xa2853f529c9a220, 0x35cd30347b39af8d, 0x784d88f28fa3e1b2, 0xe8d56989970ee9f2, 0x39ca977c4d2b2075, 0x8592a83070b04ad2,
    0x7fb3b1e3ab4b8810, 0x5ce069c3e5372841, 0x52ce7dcfea30346d, 0x43524c9b2a003ac8, 0x1dd2f84208cf36a8, 0x1178ea36c67fa0f3, 0x88f115b6d4012e11, 0x5b5b85b214905abc,
    0x9c0e7cc4e75bcfe4, 0xf480cdaafbcf4a50, 0x210657898f9ae96f, 0xdfcc3b4a193e9de2, 0x79565b87effc3834, 0x1d75dfdb0b95f51e, 0xc39fc89c0e7524ce, 0xc56637c1348218f3,
    0x4dbf4c0e277a69bd, 0x80db85948efa4d47, 0x9e509862f5c54cae, 0x745321102598ff4d, 0x17a6f84ee677d437, 0xe9fc219089070793, 0x10a93ff8b43cd3b6, 0xfe968fb997a570fa,
    0xcba9b4f2f4c3a78c, 0x4b4ca759f1c54532, 0x74434fef9151aa5, 0xc31381f7c6aa90b2, 0x99aa167b6eb3cdce, 0x63b5e5cccd11ef26, 0x61a7dbaae121c63a, 0xbce9b31c4e3d45c3,
    0x9f79173841daa2ca, 0x6ea09a876a4ffc5, 0x467baad308fea8d5, 0xeeadcb5e6fc04989, 0x6b8881377ad66004, 0xe306ab69ea02895d, 0x94caf67c9e101cf9, 0xa5ca672c777c5249,
    0x1e0b87f0b846580a, 0x47cc5f6d8d1b2acc, 0xe345626f0e480269, 0x6419efd8ab763139, 0x2ed0c1195302ee85, 0xc2d6c589d0fbf5af, 0x477e80a5e9556777, 0x6a34179c8743d274,
    0x5b0f0fd8207d6441, 0xbb6a64eea5b770a8, 0xdeac9578561e6daf, 0x17bb042c4d361626, 0xaab283d61811cf38, 0x1be6d5302394b099, 0x5493699af579c5d8, 0x4665e4f9e3274651,
    0x33c9c322cb9a307b, 0x96236d0e61c62fd6, 0x35f80db4e58bb5f7, 0x59d99a21e070ad43, 0x13415fc264559213, 0x1084f82b6a248fb6, 0x1969dee310201be1, 0x4e71a5053c6b2369,
    0xad07d08460b5f3ba, 0x850b33aa5c98a9c4, 0x16abb8f66fe48100, 0x9f8d1adb1a491c54, 0x7ee455cfd5e6effd, 0xb15b6dbec3b42a64, 0xab3d7399bc6dbd6a, 0xc754c73729b21d79,
    0xae88028a0d345d85, 0xe8d58a6237d1f204, 0xa26df59088d60a1d, 0xf34eaae9152da514, 0x392f548877cf8c95, 0x450c2c9605003c53, 0xc18bd23d57afa5f9, 0xf3af59dd261e0270,
    0x7d99295fa2b3dd07, 0x34030a2bd51b68ea, 0x440f98098a56e323, 0x452a0a8c4eafab9, 0x26e78908490cdc1f, 0x2b2deb415fd39467, 0x446f7dd252066eb0, 0x23a7e3fec74623ce,
    0x5055b14b5d9e3adf, 0xe251341d9cb0defe, 0x2cb726f7abbfd121, 0x261b5b56c2f5bf21, 0x363b15ca0e3581d7, 0xba064fb56286e960, 0x851e8151666c92c5, 0xf5587b500e844ba8,
    0xa4e680de2f1bda89, 0x66313210794d38b0, 0x35f7c56e06282c44, 0xb285feed4bd31bd8, 0xb790055f9adc928d, 0xbdbf1c74f24d9a16, 0x60f8b6a3e0bf161a, 0x7514d019e0377b4,
    0xc815cd63b19d7fa, 0x91fb3a11b7ebe08d, 0x491d67ad042652b5, 0x641999046ff51b5, 0x273a424d5bfe33f4, 0x73639bec5eb7741a, 0xba3c0a74186afbb2, 0x7199dc51c42cb6a8,
    0xde7f48ad55b3f24b, 0xbd8665a8977ff30, 0x8d82cdbc38892c29, 0x8de44b7f6fb537e7, 0x30877db798e1615a, 0x865dae167fa128a6, 0x91f863b926414cb9, 0x473926ed3ca18256,
    0x153e9e6dc90fbf70, 0x852ea789a2262e0b, 0xed7be05bfdf4fbbd, 0x66188dbcdd610ee3, 0xbaa66bcbbe0c3ca1, 0xd7e2a60a0d8500de, 0x94c3aa457a59b209, 0x10b7b684f93d09c8,
    0x6552bdaf7d6ed5c4, 0x9d01c96bfc667e23, 0x7a50d7e32c0ecefb, 0x7c04c3987f5e1fdd, 0x9dbaf4881e21e2ef, 0x302cc9c060852f4b, 0x2f0f861cef4cd3f2, 0xf65211ab371214b4,
    0x3d8527bd26e725ed, 0xaec8e593e3f1d321, 0x31dbfd937a40db15, 0x96b0b274744d094, 0xf2c1cbc5f768535c, 0xefcc741ad62867de, 0xadd48f47a19aa43e, 0xeb0f2eda656388f,
    0x37c6e3b1596ce6ab, 0x3134129fb859e396, 0xf94446c4d3450af9, 0xbaaf2d00165189a9, 0x4533dd31f03839af, 0xac336c06134d308f, 0xc81ca99a4dd05702, 0x6a15af6290ed6d2d,
    0x25a90435a02e7a41, 0x31d597f394bafd8e, 0xb01e018db907b1e, 0x1ce7527d19c673b1, 0x885b8baa612303af, 0xbce8809402dcbe87, 0xc98a726cc3f13b7a, 0x64613b6545c0052c,
    0xaab7086afb9605a4, 0xf7a49ea13865fe6e, 0x13a2ac62d509a023, 0x5319ef316f950ad5, 0x5caa3901f291ba22, 0x2f19d71407167048, 0x422a6070954078fe, 0xe62b75b6e9fae9f5,
    0xc256021a453f3443, 0x2f6e95109c7125fd, 0xd597e0da47829d81, 0x4474ccb46fa22d2, 0x8b46d24fb96efc1e, 0x3a8790c506fc5aa9, 0xdfa3471a558ad03d, 0x2bfff6fabfa86f81,
    0xdc9e023fc1c87e50, 0x182e75fe2ed833d6, 0x6cc1f9ebe8ff65ae, 0xf8b5b11a306e77d9, 0xe54101f02af02386, 0xa7f246153435b78d, 0x80e4d0569fe2bef4, 0xae9d87b1f0f6a6b0,
    0xc07ffe1c948fee8a, 0x78b963534c13afa1, 0xc36a9cc116dac64b, 0xef0c01fbafdfbb40, 0x1240775c87bee689, 0xaf1465a438a8d304, 0x94c4446090c16844, 0x3260efe99651c10f,
    0xd84c9479109b101d, 0xf915ac9b2e7e8772, 0xa2d07e6d91c4f180, 0x5137a9b223ddf9f1, 0x86877ac1dd676d5d, 0xeb617bb3140b9076, 0x79c287da333d6b8e, 0x31ad2af1c59d0987,
    0x58f4d1884e6b1057, 0x8ebabfc175eb3448, 0xfba4733b42dc7b29, 0x3f2183f739943376, 0xba19d16787330f54, 0xe401a04ff01435de, 0xeb03372b9533ae13, 0x837bf7eb8aeb3a70,
    0xa3d81ea9b3be6ee7, 0x81bb61480d2d270d, 0xe760d934d3c32c31, 0xb6b008a35126045a, 0x84eb96ac3fa73a42, 0xcf936c2eeb69db57, 0xcec5b914cfcc62e0, 0x97c065e45a36bd4e,
    0xf653d072946f169b, 0xa062422928664aae, 0x71cff2458d69c762, 0x6cd6ee8a2e9b52b, 0xced726f006263288, 0x5d8898b47d94e64b, 0x770b8f4e0921e5ac, 0x74665e19330ad575,
    0x805ef7864acfd32c, 0x9eade934d8861f7b, 0xe15ab1d88c9fac61, 0xbbacc1788caf89fc, 0xa5839a6d428183e6, 0xcfad292a7df9a417, 0xe62434c4debd5711, 0xca52a0bc8354963e,
    0x990df852ed07061f, 0xfa14515cf5932abc, 0x8429640b9f69cab8, 0x8346eeb6cfd0e445, 0xc7b9a7979a62f415, 0xb76da8ad1cdf32b8, 0x45e0216932199158, 0x19099b38036ec839,
    0xc23db1f2da25426, 0xe698ed15141a7f72, 0xbfedec016095336, 0xddf43fda44b1306c, 0xd0857594fac694b2, 0xf856455602718595, 0x6aa043f147bbf5f4, 0x3978f705c9d5ab08,
    0xf5b936f08069243e, 0xa3175432f181ff59, 0xdf75b38f93f7698b, 0x6d23862550fd8a30, 0xd6f1f4cca9bfb45c, 0x2102e2dc11cae034, 0xa1a5e4e0ea617981, 0x7369c9e0fbc6a288,
    0x5efa6454b1ed098b, 0xc1cde722f0a7c0c7, 0x6dceaf9344dfdfd1, 0x5879bd687c504a44, 0x5912480895808d64, 0x1555eea95dc3a531, 0x8ade992c9647599d, 0x3fde06614e6448e7,
    0xfdeacf624c2b738a, 0x7dd6149940ad9103, 0x417c43131254eb50, 0xf84799b4401a6a8a, 0xed8d0c68b0b3a88a, 0x9e7ba564229f9c32, 0xfc01919758bbdc5c, 0x57fab2a1d24f8de6,
    0xa70666b999539197, 0x7cd29aea5cc096f5, 0x7bb15d4aa2f0b234, 0x6a2d40ce88843125, 0xbd3e83a65c9ae7a, 0xc7f81842ac8e1913, 0x56920a023e95a9b8, 0xe140a756fb98f708,
    0x14bb6f13fb16950e, 0x4257bed4d8360626, 0x62b285d0a0f47c13, 0x9cd495eb81871fc0, 0xac2e8672fcd0cc69, 0x8ade502ac1beab75, 0x72fc3b4afccb4da8, 0x2bd40d66d8036ce2,
    0xdacfc0373d9c07a3, 0xf9da338598aa30c2, 0x8638a107ed58ece, 0xff5afe3834447ad7, 0xb14e001a03c6a3ad, 0xc048378ee59c5dfb, 0xf1b60f897d7b5960, 0xa96abb97293c1eaf,
    0x7c54aaab4b11e28b, 0x61cc041604bd60f8, 0xea82f6fb23c2a4bb, 0x4a4bd35fce624ee1, 0xebd84e43615e78ca, 0x273201370b80ec6c, 0xd078516f5321a2e4, 0xd16f9f3f3639d842,
    0xeadca79e6503bccc, 0x64e8dc0c72bbf6d0, 0x7ddf4bcee6a2012, 0x3ecc83aa524d548, 0x654f4242d8eb09dd, 0x5f04b3803f81d7a2, 0xbcb0c33eabe4e5a2, 0xa88959610a5d8a4b,
    0xcd2291acb142ff05, 0xfffa4d464f919d40, 0x506ac4883c54649a, 0xda50ec9f5d77c9dd, 0x57d394fb4ebdc63d, 0x126f9f773d4383dd, 0xe73e2bf21e23c5e5, 0x406538a0a53397b1,
    0x9e49ccffe109a80c, 0xd9bef2aa12da6572, 0x1f5b54732fe32560, 0x14323edc125715da, 0x3945316d8c9575f3, 0x883800cff47c260b, 0xda650486072a854e, 0xfbc6a53ace4614a5,
    0xfb2f1e05de8d276b, 0x86da48192cfdc471, 0xafc0043fed7240fa, 0x9982af099b28e354, 0x87a4cbe59d39f142, 0x6c3bb96ebe8250e9, 0x3ff7501f3e99b43b, 0x46dc218232716593,
    0x3a5618b37570d861, 0x807b5c2def31ed23, 0xc8a13a010dbdf10b, 0xb9ecbca2551ad6bb, 0xa719f21b898c42f4, 0xa62ad09ea76d7ce3, 0xeaba8d3c54dcb66d, 0x2f24c00b383f6640,
    0x6a66d737b4d56aa2, 0x99a308eda7eeb9c2, 0x7c622f3cd45b1c47, 0x7bdb300cb37815f1, 0x1eac2fa611c905fe, 0x21b55c0d8926599e, 0x4125f8154afb265b, 0xb025280ad6b7cb56,
    0x1c63e57a98914687, 0xa38d5e227cb51e34, 0xd128be2c08402d94, 0x24b34d6e3c9e7312, 0xc4d4ab30fca9dc2b, 0x4decacc886b2cb24, 0xb4e06388ca121290, 0x6252f2e7dae0aa0b,
    0xbceb3227c1559db3, 0x788e2d70a82d12eb, 0xb9d0d9585c05a812, 0x7202485c5e1d9c8b, 0x10ae93c179f9633f, 0xa2cc0bce68b0a080, 0x288a6c174e7f586e, 0xaeaf247a40e4dbfe,
    0x77b60412f6ed732c, 0x59fccabd05d07baf, 0x349797c67c00e163, 0x333bee8a09699f03, 0xa65138001aea002a, 0x838ba5eb93064df3, 0x2de413d4a274794c, 0xac992fce44ba9be3,
    0xfcaff59141b603c6, 0x5bd1ad25057d82d, 0x28bcf5b918b5d574, 0x570f2423315897ac, 0x3859d078f2cafe76, 0x71a3e6d6bee5c4c8, 0xdb90c1acdb9eddb8, 0xb50d0077df108b32,
    0x2fc27407af1e070c, 0xcce5632815720cfc, 0xe6d00d54e846336e, 0x1c4b26c6af06fe7d, 0x544fb3835b4e0a, 0x837ce40a8bb387ee, 0x6aafa7f6f399925e, 0x50d4341b8aca7994,
    0xd06b3c318b07707b, 0xa1db64e32cba28ff, 0x893b6c66c8e54cb9, 0xa2180b5fb02f55d1, 0x5f2551a38ab03de8, 0xb2d5ce0a026d11fc, 0xc2be9e97beb99fec, 0x8717bc0c2dae6d63,
    0x5cc2197586a968ed, 0xd4c7197a9d2c6110, 0x4f010085916304d4, 0xdc03a482a13010a3, 0x6b5f91595749f4bc, 0xf16e16aeb97c2651, 0x67b51c4533c9a0e9, 0xf18a40235920fdf0,
    0x18050451571ee275, 0xabdec8f7f26e3efd, 0x853fd0d94c3b75f3, 0xacfb418077284520, 0x25767a414e70e5c9, 0x84afeaf02bcd1e3c, 0xaaf1cfe660d5ddd5, 0x99f386dc994f9132,
    0xe2bd47a547c8693e, 0x29606e8ff0f9b2ef, 0x3a4e7a517badc292, 0xa25fc2edab997694, 0x6e8df604b80ecc24, 0x6f549e6d31392f7, 0x5c9776ed9914d24e, 0x5f2035f8a3b708fe,
    0x320686ca3d360a69, 0x9c3b42ebd287e392, 0xba703f870a4839f7, 0x23e4549f66caf1ca, 0xb7cf92b1a23e545e, 0x2ecccdaa3f78e2f6, 0xe79da7c8c7f51e5d, 0xe556027e8dbaaf94,
    0x5f20df0eb6ce9dae, 0x3c5fe200d082968b, 0x8d36972e7a73b4f1, 0xc002fe58366977bc, 0xaab8b980b9c4e99e, 0x2f0df2e7e729e1f9, 0x559fe978b001f7e5, 0x2c170b9631eee669,
    0x70892e657652c082, 0xf05c8f814cb89a4e, 0xf802789c29cfa988, 0x75ea452dbbe30326, 0xe5fda80465fd9162, 0x5000828ea6d003cc, 0x77c7df27a7b5cdd1, 0x83895b130a0435ab,
    0xcd5d7971c27a9470, 0xd14084ae5e7d3bf2, 0x5798c0ee27e85896, 0xef90891cc135400f, 0xe119fa645c69f075, 0xf75073b3ec410c1, 0xaa8620a2a78294a, 0xf18762b76f8de214,
    0xbc53170b74d6bd02, 0x35da63302357a903, 0x92b902d7e7cb8eff,
};

};
