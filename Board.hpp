#ifndef BOARD_HPP
#define BOARD_HPP

#include <algorithm>
#include <array>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Piece.hpp"

struct Move {
    int startSquare;
    int targetSquare;
    bool isEnPassant;
    int promotionPiece;

    Move(int start, int target, bool enPassant = false, int promotion = 0)
        : startSquare(start),
          targetSquare(target),
          isEnPassant(enPassant),
          promotionPiece(promotion) {}
};

class Board {
   private:
    static const int BOARD_SIZE = 64;
    static const std::array<int, 8> DIRECTION_OFFSETS;
    std::array<std::array<int, 8>, BOARD_SIZE> numSquaresToEdge;

    void initializeNumSquaresToEdge();
    void generateStraightMoves(int startSquare, int piece);
    void generateKnightMoves(int startSquare);
    void generateKingMoves(int startSquare);
    void generatePawnMoves(int startSquare);
    void addPawnPromotionMoves(int startSquare, int targetSquare);

    static char toEPDChar(int piece);
    static int fromEPDChar(char c);

   public:
    int colorTurn = 1;  // 1 for white, -1 for black
    std::string castleStatus = "KkQq";
    std::array<int, BOARD_SIZE> square;
    std::vector<Move> moves;
    Move lastMove;
    int enPassantTarget = -1;

    explicit Board(const std::string& epd);

    void makeMove(const Move& move);
    bool isLastMoveTile(int tileIndex) const;
    std::string boardToEPD() const;
    void generateMoves();
};

Board::Board(const std::string& epd) : lastMove(-1, -1) {
    std::istringstream iss(epd);
    std::string token;
    int index = 0;

    while (std::getline(iss, token, '/')) {
        for (char c : token) {
            if (std::isdigit(c)) {
                index += c - '0';
            } else {
                square[index++] = fromEPDChar(c);
            }
        }
    }

    initializeNumSquaresToEdge();
}

void Board::makeMove(const Move& move) {
    int piece = square[move.startSquare];

    if (move.promotionPiece != 0) {
        piece = move.promotionPiece;
    }

    square[move.targetSquare] = piece;
    square[move.startSquare] = Piece::Empty;

    if (move.isEnPassant) {
        int capturedPawnSquare = move.targetSquare + (colorTurn * 8);
        square[capturedPawnSquare] = Piece::Empty;
    }

    if (std::abs(piece) == Piece::Pawn &&
        std::abs(move.startSquare - move.targetSquare) == 16) {
        enPassantTarget = (move.startSquare + move.targetSquare) / 2;
    } else {
        enPassantTarget = -1;
    }

    colorTurn = -colorTurn;
    lastMove = move;
    generateMoves();
}

bool Board::isLastMoveTile(int tileIndex) const {
    return tileIndex == lastMove.startSquare ||
           tileIndex == lastMove.targetSquare;
}

std::string Board::boardToEPD() const {
    std::ostringstream epd;
    int emptyCount = 0;

    for (int rank = 0; rank < 8; ++rank) {
        for (int file = 0; file < 8; ++file) {
            int piece = square[rank * 8 + file];
            if (piece == Piece::Empty) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    epd << emptyCount;
                    emptyCount = 0;
                }
                epd << toEPDChar(piece);
            }
        }
        if (emptyCount > 0) {
            epd << emptyCount;
            emptyCount = 0;
        }
        if (rank < 7) epd << '/';
    }

    return epd.str();
}

void Board::generateMoves() {
    moves.clear();
    for (int startSquare = 0; startSquare < BOARD_SIZE; startSquare++) {
        int piece = square[startSquare];
        if (Piece::matchesColor(piece, colorTurn)) {
            switch (std::abs(piece)) {
                case Piece::King:
                    generateKingMoves(startSquare);
                    break;
                case Piece::Queen:
                case Piece::Bishop:
                case Piece::Rook:
                    generateStraightMoves(startSquare, piece);
                    break;
                case Piece::Knight:
                    generateKnightMoves(startSquare);
                    break;
                case Piece::Pawn:
                    generatePawnMoves(startSquare);
                    break;
            }
        }
    }
}

void Board::initializeNumSquaresToEdge() {
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            int numNorth = 7 - rank;
            int numSouth = rank;
            int numWest = file;
            int numEast = 7 - file;

            int squareIndex = rank * 8 + file;

            numSquaresToEdge[squareIndex] = {numNorth,
                                             numSouth,
                                             numWest,
                                             numEast,
                                             std::min(numNorth, numWest),
                                             std::min(numSouth, numEast),
                                             std::min(numNorth, numEast),
                                             std::min(numSouth, numWest)};
        }
    }
}

