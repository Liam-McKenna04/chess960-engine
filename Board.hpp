#ifndef BOARD_HPP
#define BOARD_HPP

#include <array>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "Piece.hpp"

using namespace std;
struct Move {
    Move(int startSquare, int targetSquare) {
        this->startSquare = startSquare;
        this->targetSquare = targetSquare;
    }
    int startSquare;
    int targetSquare;
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
    bool colorTurn = 1;  // positive 1 for white, -1 for black
    string castleStatus = "KkQq";
    int square[64];
    vector<Move> moves;

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
        for (int startSquare; startSquare < 64; startSquare++) {
            int piece = square[startSquare];
            if (Piece::matchesColor(piece, colorTurn)) {
                if (piece == Piece::Queen || piece == Piece::Bishop ||
                    piece == Piece::Rook) {
                    generateStraightMoves(startSquare, piece);
                }
                if (piece == Piece::Knight) {
                    generateKnightMoves(startSquare);
                }
            }
        }
    };

    void generateStraightMoves(int startSquare, int piece) {
        int startDirIndex = (piece == Piece::Bishop) ? 4 : 0;
        int endDirIndex = (piece == Piece::Rook) ? 4 : 8;

        cout << startDirIndex << endDirIndex << endl;
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
    void generatePawnMoves(int startSquare) {
        // Need to keep track of last move, I'll do that tomorrow :sleep:
    }
};

#endif