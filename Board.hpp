#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "Piece.hpp"

struct Move {
    int startSquare;     // 0..63
    int targetSquare;    // 0..63
    bool isEnPassant;
    int promotionPiece;  // PieceType index or 0
    bool isCastling;

    Move(int start, int target, bool enPassant = false, int promotion = 0, bool isCastling = false)
        : startSquare(start),
          targetSquare(target),
          isEnPassant(enPassant),
          promotionPiece(promotion),
          isCastling(isCastling) {}
};

class Board {
   private:
    // Bitboards for each piece type and color
    std::array<uint64_t, 12> bitboards;  // Indexes correspond to PieceType enum

    // Bitboards for convenience
    uint64_t whitePieces;
    uint64_t blackPieces;
    uint64_t allPieces;

    // Helper functions
    int charToPieceIndex(char c) const;
    char pieceIndexToChar(int pieceIndex) const;
    void updateAggregateBitboards();

    // Move generation helper functions
    void generatePawnMoves(int pawnPieceIndex);
    void generateKnightMoves(int knightPieceIndex);
    void generateBishopMoves(int bishopPieceIndex);
    void generateRookMoves(int rookPieceIndex);
    void generateQueenMoves(int queenPieceIndex);
    void generateKingMoves(int kingPieceIndex);

    void addPawnPromotionMoves(int startSquare, int targetSquare);

    uint64_t knightAttackBitboard(int square) const;
    uint64_t kingAttackBitboard(int square) const;
    uint64_t bishopAttackBitboard(int square, uint64_t occupancy) const;
    uint64_t rookAttackBitboard(int square, uint64_t occupancy) const;
    uint64_t queenAttackBitboard(int square, uint64_t occupancy) const;
    bool isSquareAttacked(int square, int attackingColor) const;

    bool isMoveLegal(const Move& move);

   public:
    int colorTurn;  // 1 for white, -1 for black
    std::string castleStatus;
    std::vector<Move> moves;
    Move lastMove;
    int enPassantTarget;  // Square index (0-63) or -1

    explicit Board(const std::string& epd);

    void makeMove(const Move& move, bool updateMoves = true);
    bool isLastMoveTile(int tileIndex) const;
    std::string boardToEPD() const;
    void generateMoves();

    // Helper methods
    int getPieceAt(int square) const;  // Returns PieceType index or -1 if empty

    bool canWhiteCastleKingside;
    bool canWhiteCastleQueenside;
    bool canBlackCastleKingside;
    bool canBlackCastleQueenside;

    bool isKingInCheck(int color) const;
    bool isCheckmate();
};

#endif  // BOARD_HPP
