#pragma once

#include <vector>

enum class CellState {
    Hidden,
    Revealed,
    Flagged,
    Mine
};

enum class GameState {
    Ongoing,
    Won,
    Lost
};

class MinesweeperGame {
    size_t width;
    size_t height;

    size_t numMines;

    std::vector<std::vector<bool>> mines;
    std::vector<std::vector<int>> adjacentMines;
    std::vector<std::vector<bool>> revealed;
    std::vector<std::vector<bool>> flagged;

    bool generated = false;
    int minesLeft = 0;
    GameState gameState = GameState::Ongoing;

    void revealAllMines();

public:
    MinesweeperGame(size_t width, size_t height, size_t numMines);

    void generateFrom(size_t x, size_t y);
    GameState reveal(size_t x, size_t y);
    void toggleFlag(size_t x, size_t y);

    [[nodiscard]] std::pair<CellState, int> getCell(size_t x, size_t y) const;
    [[nodiscard]] GameState getGameState() const;
    [[nodiscard]] int getMinesLeft() const;
};