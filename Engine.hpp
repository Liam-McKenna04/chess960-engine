#pragma once

#include "Board.hpp"
#include "Piece.hpp"
#include <vector>
#include <random>


class Engine {
public:
    virtual ~Engine() = default;
    virtual Move getBestMove(const Board& board) = 0;
};

