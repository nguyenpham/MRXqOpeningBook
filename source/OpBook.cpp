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

#include "OpBook.h"
#include "OpBoard.h"


using namespace opening;

OpBook::OpBook() :
    path("")
{
    assert(sizeof(BookItem) == 10);
    bookData[0] = bookData[1] = nullptr;
}

OpBook::~OpBook()
{
    if (bookData[0]) {
        for(int i = 0; i < 2; i++) {
            free(bookData[i]);
            bookData[i] = nullptr;
        }
    }
}

///////////////////////////////////////////////////////////////////////
bool OpBook::load(const std::string& path_) {
    assert(path_.size() > 8);
    path = path_;

    std::ifstream file(path, std::ios::binary);

    if (!header.readFile(file)) {
        return false;
    }

    bool ok = true;
    for(int sd = 0; sd < 2; sd++) {
        if (header.size[sd] <= 0) {
            allocatedSizes[sd] = 0;
            continue;
        }

        allocatedSizes[sd] = header.size[sd] + 250;
        bookData[sd] = (BookItem*)malloc(allocatedSizes[sd] * sizeof(BookItem) + 32);

        i64 dataSz = header.size[sd] * sizeof(BookItem);
        if (!file.read((char*)bookData[sd], dataSz)) {
            ok = false;
            break;
        }
    }

    file.close();

    if (!ok && openingVerbose) {
        std::cerr << "Error load" << std::endl;
    }
    return ok;
}

bool OpBook::verifyData() const
{
    if (!header.isValid() || header.size[0] < 0 || header.size[1] < 0 || header.size[0] + header.size[1] <= 0) {
        std::cerr << "Error: verify failed. Header incorrect." << std::endl;
        return false;
    }

    auto startTime = time(NULL);

    for(int sd = 0; sd < 2; sd++) {
        if (!verifyData(sd)) {
            return false;
        }
    }

    if (openingVerbose) {
        auto elapsed_secs = std::max(1, (int)(time(NULL) - startTime));
        std::cout << "Verify successfully, elapsed (s): " << elapsed_secs << std::endl;
    }
    return true;
}

bool OpBook::verifyData(int sd) const
{
    if (header.size[sd] == 0) {
        return true;
    }


    u16 maxVal = 0;
    for (i64 idx = 0, prevKey = 0; idx < header.size[sd]; idx++) {
        auto p = bookData[sd] + idx;
        if (prevKey >= p->key() || p->value == 0) {
            std::cerr << "Error: verify failed. Items incorrect!" << std::endl;
            return false;
        }
        prevKey = p->key();
        maxVal = std::max(maxVal, p->value);
    }

//    std::cout << "verifyData, maxVal = " << maxVal << std::endl;

    OpeningBoard board;
    board.setFen("");
    board.show();

    for (i64 idx = 0; idx < header.size[sd]; idx++) {
        auto p = bookData[sd] + idx;
        p->value = 0;
    }

    verifyData(board, sd);


    i64 reachableCnt = 0;
    for (i64 idx = 0; idx < header.size[sd]; idx++) {
        auto p = bookData[sd] + idx;
        if (p->value > 0) {
            reachableCnt++;
        }
    }

    std::cout << "Verify reachable item for " << (sd == 0 ? "black" : "white") << ", can reach " << reachableCnt << " items of total " << header.size[sd] << std::endl;
    return true;
}

bool OpBook::verifyData(OpeningBoard& board, int sd) const
{
    auto side = board.side;
    auto sameSide = static_cast<int>(side) == sd;
    if (!sameSide) {
        auto idx = find(board.key(), sd);
        if (idx < 0) {
            return false;
        }

        if (bookData[sd][idx].value) {
            return true;
        }

        bookData[sd][idx].value = 1;
    }

    MoveList moveList;
    board.gen(moveList, board.side, false);

    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
        if (!board.isIncheck(side)) {
            verifyData(board, sd);
        }
        board.takeBack();
    }

    return true;
}

