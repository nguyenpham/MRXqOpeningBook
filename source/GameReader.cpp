//
//  GameReader.cpp
//  Opening
//
//  Created by Tony Pham on 4/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

#include "GameReader.h"

#include <regex>
#include <fstream>
#include <sstream>
#include <assert.h>

#include "OpBoard.h"


using namespace opening;

GameReader::GameReader(const std::string& path)
{
    init(path);
}

std::string GameReader::loadFile(const std::string& fileName) {
    std::ifstream inFile;
    inFile.open(fileName);

    std::stringstream strStream;
    strStream << inFile.rdbuf();
    std::string content = strStream.str();
    return content;
}

bool GameReader::init(const std::string& path) {
    gameStringVector.clear();
    workingGameIdx = 0;

    std::string content = loadFile(path);

    std::string seperator = "[Event";
    if (content.find(seperator, 0) == std::string::npos) {
        seperator = "FORMAT";
        if (content.find("START{", 0) == std::string::npos) {
            return false;
        }
    }

    std::string::size_type prev_pos = 0, pos = 0;
    bool first = true;

    while ((pos = content.find(seperator, pos)) != std::string::npos) {
        if (first) {
            prev_pos = pos++;
            first = false;
            continue;
        }
        std::string substring(content.substr(prev_pos, pos-prev_pos));
        gameStringVector.push_back(substring);
        prev_pos = pos++;
    }

    gameStringVector.push_back(content.substr(prev_pos, pos-prev_pos)); // Last word
    return !gameStringVector.empty();
}

bool GameReader::nextGame(OpeningBoard& board) {
    if (workingGameIdx >= gameStringVector.size()) {
        return false;
    }

    auto gameString = gameStringVector.at(workingGameIdx);
    workingGameIdx++;

    auto theMap = parse(gameString);

    const std::string fen = theMap["FEN"];
    const std::string result = theMap["Result"];
    const std::string moves = theMap["moves"];
    return parse(board, fen, moves, result);
}

std::map<std::string, std::string> GameReader::parse(const std::string& gameString) const {
    if (gameString.find("[Event", 0) != std::string::npos) {
        return pgn_parse(gameString);
    }
    return wxf_parse(gameString);
}

std::map<std::string, std::string> GameReader::pgn_parse(const std::string& gameString) const {
    std::map<std::string, std::string> r;

    // Headers
    auto headerVec = splitString(gameString, "\\[[A-Za-z]+(\\s)+\"(.)+\"(\\s)*\\](\\s)*\n");

    for (auto &&s : headerVec) {
        if (s.length()<5 || s.at(0)!='[') {
            continue;
        }

        auto qPos0 = s.find('"'); assert(qPos0 != std::string::npos);
        auto qPos1 = s.find('"', qPos0 + 1); assert(qPos1 != std::string::npos);

        auto tag = s.substr(1, qPos0 - 2);
        trim(tag);

        auto quote = s.substr(qPos0+1, qPos1 - qPos0 - 1);
        r[tag] = quote;
    }

    // Body
    if (!headerVec.empty()) {
        // Takeout all comments
        auto lastHeaderString = headerVec.back();
        auto pos = gameString.find(lastHeaderString);
        if (pos != std::string::npos) {
            std::string body = gameString.substr(pos + lastHeaderString.length());
            //std::cout << "body:==>" << body << "<==" << std::endl;

            auto comVec = splitString(body, "(\\{(.|\r|\n)*?\\})|;.*");
            for (auto &&com : comVec) {
                removeSubstrs(body, com);
            }

            // trim out (())
            while (true) {
                auto start = body.find('(');
                if (start == std::string::npos) {
                    break;
                }
                int open = 1;
                bool trimmed = false;
                for (auto p=start+1; p<body.length(); p++) {
                    char ch = body.at(p);
                    if (ch=='(') {
                        open++;
                        continue;
                    }
                    if (ch==')') {
                        open--;
                        if (open==0) {
                            body = body.substr(0, start-1) + body.substr(p+1);
                            trimmed = true;
                            break;
                        }
                    }
                }

                if (!trimmed) {
                    body = body.substr(0, start-1);
                    break;
                }
            }

            std::string special = "+";
            removeSubstrs(body, special);
            special = "x";
            removeSubstrs(body, special);

            r["moves"] = body;
        }
    }

    if (!r.empty()) {
        r["type"] = "pgn";
    }
    return r;
}

