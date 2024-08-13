#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Piece.hpp"

using namespace std;
struct Move {
    Move(int startSquare, int targetSquare, bool isEnPassant = false) {
        this->startSquare = startSquare;
        this->targetSquare = targetSquare;
        this->isEnPassant = isEnPassant;
    }
    int startSquare;
    int targetSquare;
    bool isEnPassant;
};

class Board {  // board at a specific state
   private:
    int DIRECTION_OFFSETS[8] = {8, -8, -1, 1, 7, -7, 9, -9};
    array<array<int, 8>, 64> numSquaresToEdge;
    array<array<int, 8>, 64> initializeNumSquaresToEdge() {
        array<array<int, 8>, 64> result{};

        for (int file = 0; file < 8; file++) {
            for (int rank = 0; rank < 8; rank++) {
                int numNorth = 7 - rank;
                int numSouth = rank;
                int numWest = file;
                int numEast = 7 - file;

                int squareIndex = rank * 8 + file;

                result[squareIndex] = {numNorth,
                                       numSouth,
                                       numWest,
                                       numEast,
                                       min(numNorth, numWest),
                                       min(numSouth, numEast),
                                       min(numNorth, numEast),
                                       min(numSouth, numWest)};
            }
        }

        return result;
    }

   public:
    int colorTurn = 1;  // positive 1 for white, -1 for black
    string castleStatus = "KkQq";
    int square[64];
    vector<Move> moves;
    Move lastMove = Move(-1, -1);
    int enPassantTarget =
        -1;  // The square where en passant capture is possible

    Board(const string& epd) {
        istringstream iss(epd);
        string token;
        int index = 0;
        while (getline(iss, token, '/')) {
            for (char c : token) {
                if (isdigit(c)) {
                    int emptyCount = c - '0';
                    for (int i = 0; i < emptyCount; ++i) {
                        square[index++] = Piece::Empty;
                    }
                } else {
                    square[index++] = fromEPDChar(c);
                }
            }
        }
        numSquaresToEdge = initializeNumSquaresToEdge();
    }

    void makeMove(const Move& move) {
        int piece = square[move.startSquare];
        square[move.targetSquare] = piece;
        square[move.startSquare] = Piece::Empty;

        // Handle en passant capture
        if (move.isEnPassant) {
            int capturedPawnSquare = move.targetSquare + (colorTurn * 8);
            square[capturedPawnSquare] = Piece::Empty;
        }

        // Set en passant target for the next move
        if (abs(piece) == Piece::Pawn &&
            abs(move.startSquare - move.targetSquare) == 16) {
            enPassantTarget = (move.startSquare + move.targetSquare) / 2;
        } else {
            enPassantTarget = -1;
        }

        colorTurn = -colorTurn;
        lastMove = move;
        generateMoves();
    }

    bool isLastMoveTile(int tileIndex) const {
        return tileIndex == lastMove.startSquare ||
               tileIndex == lastMove.targetSquare;
    }
    char toEPDChar(int piece) {
        char pieceChars[] = " KQBNRP";
        char c = pieceChars[abs(piece)];
        return (piece > 0) ? c : tolower(c);
    }

    int fromEPDChar(char c) {
        string pieceChars = " KQBNRP";
        int color = isupper(c) ? Piece::White : Piece::Black;
        int type = pieceChars.find(toupper(c));
        return Piece::create(type, color);
    }

