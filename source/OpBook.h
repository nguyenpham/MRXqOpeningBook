//
//  OpBook.hpp
//  Opening
//
//  Created by Tony Pham on 4/3/18.
//  Copyright © 2018 Softgaroo. All rights reserved.
//

#ifndef OpBook_hpp
#define OpBook_hpp

#include <stdio.h>
#include <vector>
#include <assert.h>
#include <fstream>
#include <mutex>
#include <map>

#include "Opening.h"

namespace opening {

    class OpeningBoard;

    class BookItem {
    public:
        u8  _key[8];
        u16 value;

        u64 key() const {
            return *((u64 *)_key);
        }

        void set(u64 key, u16 value) {
            *((u64 *)_key) = key;
            value = value;
        }
        void incValue(u64 key) {
            if (*((u64 *)_key) == key) {
                value++;
            } else {
                *((u64 *)_key) = key;
                value = 1;
            }
        }
    };

    class BookHeader {
    public:
        const static int BookHeaderSz = 128;
        const static int BookHeaderSignature = 13579;

        void reset() {
            memset(this, 0, sizeof(BookHeader));
            signature = BookHeaderSignature;
        }

        bool isValid() const {
            return signature == BookHeaderSignature;
        }

        bool saveFile(std::ofstream& outfile) const {
            outfile.seekp(0);
            if (outfile.write ((char*)&signature, BookHeaderSz)) {
                return true;
            }
            return false;
        }

        bool readFile(std::ifstream& file) {
            return file.read((char*)&signature, BookHeaderSz) && isValid();
        }

        void setNote(const char* str) {
            strncpy(textInfo, str, sizeof(textInfo));
        }

    public:
        u16 signature;
        u16 property;
        u32 reserveVar;
        i64 size[2];
        u8  reserve[8];

        char textInfo[128];
    };

    class OpBook {
    public:
        OpBook();
        virtual ~OpBook();

        Move probe(const std::string& fen, MoveList* opMoveList = nullptr) const;
        Move probe(const int8_t* pieceList, Side side, MoveList* opMoveList = nullptr) const;
        Move probe(const MoveList& moveList, MoveList* opMoveList = nullptr) const;
        Move probe(const std::vector<Piece> pieceVec, Side side, MoveList* opMoveList = nullptr) const;
        Move probe(OpeningBoard& board, MoveList* opMoveList = nullptr) const;

        bool load(const std::string& path);
        bool save(std::string path = "");

        i64 find(u64 key, int sd) const;
        u16 getValue(u64 idx, int sd) const;

        BookHeader* getHeader() {
            return &header;
        }

        BookItem* getData(int sd) {
            return bookData[sd];
        }

//        bool verifyData(OpeningBoard& board, int sd) const;

    protected:
        Move _probe(OpeningBoard& board, MoveList* opMoveList = nullptr) const;
        static i64 find(u64 key, const char* data, i64 itemCount, int itemSize);
        
    protected:
        BookHeader header;

        BookItem* bookData[2];

        i64 allocatedSizes[2];

        std::string path;
    };

}

#endif /* OpBook_hpp */
