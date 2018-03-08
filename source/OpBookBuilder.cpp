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

#include "OpBookBuilder.h"
#include "GameReader.h"

using namespace opening;

void OpBookBuilder::create(std::map<std::string, std::string> paramMap) {
    auto it = paramMap.find("folder");
    if (it != paramMap.end()) {
        std::vector<std::string> folderVec;
        folderVec.push_back(it->second);
        create(folderVec, paramMap);
    }

    it = paramMap.find("file");
    if (it != paramMap.end()) {
        create(it->second, paramMap);
    }

    std::string bookPath;
    it = paramMap.find("out");
    if (it == paramMap.end() || it->second.empty()) {
        bookPath = "./openingbook.xob";
    } else {
        bookPath = it->second;
    }

    if (header.size[0] + header.size[1] > 0) {
        createSave(bookPath, paramMap);
    } else if (openingVerbose) {
        std::cerr << "Error: book is empty" << std::endl;
    }
}

void OpBookBuilder::create(const std::vector<std::string>& folderVec, const std::map<std::string, std::string>& paramMap)
{
    for(auto && folder : folderVec) {
        std::vector<std::string> pathVec = listdir(folder);
        for(auto && path : pathVec) {
            create(path, paramMap);
        }
    }
}

void OpBookBuilder::create(const std::string& inputPath, const std::map<std::string, std::string>& paramMap)
{
    if (openingVerbose) {
        std::cout << "\tgame: " << inputPath << std::endl;
    }

    OpeningBoard board;
    GameReader gameReader(inputPath);

    int maxply = Para_DefaultMaxPly;
    auto it = paramMap.find("maxply");
    if (it != paramMap.end()) {
        auto str = it->second;
        int k = atoi(str.c_str());
        if (k > 0) {
            maxply = k;
        }
    }

    int minply = Para_DefaultMinPly;
    it = paramMap.find("minply");
    if (it != paramMap.end()) {
        auto str = it->second;
        int k = atoi(str.c_str());
        if (k > 0) {
            minply = k;
        }
    }

    // Side
    const int White = 1;
    const int Black = 1 << 1;

    int forSide = White | Black;

    if (paramMap.find("-only-white") != paramMap.end()) {
        forSide = White;
    } else if (paramMap.find("-only-black") != paramMap.end()) {
        forSide = Black;
    }

    if (forSide & White) {
        createInit(Side::white);
    }
    if (forSide & Black) {
        createInit(Side::black);
    }

    // Add games
    while(gameReader.nextGame(board)) {
        if (board.getHistList().size() < minply) {
            if (openingVerbose) {
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << " - game too short" << std::endl;
            }
            continue;
        }

        Side workingSide = Side::none;

        if (board.getResult() == Result::win) {
            if (forSide & White) {
                workingSide = Side::white;
            }
        } else if (board.getResult() == Result::loss) {
            if (forSide & Black) {
                workingSide = Side::black;
            }
        }

        if (workingSide == Side::none) {
            if (openingVerbose) {
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << " - result is not suitable" << std::endl;
            }
            continue;
        }

        std::vector<Move> moves;
        for(auto && hist : board.getHistList()) {
            moves.push_back(hist.move);
        }

        while (!board.getHistList().empty()) {
            board.takeBack();
        }

        if (board.key() != originHashKey) {
            if (openingVerbose) {
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << " - not from the origin" << std::endl;
            }
            continue;
        }

        create_checkFlipping(board, moves, workingSide);

        if (board.side != workingSide) {
            create_add(board);
        }

        for(int ply = 0; ply < moves.size() && ply < maxply; ply++) {
            auto move = moves.at(ply);
            board.make(move);
            if (board.side != workingSide) {
                create_add(board);
            }
        }
    }
}

void OpBookBuilder::create_checkFlipping(OpeningBoard& board, std::vector<Move>& moves, Side workingSide)
{
    bool needHorizontalFlip = false;
    Hist hist;
    hist.hashKey = 0;
    if (board.side == workingSide) {
        board.make(moves.front(), hist);
    }

    auto sd = static_cast<int>(workingSide);
    auto idx = find(board.key(), sd);

    if (idx < 0) {
        board.flip(opening::FlipMode::horizontal);
        needHorizontalFlip = find(board.key(), sd) >= 0;
        board.flip(opening::FlipMode::horizontal);
    }

    if (hist.hashKey != 0) {
        board.takeBack(hist);
    }

    if (needHorizontalFlip) {
        board.flip(opening::FlipMode::horizontal);
        for(auto && move : moves) {
            move.from = OpeningBoard::flip(move.from, opening::FlipMode::horizontal);
            move.dest = OpeningBoard::flip(move.dest, opening::FlipMode::horizontal);
        }

        assert(!board.getPiece(moves.front().from).isEmpty());
    }
}