    string boardToEPD() {
        ostringstream epd;
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
                    epd << Board::toEPDChar(piece);
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

    void generateMoves() {
        moves.clear();
        for (int startSquare = 0; startSquare < 64; startSquare++) {
            int piece = square[startSquare];
            if (Piece::matchesColor(square[startSquare], colorTurn)) {
                if (Piece::isPiece(piece, Piece::King)) {
                    generateKingMoves(startSquare);
                }
                if (Piece::isPiece(piece, Piece::Queen) ||
                    Piece::isPiece(piece, Piece::Bishop) ||
                    Piece::isPiece(piece, Piece::Rook)) {
                    generateStraightMoves(startSquare, piece);
                }
                if (Piece::isPiece(piece, Piece::Knight)) {
                    generateKnightMoves(startSquare);
                }
                if (Piece::isPiece(piece, Piece::Pawn)) {
                    generatePawnMoves(startSquare);
                }
            }
        }
    };

    void generateStraightMoves(int startSquare, int piece) {
        int startDirIndex = (abs(piece) == Piece::Bishop) ? 4 : 0;
        int endDirIndex = (abs(piece) == Piece::Rook) ? 4 : 8;

        // Generates the moves of a Queen, Bishop, or Knight
        // Don't love this implementation
        for (int directionIndex = startDirIndex; directionIndex < endDirIndex;
             directionIndex++) {
            for (int n = 0; n < numSquaresToEdge[startSquare][directionIndex];
                 n++) {
                int targetSquare =
                    startSquare + DIRECTION_OFFSETS[directionIndex] * (n + 1);
                int pieceOnTargetSquare = square[targetSquare];

                // Blocked by piece of same color
                if (Piece::matchesColor(piece, pieceOnTargetSquare)) {
                    break;
                }
                this->moves.push_back(Move(startSquare, targetSquare));

                // Blocked by piece of opposite color, can't move further after
                // capturing
                if (Piece::matchesColor(-piece, pieceOnTargetSquare)) {
                    break;
                }
            }
        }
    }
    void generateKnightMoves(int startSquare) {
        const int knightMoves[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
        int piece = square[startSquare];

        for (int offset : knightMoves) {
            int targetSquare = startSquare + offset;

            // Check if the target square is on the board
            if (targetSquare < 0 || targetSquare >= 64) continue;

            // Check if the move is valid (not moving more than 2 files away)
            int startFile = startSquare % 8;
            int targetFile = targetSquare % 8;
            if (abs(startFile - targetFile) > 2) continue;

            int pieceOnTargetSquare = square[targetSquare];

            // Check if the target square is empty or occupied by an opponent's
            // piece
            if (!Piece::matchesColor(piece, pieceOnTargetSquare)) {
                moves.push_back(Move(startSquare, targetSquare));
            }
        }
    }
    void generateKingMoves(int startSquare) {
        for (int directionIndex = 0; directionIndex < 8; directionIndex++) {
            for (int n = 0;
                 n < min(numSquaresToEdge[startSquare][directionIndex], 1);
                 n++) {
                int targetSquare =
                    startSquare + DIRECTION_OFFSETS[directionIndex] * (n + 1);
                int pieceOnTargetSquare = square[targetSquare];

                // Blocked by piece of same color
                if (Piece::matchesColor(colorTurn, pieceOnTargetSquare)) {
                    break;
                }
                this->moves.push_back(Move(startSquare, targetSquare));
            }
        }
    }
    void generatePawnMoves(int startSquare) {
        int forwardDirection = (colorTurn == 1) ? -8 : 8;
        int startRank = (colorTurn == 1) ? 1 : 6;

        // Determine forward moves
        int oneForwardSquare = startSquare + forwardDirection;
        if (square[oneForwardSquare] == 0) {
            moves.push_back(Move(startSquare, oneForwardSquare));
            int twoForwardSquare = startSquare + (-16 * colorTurn);

            // move 2 forward on starting rank rule
            if (square[twoForwardSquare] == 0 &&
                (colorTurn == -1 && numSquaresToEdge[startSquare][0] == 6 ||
                 colorTurn == 1 && numSquaresToEdge[startSquare][0] == 1)) {
                moves.push_back(Move(startSquare, twoForwardSquare));
            }
        }
        // Capturing

        // Check if pawn is not on the A file (left edge) for left capture
        for (int direction : {-1, 1}) {  // Check both left and right diagonals
            int targetSquare = startSquare + forwardDirection + direction;
            if (targetSquare >= 0 && targetSquare < 64 &&
                abs((startSquare % 8) - (targetSquare % 8)) == 1) {
                // Normal capture
                if (square[targetSquare] != Piece::Empty &&
                    !Piece::matchesColor(square[targetSquare], colorTurn)) {
                    moves.push_back(Move(startSquare, targetSquare));
                }
                // En passant capture
                if (targetSquare == enPassantTarget) {
                    moves.push_back(Move(startSquare, targetSquare, true));
                }
            }
        }
    }
};

#endif