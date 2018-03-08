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