bool GameReader::parse(OpeningBoard& board, const std::string& fen, const std::string& moves, const std::string& result) {
    board.getHistList().clear();
    board.setFen(fen);
    if (!board.isValid()) {
        return false;
    }

    board.setResult(result);

    std::regex re("\\s+");
    std::sregex_token_iterator first {moves.begin(), moves.end(), re, -1}, last;
    std::vector<std::string> words = {first, last};

    std::vector<std::string> moveVec;
    for (auto &&s : words) {
        if (!s.empty() && isalpha(s.at(0))) {
            moveVec.push_back(s);

            int i = 0;
            char ch = s.at(i);

            PieceType pieceType = PieceType::pawn;
            if (ch>='A' && ch <='Z') {
                i++;
                ch += 'a' - 'A';
                const char* p = strchr(pieceTypeName, ch);
                if (p != NULL) {
                    int k = (int)(p - pieceTypeName);
                    pieceType = static_cast<PieceType>(k);
                } else {
                    assert(false);
                    break;
                }
            }

            int fromCol = -1;
            int fromRow = -1;
            int left = (int)s.length() - i;
            if (left > 2) {
                char ch = s.at(i);
                if (isalpha(ch)) {
                    fromCol = ch - 'a';
                } else if (isdigit(ch)) {
                    int r = ch - '0';
                    fromRow = 9 - r;
                }
                i++;
            }

            char colChr = s.at(i);
            char rowChr0 = s.at(i+1);

            if (isalpha(colChr) && isdigit(rowChr0)) {
                int col = colChr - 'a';
                int row = rowChr0 - '0';

                int dest = (9 - row) * 9 + col;

                auto move = findLegalMove(board, pieceType, fromCol, fromRow, dest);
                if (move.isValid()) {
                    board.make(move);
                    //                    std::cout << "move: " << s << ", " << move.from << "->" << move.dest << ", hashKey: " << hashKey << ", last hist: " << hists.back().toString() << std::endl;
                    continue;
                }

                board.show("Failed parsing move");
//                findLegalMove(board, pieceType, fromCol, fromRow, dest);
//                assert(false);
                break;
            }

            assert(false);
            break;
        }
    }

    return true;
}

opening::Move GameReader::findLegalMove(OpeningBoard& board, PieceType pieceType, int fromCol, int fromRow, int dest) {
    MoveList moveList;
    board.genLegal(moveList, board.side);

    for (int i = 0; i < moveList.end; i++) {
        auto move = moveList.list[i];
        auto movePiece = board.getPiece(move.from);
        if (movePiece.type == pieceType) {
            if (((fromCol < 0 || (fromCol >= 0 && fromCol == move.from %  9))
                 && (fromRow < 0 || (fromRow >= 0 && fromRow == move.from / 9))
                 && (dest < 0 || (dest >= 0 && dest == move.dest)))) {
                return move;
            }
        }
    }

    return Move(0, 0);
}

std::map<std::string, std::string> GameReader::wxf_parse(const std::string& gameString) const {
    std::map<std::string, std::string> r;

    std::string::size_type pos0 = 0, pos1 = 0;

    std::vector<std::string> output;

    if ((pos0 = gameString.find("START{", 0)) == std::string::npos ||
        (pos1 = gameString.find("}END", pos0 + 6)) == std::string::npos) {
        return r;
    }

    std::string headString(gameString.substr(0, pos0));
    std::string allMoves(gameString.substr(pos0 + 6, pos1 - pos0 - 6) );

    r["moves"] = allMoves;

    // Parse header
    std::string::size_type prev_pos = 0, pos = 0;
    bool first = true;

    while ((pos = headString.find("\n", pos)) != std::string::npos) {
        if (first) {
            prev_pos = pos++;
            first = false;
            continue;
        }
        std::string substring(headString.substr(prev_pos, pos-prev_pos));
        prev_pos = pos++;

        if (substring.find("RESULT", 0) != std::string::npos) {
            if (substring.find("1-0", 0) != std::string::npos) {
                r["Result"] = "1-0";
            } else if (substring.find("0-1", 0) != std::string::npos) {
                r["Result"] = "0-1";
            } else if (substring.find("0.5-0.5", 0) != std::string::npos) {
                r["Result"] = "0.5-0.5";
            }
        } else  if (substring.find("FEN", 0) != std::string::npos) {
            r["FEN"] = substring.substr(4);
        }
    }

    if (!r.empty()) {
        r["type"] = "wxf";
    }

    //
    //    // Body
    //    if (!allMoves.empty()) {
    //        // take out all comments
    //        auto comVec = Lib::splitString(allMoves, "(\\{(.|\r|\n)*?\\})|;.*");
    //        for (auto &&com : comVec) {
    //            Lib::removeSubstrs(allMoves, com);
    //        }
    //
    //        // trim out (())
    //        while (true) {
    //            auto start = allMoves.find('(');
    //            if (start == std::string::npos) {
    //                break;
    //            }
    //            int open = 1;
    //            bool trimmed = false;
    //            for (auto p=start+1; p<allMoves.length(); p++) {
    //                char ch = allMoves.at(p);
    //                if (ch=='(') {
    //                    open++;
    //                    continue;
    //                }
    //                if (ch==')') {
    //                    open--;
    //                    if (open==0) {
    //                        allMoves = allMoves.substr(0, start-1) + allMoves.substr(p+1);
    //                        trimmed = true;
    //                        break;
    //                    }
    //                }
    //            }
    //
    //            if (!trimmed) {
    //                allMoves = allMoves.substr(0, start-1);
    //                break;
    //            }
    //        }
    //
    //        std::string special = "+";
    //        Lib::removeSubstrs(allMoves, special);
    //        special = "x";
    //        Lib::removeSubstrs(allMoves, special);
    //
    //        r["moves"] = allMoves;
    //    }
    return r;
}

