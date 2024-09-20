#pragma once

#include "Engine.hpp"
#include <random>

class RandomEngine : public Engine {
public:
    Move getBestMove(const Board& board) override {
        std::vector<Move>& moves = board.moves;
        if (moves.empty()) {
            throw std::runtime_error("No legal moves available");
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, moves.size() - 1);
        return moves[dis(gen)];
    }
};