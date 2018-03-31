
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


#ifndef OpeningBoard_h
#define OpeningBoard_h

#include <vector>
#include "Opening.h"

namespace opening {

    extern const std::string originalFen;
    extern const char* pieceTypeName;
    extern const u64 originHashKey;
    extern const u64 hashTable[];

    class Piece {
    public:
        PieceType type;
        Side side;

        // this variable is used in some purpose such as to setup a board
        int idx;

    public:
        Piece() {}
        Piece(PieceType _type, Side _side, int idx = -1) {
            set(_type, _side, idx);
        }

        Piece(PieceType _type, Side _side, Squares square) {
            set(_type, _side, static_cast<int>(square));
        }

        void set(PieceType _type, Side _side, int _idx = -1) {
            type = _type; side = _side; idx = _idx;
        }

        bool operator == (const Piece& other) const {
            return type == other.type && side == other.side;
        }

        bool operator != (const Piece& other) const {
            return type != other.type || side != other.side;
        }

        void setEmpty() {
            set(PieceType::empty, Side::none, -1);
        }

        bool isEmpty() const {
            return type == PieceType::empty;
        }

        bool isPiece(PieceType _type, Side _side) const {
            return type == _type && side == _side;
        }

        bool isValid() const {
            return (side == Side::none && type == PieceType::empty) || (side != Side::none && type != PieceType::empty);
        }

        static std::string toString(const PieceType type, const Side side) {
            int k = static_cast<int>(type);
            char ch = pieceTypeName[k];
            if (side == Side::white) {
                ch += 'A' - 'a';
            }
            return std::string(1, ch);
        }

        std::string toString() const {
            return toString(type, side);
        }

    };

    class MoveCore {
    public:
        int8_t from, dest;

        MoveCore() {
            from = dest = -1;
        }
        MoveCore(int _from, int _dest) {
            from = _from; dest = _dest;
        }
    };

    class Move {
    public:
        int8_t  from, dest;
        i32     score;

        PieceType type, capType;
        Side side;

    public:
        Move() {}
        Move(PieceType _type, Side _side, int _from, int _dest) {
            set(_type, _side, _from, _dest);
        }
        Move(int from, int dest) {
            set(from, dest);
        }

        bool operator == (const Move& otherMove) const {
            return from == otherMove.from && dest == otherMove.dest;
        }

        bool operator != (const Move& otherMove) const {
            return from != otherMove.from || dest != otherMove.dest;
        }

        void set(PieceType _type, Side _side, int _from, int _dest) {
            type = _type; side = _side; from = _from; dest = _dest;
        }

        void set(int _from, int _dest) {
            from = _from; dest = _dest;
        }

        bool isValid() const {
            return from != dest && from >= 0 && from < 90 && dest >= 0 && dest < 90;
        }

        std::string toString() const {
            std::ostringstream stringStream;
            stringStream << Piece(type, side).toString()
            << posToCoordinateString(from) << posToCoordinateString(dest);
            return stringStream.str();
        }
    };

    class Hist {
    public:
        Move move;
        Piece cap;
        u64 hashKey;

        std::vector<int> ambiguousVec;
        std::string comment;
        char checkOrMate;

    public:
        Hist() { checkOrMate = 0; }

        void set(const Move& _move) { move = _move; }

        bool isValid() const {
            return move.isValid() && cap.isValid();
        }

        std::string moveString(Notation notation);
        std::string moveString_algebraic();
        std::string moveString_traditional(Notation notation);

        static std::string moveString_san(const Move& move);
        static std::string moveString_coordinate(const Move& move);
        static std::string moveString_coordinate(int from, int dest);

    };

    class MoveList {
        const static int MaxMoveNumber = 400;

    public:
        Move list[MaxMoveNumber];
        int end;

    public:
        MoveList() { reset(); }

        void reset() { end = 0; }

        void add(const Move& move) { list[end] = move; end++; }

        void add(PieceType type, Side side, int from, int dest) {
            list[end].set(type, side, from, dest); end++;
        }

        std::string toString() const {
            std::ostringstream stringStream;
            for (int i = 0; i < end; i++) {
                if (i % 2 == 0) {
                    stringStream << i / 2 + 1 << ") ";
                }
                stringStream << list[i].toString() << " ";
            }
            return stringStream.str();
        }
    };

    class TheResult {
    public:
        TheResult() {
            reset();
        }
        TheResult(ResultType _result, ReasonType _reason = ReasonType::noreason, std::string _comment = "") {
            result = _result;
            reason = _reason;
            comment = _comment;
        }

        void reset() {
            result = ResultType::noresult;
            reason = ReasonType::noreason;
            comment = "";
        }

