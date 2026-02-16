#pragma once

#include <vector>
#include <random>

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

struct Pos {
    size_t x;
    size_t y;

    bool operator==(const Pos& other) const {
        return x == other.x &&
            y == other.y;
    }
};

template <>
struct std::hash<Pos> {
    size_t operator()(const Pos& pos) const noexcept {
        size_t seed = 0;
        seed ^= std::hash<size_t>()(pos.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<size_t>()(pos.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

class MinesweeperGame {
    static std::mt19937 rng;

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

    void solve();

    [[nodiscard]] size_t getWidth() const { return width; }
    [[nodiscard]] size_t getHeight() const { return height; }
    [[nodiscard]] bool isGenerated() const { return generated; }
    [[nodiscard]] GameState getGameState() const { return gameState; }
    [[nodiscard]] int getMinesLeft() const { return minesLeft; }

    [[nodiscard]] std::pair<CellState, int> getCell(size_t x, size_t y) const;

    [[nodiscard]] MinesweeperGame copy() const;

    void setFlag(size_t x, size_t y, bool flag);
};