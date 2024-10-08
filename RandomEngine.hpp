#pragma once

#include "Engine.hpp"
#include <random>

class RandomEngine : public Engine {
private:
    std::mt19937 rng;

public:
    RandomEngine() : rng(std::random_device{}()) {}

    Move getBestMove(const Board& board) override {
        if (board.moves.empty()) {
            throw std::runtime_error("No legal moves available");
        }
        std::uniform_int_distribution<> dist(0, board.moves.size() - 1);
        return board.moves[dist(rng)];
    }
};