        ResultType result;
        ReasonType reason;
        std::string comment;

        bool isNone() const {
            return result == ResultType::noresult;
        }

        std::string toShortString() const {
            switch (result) {
            case ResultType::draw:
                return "1/2-1/2";
            case ResultType::win:
                return "1-0";
            case ResultType::loss:
                return "0-1";
            case ResultType::noresult:
            default:
                break;
            }

            return "";
        }
    };

    class OpeningBoard {
    private:
        Piece pieces[90];
        TheResult result;

    public:
        int8_t pieceList[2][16];
        Side side;

    public:
        void newGame(const std::string& fen);

        void set(int pos, PieceType type, Side side) {
            pieces[pos].set(type, side);
            pieceList_set((int8_t *)pieceList, pos, type, side);
        }

        Piece getPiece(int pos) const {
            return pieces[pos];
        }

        Side getSide(int pos) const {
            return pieces[pos].side;
        }

        TheResult getResult() const {
            return result;
        }

//        std::string getResultString() const;
//
//        Reason getReason() const {
//            return reason;
//        }

        bool isEmpty(int pos) const {
            return pieces[pos].type == PieceType::empty;
        }

        void setEmpty(int pos) {
            pieces[pos].setEmpty();
        }

        void genLegal(MoveList& moves, Side side, int from = -1, int dest = -1);
        void gen(MoveList& moveList, Side side, PieceType type = PieceType::empty, bool capOnly = false) const;

        bool isIncheck(Side beingAttackedSide) const;

        void make(const Move& move, Hist& hist);
        void takeBack(const Hist& hist);

        void make(int from, int dest, bool createMoveStrings = false);
        void make(const Move& move, bool createMoveStrings = false);
        void takeBack();

        TheResult makeRule();

        void setResult(const std::string& fen);

        void setFen(const std::string& fen);
        std::string startingFenString() const {
            return startingFen;
        }

        bool setup(const std::vector<Piece> pieceVec, Side side);

        void cloneFrom(const OpeningBoard& board);

        void show(const char* msg = nullptr) const;

        std::string getPgn() const;
        std::string getFen() const;
        std::string getFen(Side side, int halfCount = 0, int fullMoveCount = 0) const;

        void reset() {
            for (int i = 0; i < 90; i++) {
                setEmpty(i);
            }
            histList.clear();
            result.result = ResultType::noresult;
        }

        static int flip(int pos, FlipMode flipMode);
        static FlipMode flip(FlipMode oMode, FlipMode flipMode);
        void flip(FlipMode flipMode);

        bool pieceList_setupBoard(const int8_t *pieceList = nullptr);
        static void pieceList_reset(int8_t *pieceList);

        bool isThereAttacker() const {
            return pieceList_isThereAttacker((const int8_t *)pieceList);
        }

        bool isValid() const;

        u64 key() const {
            return hashKey;
        }

        std::vector<Hist>& getHistList() {
            return histList;
        }
        const std::vector<Hist>& getHistList() const {
            return histList;
        }
        void initHashKey();

        bool isLegalMove(const Move& move) {
            return isLegalMove(move.from, move.dest);
        }
        bool isLegalMove(int from, int dest);

        Move moveFromString(const std::string& str);
        Move moveFromLanString(std::string str);
        Move moveFromSanString(std::string str);

        Move moveFromString_san(const std::string& s);
        static Move moveFromString_algebraicCoordinates(const std::string& s);

        void collectExtraMoveInfo(const Move& makingmove, Hist& hist);
        void collectExtraMoveInfo_checkOrMate(Hist& hist);

        static std::string squareString(int pos);

    private:
        Move findLegalMove(PieceType pieceType, int fromCol, int fromRow, int dest);

        std::string toString() const;

        void gen_addMove(MoveList& moveList, int from, int dest, bool capOnly) const;
        int findKing(Side side) const;

        bool isPiece(int pos, PieceType type, Side side) const {
            auto p = pieces[pos];
            return p.type==type && p.side==side;
        }

        static bool pieceList_set(int8_t *pieceList, int pos, PieceType type, Side side);
        static bool pieceList_setEmpty(int8_t *pieceList, int pos);
        static bool pieceList_setEmpty(int8_t*pieceList, int pos, int sd);
        static bool pieceList_setEmpty(int8_t *pieceList, int pos, PieceType type, Side side);
        static bool pieceList_isThereAttacker(const int8_t *pieceList);

        bool pieceList_make(const Hist& hist);
        bool pieceList_takeback(const Hist& hist);

    private:
        std::string startingFen;
        void xorHashKey(int pos);

        u64 hashKey;

        std::vector<Hist> histList;
    };

} // namespace opening

#endif /* OpeningBoard.h */