bool OpBook::save(std::string path_) {
    if (path_.empty()) {
        path_ = path;
    }

    std::ofstream outfile (path_, std::ios::binary);
    outfile.seekp(0);

    assert(header.isValid());
    assert(header.size[0] + header.size[1] > 0);

    bool ok = true;

    if (!header.saveFile(outfile)) {
        ok = false;
    } else {
        for(int sd = 0; sd < 2 && ok; sd++) {
            if (header.size[sd]) {
                i64 dataSz = header.size[sd] * sizeof(BookItem);
                if (!outfile.write((const char*)bookData[sd], dataSz)) {
                    ok = false;
                    break;
                }
            }
        }
    }

    if (!ok && openingVerbose) {
        std::cerr << "Error: Cannot write opening data" << std::endl;
    }

    outfile.close();
    return ok;
}

i64 OpBook::find(u64 key, int sd) const
{
    return find(key, (const char*)bookData[sd], header.size[sd], sizeof(BookItem));
}

i64 OpBook::find(u64 key, const char* data, i64 itemCount, int itemSize)
{
    i64 i = 0, j = itemCount - 1;

    while (i <= j) {
        i64 idx = (i + j) / 2;
        const char *p = data + idx * itemSize;
        u64 theKey = *(u64 *)p;
        if (key == theKey) {
            return idx;
        }
        if (key < theKey) j = idx - 1;
        else i = idx + 1;
    }

    return -1;
}

Move OpBook::probe(const std::string& fen, MoveList* opMoveList) const
{
    OpeningBoard board;
    board.setFen(fen);
    return probe(board);
}

Move OpBook::probe(const int8_t* pieceList, Side side, MoveList* opMoveList) const
{
    OpeningBoard board;
    board.pieceList_setupBoard(pieceList);
    board.side = side;
    return probe(board, opMoveList);
}

Move OpBook::probe(const MoveList& moveList, MoveList* opMoveList) const
{
    OpeningBoard board;
    board.setFen("");
    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
    }
    return probe(board, opMoveList);
}

Move OpBook::probe(const std::vector<Piece> pieceVec, Side side, MoveList* opMoveList) const
{
    OpeningBoard board;
    board.setup(pieceVec, side);
    return probe(board, opMoveList);
}

Move OpBook::probe(OpeningBoard& board, MoveList* opMoveList) const
{
    auto bestmove = _probe(board, opMoveList);

    if (!bestmove.isValid()) {
        static const opening::FlipMode flips[] = { opening::FlipMode::horizontal, opening::FlipMode::vertical, opening::FlipMode::horizontal };

        OpeningBoard newBoard(board);
        for(int i = 0; i < 3; i++) {
            auto flipMode = flips[i];
            newBoard.flip(flipMode);
            if (i == 2) {
                newBoard.side = getXSide(board.side);
            }
            bestmove = _probe(board, opMoveList);
            if (bestmove.isValid()) {
                return bestmove;
            }
        }
    }

    return bestmove;
}

Move OpBook::_probe(OpeningBoard& board, MoveList* opMoveList) const
{
    auto side = board.side;
    int sd = static_cast<int>(side);

    Move bestmove(-1, -1);
    if (header.size[sd] <= 0) {
        return bestmove;
    }

    MoveList moveList;
    board.gen(moveList, board.side, false);

    if (opMoveList) {
        opMoveList->reset();
    }

    u16 curValue = 0;
    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
        if (!board.isIncheck(side)) {
            auto idx = find(board.key(), sd);
            if (idx >= 0) {
                auto value = bookData[sd][idx].value;
                move.score = value;
                if (opMoveList) {
                    opMoveList->add(move);
                }
                if (value > curValue) {
                    curValue = value;
                    bestmove = move;
                }
            }
        }
        board.takeBack();
    }

    return bestmove;
}

