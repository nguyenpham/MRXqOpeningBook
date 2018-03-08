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

#include <iostream>
#include <map>

#include "OpBoard.h"
#include "OpBookBuilder.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " [-h] [create] [-f inputpath] [-d directory] [-o outputpath] [-i info-copyright] [-max-fly fly] [-min-game number] [-only-white] [-only-black] [-uniform]" << std::endl;
    std::cerr << "Options:\n"
    << "\t-h,--help\tshow this help message and exit\n"
    << "\tcreate\t\tcreate opening book\n"
    << "\tverify\t\tverify opening book\n"
    << "\t-f\t\t\tinput path\n"
    << "\t-d\t\t\tinput directory\n"
    << "\t-o\t\t\toutput path (default openingbook.xob)\n"
    << "\t-i\t\t\tinfo/copyright string\n"
    << "\t-min-fly\tmininum game length in plies (half moves) to add for each game (default: 20)\n"
    << "\t-max-fly\tplies (half moves) to add for each game (default: infinite)\n"
    << "\t-min-game\tnumber of moves to be played to be kept in the book (default: 3)\n"
    << std::endl
    << "\tExample: " << name << " create -d c:\\games -i \"created by John 2018\"\n"

    << std::endl;
}

const char* appname = "MRXqOpeningBook";

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        show_usage(appname);
        return 1;
    }

    std::map<std::string, std::string> paramMap;

    const char* singleParaNames[] = {
        "-only-white", "-only-black", "create", "verify", "merge", "-uniform",
        nullptr
    };

    const char* pairParaNames[] = {
        "-d", "folder",
        "-f", "file",
        "-o", "out",
        "-out", "out",
        "-max-ply", "maxply",
        "-min-ply", "minply",
        "-min-game", "mingame",
        "-i", "info",

        nullptr, nullptr
    };

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        }
        if ((arg == "-v") || (arg == "--verbose")) {
            opening::openingVerbose = true;
            continue;
        }

        for(int j = 0; singleParaNames[j]; j++) {
            if (arg == singleParaNames[j]) {
                paramMap[arg] = "1";
            }
        }

        for(int j = 0; pairParaNames[j]; j += 2) {
            if (arg == pairParaNames[j]) {
                if (i + 1 < argc) {
                    paramMap[pairParaNames[j + 1]] = argv[++i];
                } else {
                    std::cerr << pairParaNames[j] << " option requires one argument." << std::endl;
                    return 1;
                }
                break;
            }
        }
    }

    std::cout << "Welcome to " << appname << " - version: " << opening::getVersion() << std::endl;

    ///////////////////////////////////////////
    auto it = paramMap.find("create");
    if (it != paramMap.end()) {
        opening::OpBookBuilder opBookBuilder;
        opBookBuilder.create(paramMap);
        return 0;
    }

    it = paramMap.find("verify");
    if (it != paramMap.end()) {
        it = paramMap.find("file");
        if (it != paramMap.end()) {
            opening::OpBook opBook;
            opBook.load(it->second);
            opBook.verifyData();
        }
        return 0;
    }

////    opening::OpeningBoard board;
////    board.setFen("");
////    std::cout << "Origin hashKey = " << board.key() << std::endl;
//
//
////    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/workinggame.pgn";
////    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/MyTestRule.pgn";
////    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/chasegames.pgn";
////    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/AsiaRule.pgn";
//
////    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/chasing04.pgn";
//    paramMap["folder"] = "/Users/TonyPham/workspace/Opening/testgame";
//
//    paramMap["out"] = "/Users/TonyPham/workspace/Opening/o.xob";
//    paramMap["-only-white"] = "1";
//    opBookBuilder.create(paramMap);
//
//    opening::OpBook opBook;
//    opBook.load("/Users/TonyPham/workspace/Opening/o.xob");
//    opBook.verifyData();
//
//    opening::OpeningBoard board;
//    board.setFen("");
//
//    auto move = opBook.probe(board);
//    std::cout << "opening move = " << move.toString() << std::endl;

    return 0;
}
