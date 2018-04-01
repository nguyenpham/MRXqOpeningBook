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

void dumbReportString(std::string) {}
void dumbReportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt) {}

void OpBookBuilder::create(std::map<std::string, std::string> paramMap,
                           std::function<void(std::string)> reportString,
                           std::function<void(int, int, int, int)> reportNumbers //(fileCnt, gameCnt, nodeCnt, addedNodeCnt)
                           ) {
    m_reportFileCnt = m_reportCnt = m_reportNodeCnt = 0;

    auto it = paramMap.find("folder");
    if (it != paramMap.end()) {
        std::vector<std::string> folderVec;
        folderVec.push_back(it->second);
        create(folderVec, paramMap, reportString, reportNumbers);
    }

    it = paramMap.find("file");
    if (it != paramMap.end()) {
        create(it->second, paramMap, reportString, reportNumbers);
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

void OpBookBuilder::create(const std::vector<std::string>& folderVec, const std::map<std::string, std::string>& paramMap, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers)
{
    for(auto && folder : folderVec) {
        std::vector<std::string> pathVec = listdir(folder);
        for(auto && path : pathVec) {
            create(path, paramMap, reportString, reportNumbers);
        }
    }
}

void OpBookBuilder::create(const std::string& inputPath, const std::map<std::string, std::string>& paramMap, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers)
{
    if (openingVerbose) {
        std::cout << "\tgame: " << inputPath << std::endl;
    }

    reportString(inputPath);

    m_reportFileCnt++;

    GameReader gameReader(inputPath);

    int maxply = Para_DefaultAddToPly;
    auto it = paramMap.find("maxply");
    if (it != paramMap.end()) {
        auto str = it->second;
        int k = atoi(str.c_str());
        if (k > 0) {
            maxply = k;
        }
    }

    int minply = Para_DefaultMinGameLength;
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
    OpeningBoard board;
    while(gameReader.nextGame(board)) {
        m_reportCnt++;

        if (board.getHistList().size() < minply) {
            if (openingVerbose) {
                std::cerr << "\t\tignore game " << inputPath << ", idx: " << gameReader.currentGameIdx() << " - game too short" << std::endl;
            }
            continue;
        }

        Side workingSide = Side::none;

        if (board.getResult().result == ResultType::win) {
            if (forSide & White) {
                workingSide = Side::white;
            }
        } else if (board.getResult().result == ResultType::loss) {
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

        m_keyMap.clear();
        for(int ply = 0; ply < (int)moves.size() && ply < maxply; ply++) {
            auto move = moves.at(ply);
            board.make(move);

            // Any repitition will be terminated
            auto key = board.key();
            if (m_keyMap.find(key) != m_keyMap.end()) {
                break;
            }
            m_keyMap[key] = key;
            if (board.side != workingSide) {
                create_add(board);
            }
        }
    }

    reportNumbers(m_reportFileCnt, m_reportCnt, m_reportNodeCnt, header.size[0] + header.size[1]);
}

void OpBookBuilder::create_checkFlipping(opening::OpeningBoard& board, std::vector<opening::Move>& moves, opening::Side workingSide)
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

void OpBookBuilder::createInit(opening::Side side) {
    int sd = static_cast<int>(side);

    if (bookData[sd]) {
        return;
    }

    header.reset();

    allocatedSizes[sd] = CreatingAdditionalItemNumber;
    bookData[sd] = (BookItem*)malloc(allocatedSizes[sd] * sizeof(BookItem) + 32);
}

bool OpBookBuilder::create_add(const opening::OpeningBoard& board)
{
    auto key = board.key();
    int sd = 1 - static_cast<int>(board.side);

    if (header.size[sd] == 0) {
        bookData[sd][0].incValue(key);
        header.size[sd]++;
        return true;
    }

    m_reportNodeCnt++;

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
                    if (!outfile.write((const char*)bookData[sd], header.size[sd] * sizeof(BookItem))) {
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

/////////////////////////////////////////////////////////////////////
void OpBookBuilder::verify(std::map<std::string, std::string> paramMap,
                           std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers) {
    m_reportFileCnt = m_reportCnt = m_reportNodeCnt = 0;

    auto it = paramMap.find("out");
    if (it == paramMap.end() || it->second.empty()) {
//        reportString("");
        return;
    }

    auto bookPath = it->second;

    OpBook book;

    if (!book.load(bookPath)) {
        reportString("Error: Cannot load the opening book!");
        return;
    }

    for(int sd = 0; sd < 2; sd++) {
        if (!verify(book, sd, reportString, reportNumbers)) {
            reportString("The book is not good!");
            return;
        }
    }

    reportString("Verifying is complited!");
}


bool OpBookBuilder::verify(OpBook& book, int sd, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers)
{
    if (book.getHeader()->size[sd] == 0) {
        return true;
    }


    std::string sideString = sd == 0 ? "black" : "white";

    reportString("Checking data for " + sideString);

    u16 maxVal = 0;
    for (i64 idx = 0, prevKey = 0; idx < book.getHeader()->size[sd]; idx++) {
        auto p = book.getData(sd) + idx;
        if (prevKey >= p->key() || p->value == 0) {
            std::string s = "Error: data is incorrectly sorted";
            std::cerr << s << std::endl;
            reportString(s);
            return false;
        }
        prevKey = p->key();
        p->value = 0;
        maxVal = std::max(maxVal, p->value);
    }

    std::cout << "verifyData, maxVal = " << maxVal << std::endl;

    OpeningBoard board;
    board.setFen("");
    board.show();

    verify(book, board, sd, 0, reportNumbers);

    i64 reachableCnt = 0;
    for (i64 idx = 0; idx < book.getHeader()->size[sd]; idx++) {
        auto p = book.getData(sd) + idx;
        if (p->value > 0) {
            reachableCnt++;
        }
    }

    std::ostringstream stringStream;
    stringStream << "Checked data for " << sideString << ", #reachable nodes: " << reachableCnt << " of " << header.size[sd] << std::endl;
    std::string msg = stringStream.str();
    reportString(msg);

    std::cout << "verifyData, " << msg << std::endl;
    return true;
}

bool OpBookBuilder::verify(OpBook& book, OpeningBoard& board, int sd, int ply, std::function<void(int, int, int, int)> reportNumbers)
{
    auto side = board.side;
    auto sameSide = static_cast<int>(side) == sd;
    if (!sameSide) {
        auto idx = book.find(board.key(), sd);
        if (idx < 0) {
            return false;
        }

        if (book.getData(sd)[idx].value) {
            return true;
        }
//        if (ply > 30) {
//            board.show("after making a move, ply = 0");
//            std::cout << "verifyData, m_reportNodeCnt = " << m_reportNodeCnt << std::endl;
//        }

        m_reportNodeCnt++;
        book.getData(sd)[idx].value = 1;
        assert(book.getData(sd)[idx].value);

        if (m_reportNodeCnt % 1000 == 0) {
            reportNumbers(m_reportFileCnt, m_reportCnt, m_reportNodeCnt, book.getHeader()->size[0] + book.getHeader()->size[1]);
        }
    }

    MoveList moveList;
    board.gen(moveList, side);

    for(int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        board.make(move);
//        if (ply > 10) {
//            std::cout << "verifyData, m_reportNodeCnt = " << m_reportNodeCnt << std::endl;
//            board.show("after making a move, ply = 0");
//        }
        if (!board.isIncheck(side)) {
            if (verify(book, board, sd, ply + 1, reportNumbers)) {
                if (ply == 0) {
                    reportNumbers(m_reportFileCnt, m_reportCnt, m_reportNodeCnt, book.getHeader()->size[0] + book.getHeader()->size[1]);
                }
            }
        }
        board.takeBack();
    }

    return true;
}