void Board::generateStraightMoves(int startSquare, int piece) {
    int startDirIndex = (std::abs(piece) == Piece::Bishop) ? 4 : 0;
    int endDirIndex = (std::abs(piece) == Piece::Rook) ? 4 : 8;

    for (int directionIndex = startDirIndex; directionIndex < endDirIndex;
         directionIndex++) {
        for (int n = 0; n < numSquaresToEdge[startSquare][directionIndex];
             n++) {
            int targetSquare =
                startSquare + DIRECTION_OFFSETS[directionIndex] * (n + 1);
            int pieceOnTargetSquare = square[targetSquare];

            if (Piece::matchesColor(piece, pieceOnTargetSquare)) {
                break;
            }
            moves.emplace_back(startSquare, targetSquare);

            if (Piece::matchesColor(-piece, pieceOnTargetSquare)) {
                break;
            }
        }
    }
}

void Board::generateKnightMoves(int startSquare) {
    static const int knightMoves[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
    int piece = square[startSquare];

    for (int offset : knightMoves) {
        int targetSquare = startSquare + offset;

        if (targetSquare < 0 || targetSquare >= BOARD_SIZE) continue;

        int startFile = startSquare % 8;
        int targetFile = targetSquare % 8;
        if (std::abs(startFile - targetFile) > 2) continue;

        int pieceOnTargetSquare = square[targetSquare];

        if (!Piece::matchesColor(piece, pieceOnTargetSquare)) {
            moves.emplace_back(startSquare, targetSquare);
        }
    }
}

void Board::generateKingMoves(int startSquare) {
    for (int directionIndex = 0; directionIndex < 8; directionIndex++) {
        if (numSquaresToEdge[startSquare][directionIndex] > 0) {
            int targetSquare = startSquare + DIRECTION_OFFSETS[directionIndex];
            int pieceOnTargetSquare = square[targetSquare];

            if (!Piece::matchesColor(colorTurn, pieceOnTargetSquare)) {
                moves.emplace_back(startSquare, targetSquare);
            }
        }
    }
}

void Board::generatePawnMoves(int startSquare) {
    int forwardDirection = (colorTurn == 1) ? -8 : 8;
    int startRank = (colorTurn == 1) ? 6 : 1;

    int oneForwardSquare = startSquare + forwardDirection;
    if (square[oneForwardSquare] == 0) {
        if ((colorTurn == 1 && oneForwardSquare / 8 == 0) ||
            (colorTurn == -1 && oneForwardSquare / 8 == 7)) {
            addPawnPromotionMoves(startSquare, oneForwardSquare);
        } else {
            moves.emplace_back(startSquare, oneForwardSquare);
        }

        if (startSquare / 8 == startRank) {
            int twoForwardSquare = startSquare + 2 * forwardDirection;
            if (square[twoForwardSquare] == 0) {
                moves.emplace_back(startSquare, twoForwardSquare);
            }
        }
    }

    for (int direction : {-1, 1}) {
        int targetSquare = startSquare + forwardDirection + direction;
        if (targetSquare >= 0 && targetSquare < BOARD_SIZE &&
            std::abs((startSquare % 8) - (targetSquare % 8)) == 1) {
            if (square[targetSquare] != Piece::Empty &&
                !Piece::matchesColor(square[targetSquare], colorTurn)) {
                if ((colorTurn == 1 && targetSquare / 8 == 0) ||
                    (colorTurn == -1 && targetSquare / 8 == 7)) {
                    addPawnPromotionMoves(startSquare, targetSquare);
                } else {
                    moves.emplace_back(startSquare, targetSquare);
                }
            }
            if (targetSquare == enPassantTarget) {
                moves.emplace_back(startSquare, targetSquare, true);
            }
        }
    }
}

void Board::addPawnPromotionMoves(int startSquare, int targetSquare) {
    static const int promotionPieces[4] = {Piece::Queen, Piece::Rook,
                                           Piece::Bishop, Piece::Knight};
    for (int piece : promotionPieces) {
        moves.emplace_back(startSquare, targetSquare, false, piece * colorTurn);
    }
}

char Board::toEPDChar(int piece) {
    static const char pieceChars[] = " KQBNRP";
    char c = pieceChars[std::abs(piece)];
    return (piece > 0) ? c : std::tolower(c);
}

int Board::fromEPDChar(char c) {
    static const std::string pieceChars = " KQBNRP";
    int color = std::isupper(c) ? Piece::White : Piece::Black;
    int type = pieceChars.find(std::toupper(c));
    return Piece::create(type, color);
}
const std::array<int, 8> Board::DIRECTION_OFFSETS = {8, -8, -1, 1,
                                                     7, -7, 9,  -9};

#endif  // BOARD_HPP