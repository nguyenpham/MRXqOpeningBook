//
//  OpBookBuilder.cpp
//  Opening
//
//  Created by Tony Pham on 7/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

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

    it = paramMap.find("out");
    if (it == paramMap.end() || it->second.empty()) {
        return;
    }

    if (header.size[0] + header.size[1] > 0) {
        createSave(it->second, paramMap);
    } else if (openingVerbose) {
        std::cerr << "Error: book is empty" << std::endl;
    }
}

void OpBookBuilder::create(const std::vector<std::string>& folderVec, const std::map<std::string, std::string>& paramMap) {

    for(auto && folder : folderVec) {
        std::vector<std::string> pathVec = listdir(folder);
        for(auto && path : pathVec) {
            create(path, paramMap);
        }
    }
}

void OpBookBuilder::create(const std::string& inputPath, const std::map<std::string, std::string>& paramMap) {
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
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << ". Game too short" << std::endl;
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
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << ". Result is not right" << std::endl;
            }
            continue;
        }

        std::vector<Move> moves;
        for(auto && hist : board.getHistList()) {
//            moveList.add(hist.move);
            moves.push_back(hist.move);
        }

        while (!board.getHistList().empty()) {
            board.takeBack();
        }

        if (board.key() != originHashKey) {
            if (openingVerbose) {
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << ". It is not from the origin" << std::endl;
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
//        for(int i = 0; i < moveList.end; i++) {
//            auto move = moveList.list + i;
//            move->from = OpeningBoard::flip(move->from, opening::FlipMode::horizontal);
//            move->dest = OpeningBoard::flip(move->dest, opening::FlipMode::horizontal);
//        }

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
//        std::cout << "idx = " << idx << ", key = " << key << ", ket at 0 = " << bookData[sd][0].key() << ",  sz = " << header.size[sd] << std::endl;
        for(i64 i = header.size[sd]; i > idx; i--) {
            bookData[sd][i] = bookData[sd][i - 1];
        }
    }

    header.size[sd]++;
    bookData[sd][idx].incValue(key);

//    if (header.size[sd]) {
//        for (i64 i = 0; i < header.size[sd]; i++) {
//            std::cout << i << ", " << bookData[sd][i].key() << ", val = " << bookData[sd][i].value << std::endl;
//        }
//    }
//    assert(verifyData(sd));

    return true;
}

//void OpBookBuilder::create_setupAllChildrenValues() {
//    for(int sd = 0; sd < 2; sd++) {
//        for(int idx = 0; idx < header.size[sd]; idx++) {
//            create_setupChildrenValues(idx, sd);
//        }
//    }
//}
//
//void OpBookBuilder::create_setupChildrenValues(int idx, int sd) {
//    OpeningBoard board;
//    board.pieceList_setupBoard((const int8_t *)creatingBookData[sd][idx].pieceList);
//    board.initHashKey();
//    board.side = static_cast<Side>(sd);
//    create_setupChildrenValues(board);
//}
//
//void OpBookBuilder::create_setupChildrenValues(OpeningBoard& board) {
//    opening::MoveList moveList;
//    board.gen(moveList, board.side, false);
//    int sd = static_cast<int>(board.side);
//
//    std::vector<std::pair<int, u32>> idxhitVec;
//
//    Hist hist;
//    for(int i = 0; i < moveList.end; i++) {
//        auto move = moveList.list[i];
//        board.make(move, hist);
//
//        auto key = board.key();
//
//        i64 newIdx = find(key, (const char*)creatingBookData[sd], header.size[sd], sizeof(CreatingBookItem));
//        if (newIdx >= 0) {
//            std::pair<u64, u32> idxhit;
//            idxhit.first = newIdx;
//            idxhit.second = creatingBookData[sd][newIdx].hit();
//            idxhitVec.push_back(idxhit);
//        }
//        board.takeBack(hist);
//    }
//
//    if (idxhitVec.empty()) {
//        return;
//    }
//
//    std::sort(idxhitVec.begin(), idxhitVec.end(), [](const std::pair<int, u32> & a, const std::pair<int, u32> & b) -> bool {
//        return a.second < b.second;
//    });
//
//    const u16 IncValue = 10;
//    for(int i = 0; i < idxhitVec.size(); i++) {
//        auto idx = idxhitVec[i].first; assert(idx >= 0 && idx < header.size[sd]);
//        creatingBookData[sd][idx].value = std::max(creatingBookData[sd][idx].value, (u16)((i + 1) * IncValue));
//    }
//}

bool OpBookBuilder::createSave(const std::string& path_, const std::map<std::string, std::string>& paramMap) {
    path = path_;

    std::ofstream outfile (path_, std::ios::binary);
    //    outfile.seekp(0);

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
            std::cerr << "Error: Cannot write data" << std::endl;
        }
    }

    return ok;
}

