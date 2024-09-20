#ifndef BASIC_ENGINE_HPP
#define BASIC_ENGINE_HPP

#include "Engine.hpp"
#include "Board.hpp"

class BasicEngine : public Engine {
public:
    Move getBestMove(const Board& board) override {
        // Simple evaluation: prefer captures and check moves
        int bestScore = -1000000;
        Move bestMove;

        for (const Move& move : board.moves) {
            int score = 0;
            
            // Prefer captures
            if (board.getPieceAt(move.targetSquare) != -1) {
                score += 10;
            }
            
            // Simulate the move
            Board tempBoard = board;
            tempBoard.makeMove(move);
            

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }

        return bestMove;
    }
};

#endif // BASIC_ENGINE_HPP