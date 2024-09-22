#include "Board.hpp"

#include <bitset>
#include <iostream>
#include <random>

static const uint64_t FILE_A = 0x0101010101010101ULL;
static const uint64_t FILE_H = 0x8080808080808080ULL;

std::array<std::array<uint64_t, 64>, 12> zobristTable;
std::array<uint64_t, 4> zobristCastle;
std::array<uint64_t, 8> zobristEnPassant;
uint64_t zobristBlackToMove;

void initializeZobristTables() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    for (auto& pieceArray : zobristTable) {
        for (auto& value : pieceArray) {
            value = dis(gen);
        }
    }

    for (auto& value : zobristCastle) {
        value = dis(gen);
    }

    for (auto& value : zobristEnPassant) {
        value = dis(gen);
    }

    zobristBlackToMove = dis(gen);
}

Board::Board(const std::string& epd)
    : lastMove(-1, -1), enPassantTarget(-1), colorTurn(1), halfMoveClock(0) {
    bitboards.fill(0);
    whitePieces = 0;
    blackPieces = 0;
    allPieces = 0;

    // Parse the EPD string and set the bits in the bitboards
    int squareIndex = 56;  // Start from rank 8
    std::istringstream iss(epd);
    std::string token;
    canWhiteCastleKingside = true;
    canWhiteCastleQueenside = true;
    canBlackCastleKingside = true;
    canBlackCastleQueenside = true;

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

    positionHistory.push_back(boardToEPD());
    generateMoves();

    initializeZobristTables();
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

void Board::makeMove(const Move& move, bool updateMoves) {
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

    // Promotion
    if (move.promotionPiece != 0) {
        bitboards[move.promotionPiece] |= toBit;
    } else {
        bitboards[pieceIndex] |= toBit;
    }

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
    if (updateMoves) {
        lastMove = move;
    }

    // Castling
    if (pieceIndex == PieceType::WhiteKing || pieceIndex == PieceType::BlackKing) {
        if (std::abs(move.startSquare - move.targetSquare) == 2) {
            int rookStartSquare, rookTargetSquare;
            if (move.targetSquare > move.startSquare) {
                // Kingside castling
                rookStartSquare = move.startSquare + 3;
                rookTargetSquare = move.startSquare + 1;
            } else {
                // Queenside castling
                rookStartSquare = move.startSquare - 4;
                rookTargetSquare = move.startSquare - 1;
            }
            int rookPieceIndex = getPieceAt(rookStartSquare);
            bitboards[rookPieceIndex] &= ~(1ULL << rookStartSquare);
            bitboards[rookPieceIndex] |= (1ULL << rookTargetSquare);
        }
    }

    // Update castling rights
    if (pieceIndex == PieceType::WhiteKing) {
        canWhiteCastleKingside = false;
        canWhiteCastleQueenside = false;
    } else if (pieceIndex == PieceType::BlackKing) {
        canBlackCastleKingside = false;
        canBlackCastleQueenside = false;
    } else if (pieceIndex == PieceType::WhiteRook) {
        if (move.startSquare == 0) canWhiteCastleQueenside = false;
        if (move.startSquare == 7) canWhiteCastleKingside = false;
    } else if (pieceIndex == PieceType::BlackRook) {
        if (move.startSquare == 56) canBlackCastleQueenside = false;
        if (move.startSquare == 63) canBlackCastleKingside = false;
    }

    // Update halfMoveClock
    if (getPieceAt(move.targetSquare) != -1 || charToPieceIndex('P') == (getPieceAt(move.startSquare) % 6)) {
        halfMoveClock = 0;
    } else {
        halfMoveClock++;
    }

    // Update position history
    positionHistory.push_back(boardToEPD());

    if (updateMoves) {
        generateMoves();
    }
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

    std::vector<Move> legalMoves;
    for (const Move& move : moves) {
        if (isMoveLegal(move)) {
            legalMoves.push_back(move);
        }
    }
    moves = std::move(legalMoves);
}



// Knight move generation
void Board::generateKnightMoves(int knightPieceIndex) {
    uint64_t knights = bitboards[knightPieceIndex];
    uint64_t ownPieces = (colorTurn == 1) ? whitePieces : blackPieces;

    while (knights) {
        int square = __builtin_ctzll(knights);  // Get least significant bit index
        uint64_t knightBit = 1ULL << square;
        knights &= ~knightBit;  // Remove this knight from knights

        uint64_t attacks = knightAttackBitboard(square) & ~ownPieces;  // Exclude own pieces

        while (attacks) {
            int targetSquare = __builtin_ctzll(attacks);
            uint64_t targetBit = 1ULL << targetSquare;
            attacks &= ~targetBit;

            moves.emplace_back(square, targetSquare);
        }
    }
}

uint64_t Board::knightAttackBitboard(int square) const {
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
        int square = __builtin_ctzll(kings);
        uint64_t kingBit = 1ULL << square;
        kings &= ~kingBit;

        uint64_t attacks = kingAttackBitboard(square) & ~ownPieces;

        while (attacks) {
            int targetSquare = __builtin_ctzll(attacks);
            uint64_t targetBit = 1ULL << targetSquare;
            attacks &= ~targetBit;

            moves.emplace_back(square, targetSquare);
        }

        // Generate castling moves
        if (colorTurn == 1) {
            if (canWhiteCastleKingside && 
                !(allPieces & ((1ULL << 5) | (1ULL << 6)))) {
                moves.emplace_back(4, 6, false, 0, true);
            }
            if (canWhiteCastleQueenside &&
                !(allPieces & ((1ULL << 3) | (1ULL << 2) | (1ULL << 1)))) {
                moves.emplace_back(4, 2, false, 0, true);
            }
        } else {
            if (canBlackCastleKingside && 
                !(allPieces & ((1ULL << 61) | (1ULL << 62)))) {
                moves.emplace_back(60, 62, false, 0, true);
            }
            if (canBlackCastleQueenside && 
                !(allPieces & ((1ULL << 59) | (1ULL << 58) | (1ULL << 57)))) {
                moves.emplace_back(60, 58, false, 0, true);
            }
        }
    }
}
uint64_t Board::kingAttackBitboard(int square) const {
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


void Board::generatePawnMoves(int pawnPieceIndex) {
    uint64_t pawns = bitboards[pawnPieceIndex];

    // For each pawn
    while (pawns) {
        int square = __builtin_ctzll(pawns);  // Get least significant bit index
        uint64_t pawnBit = 1ULL << square;
        pawns &= ~pawnBit;  // Remove this pawn from pawns

        if (colorTurn == 1) {
            // White pawn moves one square forward
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
            // Captures diagonal left
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
            // Black pawn moves one square forward
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

bool Board::isSquareAttacked(int square, int attackingColor) const {
    uint64_t targetBit = 1ULL << square;
    uint64_t opponentPawns = (attackingColor == 1) ? bitboards[PieceType::WhitePawn] : bitboards[PieceType::BlackPawn];
    uint64_t opponentKnights = (attackingColor == 1) ? bitboards[PieceType::WhiteKnight] : bitboards[PieceType::BlackKnight];
    uint64_t opponentBishops = (attackingColor == 1) ? bitboards[PieceType::WhiteBishop] : bitboards[PieceType::BlackBishop];
    uint64_t opponentRooks = (attackingColor == 1) ? bitboards[PieceType::WhiteRook] : bitboards[PieceType::BlackRook];
    uint64_t opponentQueens = (attackingColor == 1) ? bitboards[PieceType::WhiteQueen] : bitboards[PieceType::BlackQueen];
    uint64_t opponentKing = (attackingColor == 1) ? bitboards[PieceType::WhiteKing] : bitboards[PieceType::BlackKing];

    // Pawn attacks
    if (attackingColor == 1) {  // White pawn attacks
        uint64_t pawnAttacks = ((opponentPawns & ~FILE_H) << 9) | ((opponentPawns & ~FILE_A) << 7);
        if (pawnAttacks & targetBit) return true;
    } else {  // Black pawn attacks
        uint64_t pawnAttacks = ((opponentPawns & ~FILE_A) >> 9) | ((opponentPawns & ~FILE_H) >> 7);
        if (pawnAttacks & targetBit) return true;
    }

    // Knight attacks
    uint64_t knightAttackers = 0ULL;
    uint64_t knights = opponentKnights;
    while (knights) {
        int knightSquare = __builtin_ctzll(knights);
        knights &= ~(1ULL << knightSquare);
        knightAttackers |= knightAttackBitboard(knightSquare);
    }
    if (knightAttackers & targetBit) return true;

    // King attacks
    uint64_t kingAttackers = 0ULL;
    uint64_t kings = opponentKing;
    while (kings) {
        int kingSquare = __builtin_ctzll(kings);
        kings &= ~(1ULL << kingSquare);
        kingAttackers |= kingAttackBitboard(kingSquare);
    }
    if (kingAttackers & targetBit) return true;

    // Bishop and Queen attacks
    uint64_t bishopAttackers = opponentBishops | opponentQueens;
    if (bishopAttackBitboard(square, allPieces) & bishopAttackers) return true;

    // Rook and Queen attacks
    uint64_t rookAttackers = opponentRooks | opponentQueens;
    if (rookAttackBitboard(square, allPieces) & rookAttackers) return true;

    return false;
}

uint64_t Board::bishopAttackBitboard(int square, uint64_t occupancy) const {
    uint64_t attacks = 0ULL;
    static const int bishopOffsets[4] = {9, 7, -7, -9};

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
            attacks |= targetBit;

            if (occupancy & targetBit) break;  // Blocked
        }
    }
    return attacks;
}

uint64_t Board::rookAttackBitboard(int square, uint64_t occupancy) const {
    uint64_t attacks = 0ULL;
    static const int rookOffsets[4] = {8, -8, 1, -1};

    for (int offset : rookOffsets) {
        int targetSquare = square;
        while (true) {
            int fromFile = targetSquare % 8;
            int fromRank = targetSquare / 8;

            targetSquare += offset;
            int toFile = targetSquare % 8;
            int toRank = targetSquare / 8;

            if (targetSquare < 0 || targetSquare >= 64 ||
                ((offset == 1 || offset == -1) && (fromRank != toRank)))
                break;

            uint64_t targetBit = 1ULL << targetSquare;
            attacks |= targetBit;

            if (occupancy & targetBit) break;  // Blocked
        }
    }
    return attacks;
}

bool Board::isStalemate() const {
    if (isKingInCheck(colorTurn)) {
        return false;
    }
    return moves.empty();
}

bool Board::isThreefoldRepetition() const {
    if (positionHistory.size() < 8) {
        return false;
    }
    
    std::string currentPosition = boardToEPD();
    int repetitionCount = 0;
    
    for (const auto& position : positionHistory) {
        if (position == currentPosition) {
            repetitionCount++;
            if (repetitionCount >= 3) {
                return true;
            }
        }
    }
    
    return false;
}

bool Board::isFiftyMoveRule() const {
    return halfMoveClock >= 100;
}

bool Board::isDraw() const {
    return isStalemate() || isThreefoldRepetition() || isFiftyMoveRule();
}

bool Board::isMoveLegal(const Move& move) {
    // Slow way of checking if a move is legal, in the future we will not check if the move is 
    //legal by making the move and then undoing it, instead we will use a more sophisticated algorithm


    // Save the current state
    auto savedBitboards = bitboards;
    auto savedWhitePieces = whitePieces;
    auto savedBlackPieces = blackPieces;
    auto savedAllPieces = allPieces;
    int savedEnPassantTarget = enPassantTarget;
    bool savedCanWhiteCastleKingside = canWhiteCastleKingside;
    bool savedCanWhiteCastleQueenside = canWhiteCastleQueenside;
    bool savedCanBlackCastleKingside = canBlackCastleKingside;
    bool savedCanBlackCastleQueenside = canBlackCastleQueenside;
    int savedColorTurn = colorTurn;

    // Make the move
    makeMove(move, false);

    // After makeMove, colorTurn is flipped
    int kingPieceIndex = (savedColorTurn == 1) ? PieceType::WhiteKing : PieceType::BlackKing;
    uint64_t kingBits = bitboards[kingPieceIndex];
    bool inCheck = false;
    if (kingBits) {
        int kingSquare = __builtin_ctzll(kingBits);
        inCheck = isSquareAttacked(kingSquare, -savedColorTurn);
    }

    // Revert the move
    bitboards = savedBitboards;
    whitePieces = savedWhitePieces;
    blackPieces = savedBlackPieces;
    allPieces = savedAllPieces;
    enPassantTarget = savedEnPassantTarget;
    canWhiteCastleKingside = savedCanWhiteCastleKingside;
    canWhiteCastleQueenside = savedCanWhiteCastleQueenside;
    canBlackCastleKingside = savedCanBlackCastleKingside;
    canBlackCastleQueenside = savedCanBlackCastleQueenside;
    colorTurn = savedColorTurn;

    return !inCheck;
}

bool Board::isKingInCheck(int color) const {
    int kingPieceIndex = (color == 1) ? PieceType::WhiteKing : PieceType::BlackKing;
    uint64_t kingBits = bitboards[kingPieceIndex];
    if (kingBits) {
        int kingSquare = __builtin_ctzll(kingBits);
        return isSquareAttacked(kingSquare, -color);
    }
    return false;  // King is not on the board (should not happen)
}

bool Board::isCheckmate() {
    return moves.empty() && isKingInCheck(colorTurn);
}

uint64_t Board::computeHash() const {
    uint64_t hash = 0ULL;

    // Using random numbers for each piece on each square
    for (int i = 0; i < 12; ++i) {
        uint64_t pieces = bitboards[i];
        while (pieces) {
            int square = __builtin_ctzll(pieces);
            pieces &= ~(1ULL << square);
            hash ^= zobristTable[i][square];
        }
    }

    if (canWhiteCastleKingside) hash ^= zobristCastle[0];
    if (canWhiteCastleQueenside) hash ^= zobristCastle[1];
    if (canBlackCastleKingside) hash ^= zobristCastle[2];
    if (canBlackCastleQueenside) hash ^= zobristCastle[3];
    if (enPassantTarget != -1) hash ^= zobristEnPassant[enPassantTarget % 8];
    if (colorTurn == -1) hash ^= zobristBlackToMove;

    return hash;
}