//bool GameReader::wxf_parse(const std::string& fen, const std::string& moves) {
//    newGame(fen);
//
//    //    printOut(fen.c_str());
//
//    std::regex re("\\s+");
//    std::sregex_token_iterator first {moves.begin(), moves.end(), re, -1}, last;
//    std::vector<std::string> words = {first, last};
//
//    std::vector<std::string> moveVec;
//
//    char buf[200];
//
//    for (auto &&s : words) {
//        if (!s.empty() && isalpha(s.at(0))) {
//            moveVec.push_back(s);
//
//            int i = 0;
//            char ch = s.at(i);
//
//            PieceType pieceType = PieceType::empty;
//            Side pieceSide = Side::black;
//            if (ch>='A' && ch <='Z') {
//                ch += 'a' - 'A';
//                pieceSide = Side::white;
//            }
//            if (ch>='a' && ch <='z') {
//                i++;
//                const char* p = strchr(pieceTypeName, ch);
//                if (p != NULL) {
//                    int k = (int)(p - pieceTypeName);
//                    pieceType = static_cast<PieceType>(k);
//                } else {
//                    assert(false);
//                    return false;
//                }
//            }
//
//            int fromCol = -1;
//
//            ch = s.at(i);
//            if (isdigit(ch)) {
//                fromCol = ch - '0';
//
//                if (pieceSide == Side::white) {
//                    fromCol = 10 - fromCol;
//                }
//            } else {
//                //                break; //something wrong
//                return false;
//            }
//            i++;
//
//            char adv = s.at(i);
//            i++;
//
//
//            int target = -1;
//            ch = s.at(i);
//            if (isdigit(ch)) {
//                target = ch - '0';
//            } else {
//                //break; //something wrong
//                return false;
//            }
//            i++;
//
//            auto move = wxf_findLegalMove(pieceType, fromCol - 1, adv, target);
//            if (move.isValid()) {
//                make(move);
//                //                    std::cout << "move: " << s << ", " << move.from << "->" << move.dest << ", hashKey: " << hashKey << ", last hist: " << hists.back().toString() << std::endl;
//                //                snprintf(buf, 200, "move %s, %s", s.c_str(), move.toString().c_str());
//                //                printOut(buf);
//                continue;
//            }
//
//            snprintf(buf, 200, "Failed parsing move %s", s.c_str());
//            printOut(buf);
//            //            move = wxf_findLegalMove(pieceType, fromCol - 1, adv, target);
//            assert(false);
//            return false;
//        }
//    }
//
//    //    printf("Succ parsing, hists.size()=%lu\n", hists.size());
//    return true;
//}
//
//Move GameReader::wxf_findLegalMove(PieceType pieceType, int fromCol, int adv, int target) {
//    MoveList moveList;
//    board.genLegalOnly(moveList, side);
//
//    for (int i = 0; i < moveList.end; i++) {
//        auto move = moveList.list[i];
//        auto movePiece = board.getPiece(move.from);
//        if (movePiece.type == pieceType) {
//            if (fromCol == move.from %  9) {
//
//                bool ok = true;
//                switch (pieceType) {
//                    case PieceType::rook:
//                    case PieceType::cannon:
//                    case PieceType::pawn:
//                    case PieceType::king:
//                        if (adv == '.') { // same ranks
//                            auto destCol = target;
//                            if (side == Side::white) {
//                                destCol = 10 - destCol;
//                            }
//                            destCol--;
//
//                            ok = destCol == move.dest %  9;
//                        } else {
//                            auto k = side == Side::white ? (move.from - move.dest) / 9 : (move.dest - move.from) / 9;
//                            if (adv == '-') {
//                                k = -k;
//                            }
//                            ok = target == k;
//                        }
//                        break;
//                    case PieceType::horse:
//                    case PieceType::advisor:
//                    case PieceType::elephant:
//                    {
//                        auto destCol = target;
//                        if (side == Side::white) {
//                            destCol = 10 - destCol;
//                        }
//                        destCol--;
//
//                        if (destCol == move.dest %  9) {
//                            auto k = side == Side::white ? (move.from - move.dest) : (move.dest - move.from);
//                            if (adv == '-') {
//                                k = -k;
//                            }
//
//                            ok = k > 0;
//                        } else {
//                            ok = false;
//                        }
//                        break;
//                    }
//
//                    default:
//                        break;
//                }
//
//                if (ok) {
//                    return move;
//                }
//            }
//        }
//    }
//
//    return Move(0, 0);
//}

