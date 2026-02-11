#include "MinesweeperGame.hpp"

#include <random>

MinesweeperGame::MinesweeperGame(size_t width, size_t height, size_t numMines) {
    this->width = width;
    this->height = height;
    this->numMines = numMines;

    mines.resize(height);
    adjacentMines.resize(height);
    revealed.resize(height);
    flagged.resize(height);
    for (size_t i = 0; i < height; i++) {
        mines[i].resize(width, false);
        adjacentMines[i].resize(width, 0);
        revealed[i].resize(width, false);
        flagged[i].resize(width, false);
    }
}

void MinesweeperGame::revealAllMines() {
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            if (mines[i][j]) revealed[i][j] = true;
            else if (flagged[i][j]) {
                flagged[i][j] = false;
                minesLeft++;
            }
        }
    }
}

void MinesweeperGame::generateFrom(size_t x, size_t y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(0, width - 1);
    std::uniform_int_distribution<> distY(0, height - 1);

    size_t i = 0;
    MAIN:
    while (i < numMines) {
        size_t mineX = distX(gen);
        size_t mineY = distY(gen);

        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (mineX == nx && mineY == ny) goto MAIN;
            }
        }

        if (!mines[mineY][mineX]) {
            mines[mineY][mineX] = true;
            i++;
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (!mines[i][j]) continue;

            adjacentMines[i][j] = -1;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = i + dy;
                    int nx = j + dx;
                    if (ny < 0 || ny >= height || nx < 0 | nx >= width) continue;
                    if (adjacentMines[ny][nx] == -1) continue;
                    adjacentMines[ny][nx]++;
                }
            }
        }
    }

    generated = true;
    minesLeft = numMines;
}

GameState MinesweeperGame::reveal(size_t x, size_t y) {
    if (!generated) generateFrom(x, y);

    if (revealed[y][x] || flagged[y][x]) return gameState;
    revealed[y][x] = true;

    if (mines[y][x]) {
        gameState = GameState::Lost;
        revealAllMines();
        return gameState;
    }

    if (adjacentMines[y][x] == 0) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int ny = y + dy;
                int nx = x + dx;
                if (ny < 0 || ny >= height || nx < 0 | nx >= width) continue;
                reveal(nx, ny);
            }
        }
    }

    size_t revealedCount = 0;
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            if (revealed[i][j]) revealedCount++;
        }
    }

    if (revealedCount == width * height - numMines) {
        gameState = GameState::Won;
        revealAllMines();
    }

    return gameState;
}

void MinesweeperGame::toggleFlag(size_t x, size_t y) {
    if (!generated) return;
    if (revealed[y][x]) return;
    flagged[y][x] = !flagged[y][x];
    if (flagged[y][x]) minesLeft--;
    else minesLeft++;
}

std::pair<CellState, int> MinesweeperGame::getCell(size_t x, size_t y) const {
    if (flagged[y][x]) return { CellState::Flagged, 0 };
    if (!revealed[y][x]) return { CellState::Hidden, 0 };
    if (mines[y][x]) return { CellState::Mine, 0 };
    return { CellState::Revealed, adjacentMines[y][x] };
}

GameState MinesweeperGame::getGameState() const {
    return gameState;
}

int MinesweeperGame::getMinesLeft() const {
    return minesLeft;
}