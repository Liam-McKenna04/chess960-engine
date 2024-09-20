#ifndef BASIC_ENGINE_HPP
#define BASIC_ENGINE_HPP

#include "Engine.hpp"
#include "Board.hpp"
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <chrono>

class BasicEngine : public Engine {
public:
    Move getBestMove(const Board& board) override {
        Move bestMove;
        int maxTime = 5000;  // Maximum thinking time in milliseconds
        auto startTime = std::chrono::high_resolution_clock::now();

        for (int depth = 1; depth <= 20; ++depth) {
            Move currentBestMove = searchAtDepth(board, depth);
            
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
            
            if (elapsedTime >= maxTime) {
                break;
            }
            
            bestMove = currentBestMove;
        }

        return bestMove;
    }

    Move searchAtDepth(const Board& board, int depth) {
        // Implement alpha-beta search at the given depth
        // ...
    }

private:
    struct TTEntry {
        int depth;
        int score;
        int flag;  // 0: Exact, -1: Alpha, 1: Beta
    };

    std::unordered_map<uint64_t, TTEntry> transpositionTable;
    void sortMoves(Board& board, std::vector<Move>& moves) {
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            int scoreA = moveOrderingHeuristic(board, a);
            int scoreB = moveOrderingHeuristic(board, b);
            return scoreA > scoreB;
        });
    }

    int moveOrderingHeuristic(Board& board, const Move& move) {
        int score = 0;
        Piece movingPiece = board.getPieceAt(move.sourceSquare);
        Piece capturedPiece = board.getPieceAt(move.targetSquare);

        // Prioritize captures based on MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
        if (capturedPiece != -1) {
            score += 10000 + (capturedPiece % 6) * 10 - (movingPiece % 6);
        }

        if (move.promotionPiece != 0) {
            score += 9000 + move.promotionPiece;  // Prioritize promotions
        }

        // Consider checks
        Board tempBoard = board;
        tempBoard.makeMove(move);
        if (tempBoard.isKingInCheck(board.colorTurn)) {
            score += 8000;
        }

        // Add history heuristic
        score += getHistoryScore(move);

        // Add killer move heuristic
        if (isKillerMove(move, board.plyCount)) {
            score += 7000;
        }

        return score;
    }

    int alphaBeta(Board& board, int depth, int alpha, int beta) {
        if (depth == 0) {
            return quiescence(board, alpha, beta);
        }

        uint64_t hash = board.computeHash();
        if (transpositionTable.count(hash)) {
            TTEntry entry = transpositionTable[hash];
            if (entry.depth >= depth) {
                if (entry.flag == 0) {
                    return entry.score;
                } else if (entry.flag == -1 && entry.score <= alpha) {
                    return alpha;
                } else if (entry.flag == 1 && entry.score >= beta) {
                    return beta;
                }
            }
        }

        int originalAlpha = alpha;
        int bestScore = std::numeric_limits<int>::min();

        board.generateMoves();
        if (board.moves.empty()) {
            if (board.isKingInCheck(board.colorTurn)) {
                return -100000 + depth;  // Checkmate detected
            } else {
                return 0;  // Stalemate detected
            }
        }
        sortMoves(board, board.moves);


        for (Move& move : board.moves) {
            Board childBoard = board;
            childBoard.makeMove(move);
            int score = -alphaBeta(childBoard, depth - 1, -beta, -alpha);
            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) {
                break;  // Beta cutoff
            }
        }

        TTEntry newEntry;
        newEntry.depth = depth;
        newEntry.score = bestScore;
        if (bestScore <= originalAlpha) {
            newEntry.flag = 1;  // Beta cutoff
        } else if (bestScore >= beta) {
            newEntry.flag = -1;  // Alpha cutoff
        } else {
            newEntry.flag = 0;  // Exact value
        }
        transpositionTable[hash] = newEntry;

        return bestScore;
    }

    int evaluate(const Board& board) const {
        static const int pieceValues[12] = {
            100, 320, 330, 500, 900, 20000,    // White pieces
            -100, -320, -330, -500, -900, -20000  // Black pieces
        };

        int score = 0;
        for (int i = 0; i < 12; ++i) {
            uint64_t pieces = board.bitboards[i];
            int pieceCount = __builtin_popcountll(pieces);
            score += pieceValues[i] * pieceCount;
        }

        // Add positional evaluation
        score += evaluatePosition(board);
        
        // Add pawn structure evaluation
        score += evaluatePawnStructure(board);
        
        // Add king safety evaluation
        score += evaluateKingSafety(board);

        // Consider the side to move
        score *= (board.colorTurn == WHITE ? 1 : -1);

        return score;
    }

    // Implement these new evaluation functions
    int evaluatePosition(const Board& board) const {
        // Evaluate piece positions using piece-square tables
        // ...
    }

    int evaluatePawnStructure(const Board& board) const {
        // Evaluate pawn chains, isolated pawns, doubled pawns, etc.
        // ...
    }

    int evaluateKingSafety(const Board& board) const {
        // Evaluate king safety, considering pawn shield, open files near the king, etc.
        // ...
    }

    int quiescence(Board& board, int alpha, int beta) {
        int standPat = evaluate(board);
        if (standPat >= beta) {
            return beta;
        }
        if (alpha < standPat) {
            alpha = standPat;
        }

        board.generateCaptures();
        sortMoves(board, board.captures);

        for (const Move& move : board.captures) {
            Board childBoard = board;
            childBoard.makeMove(move);
            int score = -quiescence(childBoard, -beta, -alpha);
            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }

        return alpha;
    }
};

#endif // BASIC_ENGINE_HPP
