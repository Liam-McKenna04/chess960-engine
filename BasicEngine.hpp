#ifndef BASIC_ENGINE_HPP
#define BASIC_ENGINE_HPP

#include "Engine.hpp"
#include "Board.hpp"
#include <unordered_map>
#include <limits>
#include <algorithm>

class BasicEngine : public Engine {
public:
    Move getBestMove(const Board& board) override {
        int depth = 5;  // You can adjust the depth as needed
        int alpha = std::numeric_limits<int>::min();
        int beta = std::numeric_limits<int>::max();
        int bestScore = alpha;
        Move bestMove;
        transpositionTable.clear();

        for (const Move& move : board.moves) {
            Board tempBoard = board;
            tempBoard.makeMove(move);
            int score = -alphaBeta(tempBoard, depth - 1, -beta, -alpha);
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
            alpha = std::max(alpha, bestScore);
        }

        return bestMove;
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
        // Assign high scores to captures, promotions, and checks
        if (board.getPieceAt(move.targetSquare) != -1) {
            score += 1000;  // Capture
        }
        if (move.promotionPiece != 0) {
            score += 800;  // Promotion
        }
        // Additional heuristics can be added
        return score;
    }

    int alphaBeta(Board& board, int depth, int alpha, int beta) {
        if (depth == 0 || board.isDraw() || board.isCheckmate()) {
            return evaluate(board);
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

        return score;
    }
};

#endif // BASIC_ENGINE_HPP
