#include "Board.hpp"

#include <bitset>
#include <iostream>

Board::Board(const std::string& epd)
    : lastMove(-1, -1), enPassantTarget(-1), colorTurn(1) {
    // Initialize bitboards to zero
    bitboards.fill(0);
    whitePieces = 0;
    blackPieces = 0;
    allPieces = 0;

    // Parse the EPD string and set the bits in the bitboards
    int squareIndex = 56;  // Start from rank 8
    std::istringstream iss(epd);
    std::string token;

    while (std::getline(iss, token, '/')) {
        for (char c : token) {
            if (std::isdigit(c)) {
                squareIndex += c - '0';
            } else {
                int pieceIndex = charToPieceIndex(c);
                if (pieceIndex != -1 && squareIndex >= 0 && squareIndex < 64) {
                    uint64_t bit = 1ULL << squareIndex;  // Bitboard mapping
                    bitboards[pieceIndex] |= bit;
                    if (pieceIndex < 6) {
                        whitePieces |= bit;
                    } else {
                        blackPieces |= bit;
                    }
                    allPieces |= bit;
                    squareIndex++;
                } else {
                    throw std::runtime_error("Invalid piece character in EPD");
                }
            }
        }
        squareIndex -= 16;  // Move to the next rank
    }

    generateMoves();
}

int Board::charToPieceIndex(char c) const {
    switch (c) {
        case 'P':
            return PieceType::WhitePawn;
        case 'N':
            return PieceType::WhiteKnight;
        case 'B':
            return PieceType::WhiteBishop;
        case 'R':
            return PieceType::WhiteRook;
        case 'Q':
            return PieceType::WhiteQueen;
        case 'K':
            return PieceType::WhiteKing;
        case 'p':
            return PieceType::BlackPawn;
        case 'n':
            return PieceType::BlackKnight;
        case 'b':
            return PieceType::BlackBishop;
        case 'r':
            return PieceType::BlackRook;
        case 'q':
            return PieceType::BlackQueen;
        case 'k':
            return PieceType::BlackKing;
        default:
            return -1;  // Invalid character
    }
}

char Board::pieceIndexToChar(int pieceIndex) const {
    switch (pieceIndex) {
        case PieceType::WhitePawn:
            return 'P';
        case PieceType::WhiteKnight:
            return 'N';
        case PieceType::WhiteBishop:
            return 'B';
        case PieceType::WhiteRook:
            return 'R';
        case PieceType::WhiteQueen:
            return 'Q';
        case PieceType::WhiteKing:
            return 'K';
        case PieceType::BlackPawn:
            return 'p';
        case PieceType::BlackKnight:
            return 'n';
        case PieceType::BlackBishop:
            return 'b';
        case PieceType::BlackRook:
            return 'r';
        case PieceType::BlackQueen:
            return 'q';
        case PieceType::BlackKing:
            return 'k';
        default:
            return '?';
    }
}

int Board::getPieceAt(int square) const {
    uint64_t bit = 1ULL << square;
    for (int i = 0; i < 12; ++i) {
        if (bitboards[i] & bit) {
            return i;  // PieceType index
        }
    }
    return -1;  // Empty square
}

std::string Board::boardToEPD() const {
    std::ostringstream epd;
    int emptyCount = 0;

    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int squareIndex = rank * 8 + file;
            int pieceIndex = getPieceAt(squareIndex);
            if (pieceIndex == -1) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    epd << emptyCount;
                    emptyCount = 0;
                }
                epd << pieceIndexToChar(pieceIndex);
            }
        }
        if (emptyCount > 0) {
            epd << emptyCount;
            emptyCount = 0;
        }
        if (rank > 0) epd << '/';
    }

    return epd.str();
}

void Board::makeMove(const Move& move) {
    int pieceIndex = getPieceAt(move.startSquare);
    if (pieceIndex == -1) {
        throw std::runtime_error("No piece on start square");
    }

    uint64_t fromBit = 1ULL << move.startSquare;
    uint64_t toBit = 1ULL << move.targetSquare;

    // Remove piece from start square
    bitboards[pieceIndex] &= ~fromBit;

    // Handle capture
    for (int i = 0; i < 12; ++i) {
        if (i != pieceIndex && (bitboards[i] & toBit)) {
            // Capture
            bitboards[i] &= ~toBit;
            break;
        }
    }

    // Handle en passant capture
    if (move.isEnPassant) {
        int capturedPawnSquare =
            move.targetSquare + ((colorTurn == 1) ? -8 : 8);
        uint64_t capturedPawnBit = 1ULL << capturedPawnSquare;
        int capturedPawnIndex = (colorTurn == 1) ? PieceType::BlackPawn
                                                 : PieceType::WhitePawn;
        bitboards[capturedPawnIndex] &= ~capturedPawnBit;
    }

    // Handle promotion
    if (move.promotionPiece != 0) {
        bitboards[move.promotionPiece] |= toBit;
    } else {
        // Move piece to target square
        bitboards[pieceIndex] |= toBit;
    }

    // Update aggregate bitboards
    updateAggregateBitboards();

    // Update enPassantTarget
    if ((pieceIndex == PieceType::WhitePawn ||
         pieceIndex == PieceType::BlackPawn) &&
        std::abs(move.startSquare - move.targetSquare) == 16) {
        enPassantTarget = (move.startSquare + move.targetSquare) / 2;
    } else {
        enPassantTarget = -1;
    }

    colorTurn = -colorTurn;
    lastMove = move;

    generateMoves();
}

