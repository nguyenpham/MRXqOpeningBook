//
//  OpBookBuilder.hpp
//  Opening
//
//  Created by Tony Pham on 7/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

#ifndef OpBookBuilder_hpp
#define OpBookBuilder_hpp

#include "OpBook.h"

namespace opening {
#include <functional>

extern void dumbReportString(std::string msg);
extern void dumbReportNumbers(int fileCnt, int gameCnt, int nodeCnt, int addedNodeCnt);

    class OpBookBuilder : public OpBook
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
        bool verify(OpBook& book, int sd, std::function<void(std::string)> reportString, std::function<void(int, int, int, int)> reportNumbers);
        bool verify(OpBook& book, OpeningBoard& board, int sd, int ply, std::function<void(int, int, int, int)> reportNumbers);

    private:
        std::map<u64, u64> m_keyMap;

        int m_reportFileCnt, m_reportCnt, m_reportNodeCnt;

    };

} // namespace opening

#endif /* OpBookBuilder_hpp */

