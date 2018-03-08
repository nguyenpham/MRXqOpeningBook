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
        bool verifyData() const;

    protected:
        Move _probe(OpeningBoard& board, MoveList* opMoveList = nullptr) const;

        i64 find(u64 key, int sd) const;
        static i64 find(u64 key, const char* data, i64 itemCount, int itemSize);
        
        bool verifyData(int sd) const;
        bool verifyData(OpeningBoard& board, int sd) const;

    protected:
        BookHeader header;

        BookItem* bookData[2];

        i64 allocatedSizes[2];

        std::string path;
    };

}

#endif /* OpBook_hpp */