void Board::updateAggregateBitboards() {
    whitePieces = 0;
    blackPieces = 0;
    allPieces = 0;

    for (int i = 0; i < 6; ++i) {
        whitePieces |= bitboards[i];
    }
    for (int i = 6; i < 12; ++i) {
        blackPieces |= bitboards[i];
    }
    allPieces = whitePieces | blackPieces;
}

bool Board::isLastMoveTile(int tileIndex) const {
    return tileIndex == lastMove.startSquare ||
           tileIndex == lastMove.targetSquare;
}

void Board::generateMoves() {
    moves.clear();

    if (colorTurn == 1) {
        generatePawnMoves(PieceType::WhitePawn);
        generateKnightMoves(PieceType::WhiteKnight);
        generateBishopMoves(PieceType::WhiteBishop);
        generateRookMoves(PieceType::WhiteRook);
        generateQueenMoves(PieceType::WhiteQueen);
        generateKingMoves(PieceType::WhiteKing);
    } else {
        generatePawnMoves(PieceType::BlackPawn);
        generateKnightMoves(PieceType::BlackKnight);
        generateBishopMoves(PieceType::BlackBishop);
        generateRookMoves(PieceType::BlackRook);
        generateQueenMoves(PieceType::BlackQueen);
        generateKingMoves(PieceType::BlackKing);
    }
}

