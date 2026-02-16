#include "MinesweeperGame.hpp"

#include <unordered_map>
#include <algorithm>
#include <ranges>

std::mt19937 MinesweeperGame::rng(std::random_device{}());

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
    const MinesweeperGame gameCopy = copy();

    std::uniform_int_distribution<> distX(0, width - 1);
    std::uniform_int_distribution<> distY(0, height - 1);

    size_t i = 0;
    MAIN:
    while (i < numMines) {
        size_t mineX = distX(rng);
        size_t mineY = distY(rng);

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
                    if (ny < 0 || ny >= height || nx < 0 || nx >= width) continue;
                    if (adjacentMines[ny][nx] == -1) continue;
                    adjacentMines[ny][nx]++;
                }
            }
        }
    }

    generated = true;
    minesLeft = numMines;

//    MinesweeperGame solveCopy = copy();
//    solveCopy.solve();
//    if (solveCopy.gameState != GameState::Won) {
//        *this = gameCopy;
//        generateFrom(x, y);
//    }
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
                if (ny < 0 || ny >= height || nx < 0 || nx >= width) continue;
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
    setFlag(x, y, !flagged[y][x]);
}

void MinesweeperGame::setFlag(size_t x, size_t y, bool flag) {
    if (!generated) return;
    if (revealed[y][x]) return;
    if (flagged[y][x] == flag) return;
    flagged[y][x] = flag;
    if (flag) minesLeft--;
    else minesLeft++;
}

std::pair<CellState, int> MinesweeperGame::getCell(size_t x, size_t y) const {
    if (flagged[y][x]) return { CellState::Flagged, 0 };
    if (!revealed[y][x]) return { CellState::Hidden, 0 };
    if (mines[y][x]) return { CellState::Mine, 0 };
    return { CellState::Revealed, adjacentMines[y][x] };
}

MinesweeperGame MinesweeperGame::copy() const {
    MinesweeperGame copy(width, height, numMines);
    copy.mines = mines;
    copy.adjacentMines = adjacentMines;
    copy.revealed = revealed;
    copy.flagged = flagged;
    copy.generated = generated;
    copy.minesLeft = minesLeft;
    copy.gameState = gameState;
    return copy;
}

void MinesweeperGame::solve() {
    if (!generated) return;

    std::vector<std::vector<int>> nearby;
    std::vector<std::vector<bool>> possibleLocations;
    std::unordered_map<Pos, std::pair<int, std::vector<Pos>>> mineRequirements;

    nearby.resize(height);
    possibleLocations.resize(height);
    for (size_t i = 0; i < height; i++) {
        nearby[i].resize(width, -1);
        possibleLocations[i].resize(width, false);
    }

    bool progress = true;

    constexpr size_t MAX_ITERATIONS = 10;
    size_t iterations = 0;
    while (progress && gameState == GameState::Ongoing) {
        progress = false;
        for (size_t y = 0; y < height; y++) {
            for (size_t x = 0; x < width; x++) {
                if (!revealed[y][x]) continue;
                int adjacent = adjacentMines[y][x];
                if (adjacent < 1) continue;

                int foundAdjacent = 0;
                std::vector<Pos> all;
                std::vector<Pos> unconfirmed;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int ny = y + dy;
                        int nx = x + dx;
                        if (ny < 0 || ny >= height || nx < 0 || nx >= width) continue;

                        if (revealed[ny][nx]) continue;
                        all.emplace_back(nx, ny);

                        if (flagged[ny][nx]) {
                            adjacent--;
                            continue;
                        }

                        foundAdjacent++;
                        unconfirmed.emplace_back(nx, ny);
                    }
                }
                nearby[y][x] = adjacent;
                mineRequirements[{x, y}] = {adjacent, unconfirmed};
                if (adjacent == 0) {
                    for (Pos pos: unconfirmed) {
                        reveal(pos.x, pos.y);
                        progress = true;
                    }
                } else if (foundAdjacent > adjacent) {
                    int actualAdjacent = adjacent;
                    int currentAdjacent = foundAdjacent;
                    std::vector<Pos> current = unconfirmed;

                    bool madeProgress = true;
                    while (madeProgress) {
                        madeProgress = false;
                        std::vector<std::pair<int, std::vector<Pos>>> applicableSubsets;

                        for (int dy = -2; dy <= 2; dy++) {
                            for (int dx = -2; dx <= 2; dx++) {
                                int ny = y + dy;
                                int nx = x + dx;
                                if (ny < 0 || ny >= height || nx < 0 || nx >= width) continue;

                                if (!revealed[ny][nx]) continue;

                                Pos key = {static_cast<size_t>(nx), static_cast<size_t>(ny)};
                                if (!mineRequirements.contains(key)) continue;

                                std::pair<int, std::vector<Pos>> cellRequirements = mineRequirements[key];
                                if (cellRequirements.second.empty()) continue;

                                bool isSubset = std::ranges::all_of(cellRequirements.second, [&](const Pos& reqPos) {
                                    return std::ranges::find(current, reqPos) != current.end();
                                });

                                if (isSubset) applicableSubsets.push_back(cellRequirements);
                            }
                        }

                        if (!applicableSubsets.empty()) {
                            madeProgress = true;
                            for (const auto& subset : applicableSubsets) {
                                actualAdjacent -= subset.first;
                                currentAdjacent -= static_cast<int>(subset.second.size());
                                for (const Pos &reqPos: subset.second) {
                                    auto it = std::ranges::find(current, reqPos);
                                    if (it != current.end()) {
                                        current.erase(it);
                                    }
                                }
                            }
                        }
                    }

                    if (currentAdjacent != foundAdjacent) {
                        if (currentAdjacent == 0) {
                            for (Pos pos: current) {
                                reveal(pos.x, pos.y);
                                progress = true;
                            }
                        } else if (currentAdjacent == actualAdjacent) {
                            for (Pos pos: current) {
                                setFlag(pos.x, pos.y, true);
                                progress = true;
                            }
                        }
                    }
                } else if (foundAdjacent == adjacent) {
                    for (Pos pos: unconfirmed) {
                        setFlag(pos.x, pos.y, true);
                        progress = true;
                    }
                } else {
                    for (Pos pos: unconfirmed) possibleLocations[pos.y][pos.x] = true;
                }
            }
        }
        if (!progress && iterations++ >= MAX_ITERATIONS) {
            progress = true;
        }
    }
}