void OpBookBuilder::createInit(Side side) {
    int sd = static_cast<int>(side);

    if (bookData[sd]) {
        return;
    }

    header.reset();

    allocatedSizes[sd] = CreatingAdditionalItemNumber;
    bookData[sd] = (BookItem*)malloc(allocatedSizes[sd] * sizeof(BookItem) + 32);
}

bool OpBookBuilder::create_add(const OpeningBoard& board)
{
    auto key = board.key();
    int sd = 1 - static_cast<int>(board.side);

    if (header.size[sd] == 0) {
        bookData[sd][0].incValue(key);
        header.size[sd]++;
        return true;
    }

    i64 idx = 0;
    i64 i = 0, j = header.size[sd] - 1;

    while (i <= j) {
        idx = (i + j) / 2;
        if (key == bookData[sd][idx].key()) {
            bookData[sd][idx].incValue(key);
            return true;
        }
        if (key < bookData[sd][idx].key()) j = idx - 1;
        else i = idx + 1;
    }

    if (key >= bookData[sd][idx].key()) {
        idx++;
    }

    if (header.size[sd] + 1 >= allocatedSizes[sd]) {
        allocatedSizes[sd] += CreatingAdditionalItemNumber;
        auto tmpBuf = bookData[sd];
        bookData[sd] = (BookItem*)malloc(allocatedSizes[sd] * sizeof(BookItem) + 32);

        i64 i = 0;
        for(; i < idx; i++) {
            bookData[sd][i] = tmpBuf[i];
        }
        for(; i < header.size[sd]; i++) {
            bookData[sd][i + 1] = tmpBuf[i];
        }
        free(tmpBuf);
    } else {
        for(i64 i = header.size[sd]; i > idx; i--) {
            bookData[sd][i] = bookData[sd][i - 1];
        }
    }

    header.size[sd]++;
    bookData[sd][idx].incValue(key);

    return true;
}

bool OpBookBuilder::createSave(const std::string& path_, const std::map<std::string, std::string>& paramMap) {
    path = path_;

    std::ofstream outfile (path_, std::ios::binary);

    assert(header.isValid());
    assert(header.size[0] + header.size[1] > 0);

    i64 startSize[2] = { header.size[0], header.size[1] };

    bool ok = true;

    // Header
    auto it = paramMap.find("info");
    if (it != paramMap.end()) {
        auto str = it->second;
        header.setNote(str.c_str());
    }

    if (!header.saveFile(outfile)) {
        ok = false;
    } else {
        int maxgame = Para_DefaultMinGame;
        auto it = paramMap.find("mingame");
        if (it != paramMap.end()) {
            auto str = it->second;
            int k = atoi(str.c_str());
            if (k >= 0) {
                maxgame = k;
            }
        }

        if (maxgame <= 0) {
            for(int sd = 0; sd < 2 && ok; sd++) {
                if (header.size[sd] > 0) {
                    if (!outfile.write((const char*)bookData, header.size[sd] * sizeof(BookItem))) {
                        ok = false;
                        break;
                    }
                }
            }
        } else {
            int n = 2 * 1024;
            BookItem* tmpBuf = (BookItem*)malloc(n * sizeof(BookItem) + 16);

            for(int sd = 0; sd < 2 && ok; sd++) {
                i64 itemCnt = 0;
                for(int i = 0; i < header.size[sd]; i += n) {
                    auto *p = tmpBuf;
                    for(int j = 0; j < n && (i + j) < header.size[sd]; j++) {
                        auto q = bookData[sd] + i + j;
                        if (q->value >= maxgame) {
                            memcpy(p, q, sizeof(BookItem));
                            p++;
                            itemCnt++;
                        }
                    }

                    assert(p - tmpBuf <= n);
                    if (p > tmpBuf && !outfile.write((const char*)tmpBuf, (p - tmpBuf) * sizeof(BookItem))) {
                        ok = false;
                        break;
                    }
                }

                header.size[sd] = itemCnt;
            }

            free(tmpBuf);

            if (ok) {
                header.saveFile(outfile);
            }
        }
    }

    outfile.close();

    if (openingVerbose) {
        if (ok) {
            std::cout << "Book has been created, #items: " << header.size[0] << ", " << header.size[1] << ", starting: " << startSize[0] << ", " << startSize[1] << std::endl;
        } else {
            std::cerr << "Error: Cannot write book data." << std::endl;
        }
    }

    return ok;
}

