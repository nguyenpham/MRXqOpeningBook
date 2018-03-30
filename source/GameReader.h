//
//  GameReader.hpp
//  Opening
//
//  Created by Tony Pham on 4/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

#ifndef GameReader_hpp
#define GameReader_hpp

#include <stdio.h>
#include <string>
#include <list>
#include <map>

#include "Opening.h"

class Move;
class OpeningBoard;

namespace opening {

    class GameReader {
    public:
        GameReader(const std::string& path);
        bool init(const std::string& path);

        static std::string loadFile(const std::string& fileName);

        bool nextGame(OpeningBoard& board);

        int currentGameIdx() const {
            return workingGameIdx - 1;
        }

    private:
        std::map<std::string, std::string> parse(const std::string& gameString) const;
        std::map<std::string, std::string> pgn_parse(const std::string& gameString) const;

        bool parse(OpeningBoard& board, const std::string& fen, const std::string& moves, const std::string& result);

        opening::Move findLegalMove(OpeningBoard& board, PieceType pieceType, int fromCol, int fromRow, int dest);

        std::map<std::string, std::string> wxf_parse(const std::string& gameString) const;
//        bool wxf_parse(OpeningBoard& board, const std::string& fen, const std::string& moves);

    private:
        std::vector<std::string> gameStringVector;
        int workingGameIdx;
    };

}

#endif /* GameReader_hpp */
