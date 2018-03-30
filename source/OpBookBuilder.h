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

    class OpBookBuilder : public OpBook
    {
    private:
        const int CreatingAdditionalItemNumber = 1024 * 1024;
        const int Para_DefaultMinGame = 1;
        const int Para_DefaultMinPly = 10;
        const int Para_DefaultMaxPly = 1024 * 1024;

    public:
        void create(std::map<std::string, std::string> paramMap);

    private:
        void create_checkFlipping(OpeningBoard& board, std::vector<Move>& moves, Side workingSide);

        bool create_add(const OpeningBoard& board);

        void create(const std::vector<std::string>& folderVec, const std::map<std::string, std::string>& paramMap);
        void create(const std::string& inputPath, const std::map<std::string, std::string>& paramMap);

        bool createSave(const std::string& path_, const std::map<std::string, std::string>& paramMap);
        void createInit(Side side);

    };

} // namespace opening

#endif /* OpBookBuilder_hpp */

