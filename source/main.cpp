//
//  main.cpp
//  Opening
//
//  Created by Tony Pham on 3/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

#include <iostream>
#include <map>

#include "OpBoard.h"
#include "OpBookBuilder.h"

static void show_usage(std::string name)
{
    std::cerr << "Usage: " << name << " [-h] [-f inputpath] [-d directory] [-o outputpath] [-i info-copyright] [-max-fly fly] [-min-game number] [-only-white] [-only-black] [-uniform]" << std::endl;
    std::cerr << "Options:\n"
    << "\t-h,--help\t\tshow this help message and exit\n"
    << "\t-f\t\tinput path\n"
    << "\t-d\t\tinput directory\n"
    << "\t-o\t\toutput path\n"
    << "\t-i\t\tinfo/copyright string\n"
    << "\t-max-fly\t\tplies (half moves) to add for each game (default: infinite)\n"
    << "\t-min-game\t\tnumber of moves to be played to be kept in the book (default: 3)\n"
    << std::endl
    << "\tExample: opening -d c:\\games -o c:\\opening.xob \n"

    << std::endl;
}

int main(int argc, const char * argv[]) {
    if (argc < 3) {
        show_usage(argv[0]);
//        return 1;
    }

    std::map<std::string, std::string> paramMap;

    const char* singleParaNames[] = {
        "-only-white", "-only-black", "merge-book", "-uniform",
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
                if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                    auto destination = argv[i++]; // Increment 'i' so we don't get the argument as the next argv[i].
                    paramMap[pairParaNames[j + 1]] = destination;
                } else {
                    std::cerr << pairParaNames[j] << " option requires one argument." << std::endl;
                    return 1;
                }
                break;
            }
        }
    }

    std::cout << "Welcome to Opening - version: " << opening::getVersion() << std::endl;

    ///////////////////////////////////////////
//    opening::OpeningBoard board;
//    board.setFen("");
//    std::cout << "Origin hashKey = " << board.key() << std::endl;

    opening::OpBookBuilder opBookBuilder;

//    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/workinggame.pgn";
//    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/MyTestRule.pgn";
//    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/chasegames.pgn";
//    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/AsiaRule.pgn";

//    paramMap["file"] = "/Users/TonyPham/workspace/Opening/testgame/chasing04.pgn";
    paramMap["folder"] = "/Users/TonyPham/workspace/Opening/testgame";

    paramMap["out"] = "/Users/TonyPham/workspace/Opening/o.xob";
    paramMap["-only-white"] = "1";
    opBookBuilder.create(paramMap);

    opening::OpBook opBook;
    opBook.load("/Users/TonyPham/workspace/Opening/o.xob");
    opBook.verifyData();

    opening::OpeningBoard board;
    board.setFen("");

    auto move = opBook.probe(board);
    std::cout << "opening move = " << move.toString() << std::endl;

    return 0;
}
