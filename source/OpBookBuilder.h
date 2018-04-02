
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

#ifndef OpBookBuilder_hpp
#define OpBookBuilder_hpp

#include "OpBook.h"

namespace opening {
#include <functional>

extern void dumbReportString(std::string msg);
extern void dumbReportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt);

    class OpBookBuilder : public OpBookCore
    {
    private:
        const int CreatingAdditionalItemNumber = 1024 * 1024;
        const int Para_DefaultMinGame = 1;
        const int Para_DefaultMinGameLength = 20;
        const int Para_DefaultAddToPly = 20;

    public:
        void create(std::map<std::string, std::string> paramMap, std::function<void(std::string)> reportString = &dumbReportString, std::function<void(int, int, int, int)> reportNumbers = &dumbReportNumbers);
        void verify(std::map<std::string, std::string> paramMap, std::function<void(std::string)> reportString = &dumbReportString, std::function<void(int, int, int, int)> reportNumbers = &dumbReportNumbers);

    private:
        void create_checkFlipping(OpeningBoard& board, std::vector<Move>& moves, Side workingSide);

        bool create_add(const OpeningBoard& board);

        void create(const std::vector<std::string>& folderVec, const std::map<std::string, std::string>& paramMap, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers);
        void create(const std::string& inputPath, const std::map<std::string, std::string>& paramMap,
                    std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers);

        bool createSave(const std::string& path_, const std::map<std::string, std::string>& paramMap);
        void createInit(Side side);

    private:
        bool verify(OpBookCore& book, int sd, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers);
        bool verify(OpBookCore& book, OpeningBoard& board, int sd, int ply, std::function<void(int, int, int, int)> reportNumbers);

    private:
        std::map<u64, u64> m_keyMap;

        int m_reportFileCnt, m_reportCnt, m_reportNodeCnt;

    };

} // namespace opening

#endif /* OpBookBuilder_hpp */