// Knight move generation
void Board::generateKnightMoves(int knightPieceIndex) {
    uint64_t knights = bitboards[knightPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;

    while (knights) {
        int square = __builtin_ctzll(knights);  // Get least significant bit index
        uint64_t knightBit = 1ULL << square;
        knights &= ~knightBit;  // Remove this knight from knights

        uint64_t attacks = knightAttacks(square) & ~ownPieces;  // Exclude own pieces

        while (attacks) {
            int targetSquare = __builtin_ctzll(attacks);
            uint64_t targetBit = 1ULL << targetSquare;
            attacks &= ~targetBit;

            moves.emplace_back(square, targetSquare);
        }
    }
}

uint64_t Board::knightAttacks(int square) const {
    static const int knightOffsets[8] = {17, 15, 10, 6, -6, -10, -15, -17};
    uint64_t attacks = 0ULL;
    int file = square % 8;
    int rank = square / 8;

    for (int offset : knightOffsets) {
        int targetSquare = square + offset;
        int targetFile = targetSquare % 8;
        int targetRank = targetSquare / 8;

        if (targetSquare >= 0 && targetSquare < 64 &&
            std::abs(file - targetFile) <= 2 && std::abs(rank - targetRank) <= 2 &&
            ((std::abs(file - targetFile) == 1 && std::abs(rank - targetRank) == 2) ||
             (std::abs(file - targetFile) == 2 && std::abs(rank - targetRank) == 1))) {
            attacks |= 1ULL << targetSquare;
        }
    }
    return attacks;
}

// King move generation
void Board::generateKingMoves(int kingPieceIndex) {
    uint64_t kings = bitboards[kingPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;

    while (kings) {
        int square = __builtin_ctzll(kings);  // Get least significant bit index
        uint64_t kingBit = 1ULL << square;
        kings &= ~kingBit;  // Remove this king from kings

        uint64_t attacks = kingAttacks(square) & ~ownPieces;  // Exclude own pieces

        while (attacks) {
            int targetSquare = __builtin_ctzll(attacks);
            uint64_t targetBit = 1ULL << targetSquare;
            attacks &= ~targetBit;

            moves.emplace_back(square, targetSquare);
        }
    }
}

uint64_t Board::kingAttacks(int square) const {
    static const int kingOffsets[8] = {8, 1, -1, -8, 9, 7, -7, -9};
    uint64_t attacks = 0ULL;
    int file = square % 8;
    int rank = square / 8;

    for (int offset : kingOffsets) {
        int targetSquare = square + offset;
        int targetFile = targetSquare % 8;
        int targetRank = targetSquare / 8;

        if (targetSquare >= 0 && targetSquare < 64 &&
            std::abs(file - targetFile) <= 1 && std::abs(rank - targetRank) <= 1) {
            attacks |= 1ULL << targetSquare;
        }
    }
    return attacks;
}

// Bishop move generation
void Board::generateBishopMoves(int bishopPieceIndex) {
    uint64_t bishops = bitboards[bishopPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;
    uint64_t opponentPieces = (colorTurn == 1) ? blackPieces : whitePieces;

    static const int bishopOffsets[4] = {9, 7, -7, -9};

    while (bishops) {
        int square = __builtin_ctzll(bishops);  // Get least significant bit index
        uint64_t bishopBit = 1ULL << square;
        bishops &= ~bishopBit;  // Remove this bishop from bishops

        for (int offset : bishopOffsets) {
            int targetSquare = square;
            while (true) {
                int fromFile = targetSquare % 8;
                int fromRank = targetSquare / 8;

                targetSquare += offset;
                int toFile = targetSquare % 8;
                int toRank = targetSquare / 8;

                if (targetSquare < 0 || targetSquare >= 64 ||
                    std::abs(fromFile - toFile) != 1 || std::abs(fromRank - toRank) != 1)
                    break;

                uint64_t targetBit = 1ULL << targetSquare;

                if (ownPieces & targetBit) {
                    break;  // Blocked by own piece
                }

                moves.emplace_back(square, targetSquare);

                if (opponentPieces & targetBit) {
                    break;  // Capture and stop
                }
            }
        }
    }
}

// Rook move generation
void Board::generateRookMoves(int rookPieceIndex) {
    uint64_t rooks = bitboards[rookPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;
    uint64_t opponentPieces = (colorTurn == 1) ? blackPieces : whitePieces;

    static const int rookOffsets[4] = {8, -8, 1, -1};

    while (rooks) {
        int square = __builtin_ctzll(rooks);  // Get least significant bit index
        uint64_t rookBit = 1ULL << square;
        rooks &= ~rookBit;  // Remove this rook from rooks

        for (int offset : rookOffsets) {
            int targetSquare = square;
            while (true) {
                int fromFile = targetSquare % 8;
                int fromRank = targetSquare / 8;

                targetSquare += offset;
                int toFile = targetSquare % 8;
                int toRank = targetSquare / 8;

                if (targetSquare < 0 || targetSquare >= 64 ||
                    (offset == 1 || offset == -1) && (fromRank != toRank))
                    break;

                if ((offset == 8 || offset == -8) && (fromFile != toFile))
                    break;

                uint64_t targetBit = 1ULL << targetSquare;

                if (ownPieces & targetBit) {
                    break;  // Blocked by own piece
                }

                moves.emplace_back(square, targetSquare);

                if (opponentPieces & targetBit) {
                    break;  // Capture and stop
                }
            }
        }
    }
}

// Queen move generation
void Board::generateQueenMoves(int queenPieceIndex) {
    uint64_t queens = bitboards[queenPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;
    uint64_t opponentPieces = (colorTurn == 1) ? blackPieces : whitePieces;

    static const int queenOffsets[8] = {8, -8, 1, -1, 9, 7, -7, -9};

    while (queens) {
        int square = __builtin_ctzll(queens);  // Get least significant bit index
        uint64_t queenBit = 1ULL << square;
        queens &= ~queenBit;  // Remove this queen from queens

        for (int offset : queenOffsets) {
            int targetSquare = square;
            while (true) {
                int fromFile = targetSquare % 8;
                int fromRank = targetSquare / 8;

                targetSquare += offset;
                int toFile = targetSquare % 8;
                int toRank = targetSquare / 8;

                if (targetSquare < 0 || targetSquare >= 64)
                    break;

                if ((offset == 1 || offset == -1) && (fromRank != toRank))
                    break;

                if ((offset == 8 || offset == -8) && (fromFile != toFile))
                    break;

                if ((offset == 9 || offset == -9 || offset == 7 || offset == -7) &&
                    (std::abs(fromFile - toFile) != 1 || std::abs(fromRank - toRank) != 1))
                    break;

                uint64_t targetBit = 1ULL << targetSquare;

                if (ownPieces & targetBit) {
                    break;  // Blocked by own piece
                }

                moves.emplace_back(square, targetSquare);

                if (opponentPieces & targetBit) {
                    break;  // Capture and stop
                }
            }
        }
    }
}

// Pawn move generation (already provided in previous code)
// The implementation remains the same as previously shown

// ... [Add the existing generatePawnMoves function here]


void Board::generatePawnMoves(int pawnPieceIndex) {
    uint64_t pawns = bitboards[pawnPieceIndex];

    // For each pawn
    while (pawns) {
        int square = __builtin_ctzll(pawns);  // Get least significant bit index
        uint64_t pawnBit = 1ULL << square;
        pawns &= ~pawnBit;  // Remove this pawn from pawns

        if (colorTurn == 1) {
            // White pawn moves
            // One square forward
            int forwardSquare = square + 8;
            uint64_t forwardBit = 1ULL << forwardSquare;
            if (!(allPieces & forwardBit)) {
                // Empty square ahead
                if (forwardSquare >= 56) {
                    // Promotion
                    addPawnPromotionMoves(square, forwardSquare);
                } else {
                    moves.emplace_back(square, forwardSquare);
                    // Two squares forward from rank 2
                    if (square >= 8 && square < 16) {
                        int doubleForwardSquare = square + 16;
                        uint64_t doubleForwardBit = 1ULL << doubleForwardSquare;
                        if (!(allPieces & doubleForwardBit)) {
                            moves.emplace_back(square, doubleForwardSquare);
                        }
                    }
                }
            }
            // Captures
            // Diagonal left
            if ((square % 8) != 0) {
                int captureSquare = square + 7;
                uint64_t captureBit = 1ULL << captureSquare;
                if (blackPieces & captureBit) {
                    if (captureSquare >= 56) {
                        addPawnPromotionMoves(square, captureSquare);
                    } else {
                        moves.emplace_back(square, captureSquare);
                    }
                } else if (captureSquare == enPassantTarget) {
                    moves.emplace_back(square, captureSquare, true);
                }
            }
            // Diagonal right
            if ((square % 8) != 7) {
                int captureSquare = square + 9;
                uint64_t captureBit = 1ULL << captureSquare;
                if (blackPieces & captureBit) {
                    if (captureSquare >= 56) {
                        addPawnPromotionMoves(square, captureSquare);
                    } else {
                        moves.emplace_back(square, captureSquare);
                    }
                } else if (captureSquare == enPassantTarget) {
                    moves.emplace_back(square, captureSquare, true);
                }
            }
        } else {
            // Black pawn moves
            // One square forward
            int forwardSquare = square - 8;
            uint64_t forwardBit = 1ULL << forwardSquare;
            if (!(allPieces & forwardBit)) {
                // Empty square ahead
                if (forwardSquare < 8) {
                    // Promotion
                    addPawnPromotionMoves(square, forwardSquare);
                } else {
                    moves.emplace_back(square, forwardSquare);
                    // Two squares forward from rank 7
                    if (square >= 48 && square < 56) {
                        int doubleForwardSquare = square - 16;
                        uint64_t doubleForwardBit = 1ULL << doubleForwardSquare;
                        if (!(allPieces & doubleForwardBit)) {
                            moves.emplace_back(square, doubleForwardSquare);
                        }
                    }
                }
            }
            // Captures
            // Diagonal left
            if ((square % 8) != 0) {
                int captureSquare = square - 9;
                uint64_t captureBit = 1ULL << captureSquare;
                if (whitePieces & captureBit) {
                    if (captureSquare < 8) {
                        addPawnPromotionMoves(square, captureSquare);
                    } else {
                        moves.emplace_back(square, captureSquare);
                    }
                } else if (captureSquare == enPassantTarget) {
                    moves.emplace_back(square, captureSquare, true);
                }
            }
            // Diagonal right
            if ((square % 8) != 7) {
                int captureSquare = square - 7;
                uint64_t captureBit = 1ULL << captureSquare;
                if (whitePieces & captureBit) {
                    if (captureSquare < 8) {
                        addPawnPromotionMoves(square, captureSquare);
                    } else {
                        moves.emplace_back(square, captureSquare);
                    }
                } else if (captureSquare == enPassantTarget) {
                    moves.emplace_back(square, captureSquare, true);
                }
            }
        }
    }
}

void Board::addPawnPromotionMoves(int startSquare, int targetSquare) {
    if (colorTurn == 1) {
        moves.emplace_back(startSquare, targetSquare, false, PieceType::WhiteQueen);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::WhiteRook);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::WhiteBishop);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::WhiteKnight);
    } else {
        moves.emplace_back(startSquare, targetSquare, false, PieceType::BlackQueen);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::BlackRook);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::BlackBishop);
        moves.emplace_back(startSquare, targetSquare, false, PieceType::BlackKnight);
    }
}

// Implement other piece move generations similarly
