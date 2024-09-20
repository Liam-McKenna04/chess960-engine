#ifndef PIECE_HPP
#define PIECE_HPP

#include <string>

enum BasePieceType {
    King = 0,
    Queen,
    Bishop,
    Knight,
    Rook,
    Pawn
};

enum PieceType {
    WhiteKing = BasePieceType::King,
    WhiteQueen = BasePieceType::Queen,
    WhiteBishop = BasePieceType::Bishop,
    WhiteKnight = BasePieceType::Knight,
    WhiteRook = BasePieceType::Rook,
    WhitePawn = BasePieceType::Pawn,
    BlackKing = BasePieceType::King + 6,
    BlackQueen = BasePieceType::Queen + 6,
    BlackBishop = BasePieceType::Bishop + 6,
    BlackKnight = BasePieceType::Knight + 6,
    BlackRook = BasePieceType::Rook + 6,
    BlackPawn = BasePieceType::Pawn + 6
};


#endif
