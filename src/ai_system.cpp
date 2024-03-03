// internal
#include "ai_system.hpp"
#include "vector"
#include <level_manager.hpp>

const int gridSize = 30;

const int gridWidth = window_width_px / gridSize;

const int gridHeight = window_height_px / gridSize;

int grid[gridHeight][gridWidth];

void AISystem::step(float elapsed_ms)
{

}

void AISystem::init() {
    for (int i = 0; i < gridHeight; i++) {
        for (int j = 0; j < gridWidth; j++) {
            grid[i][j] = 0;
        }
    };
}

void AISystem::updateGrid(std::vector<initWall> walls) {
    init();
    for (initWall w : walls) {
        int left = (w.x - w.xSize / 2) / gridSize;
        int top = (w.y - w.ySize / 2) / gridSize;
        int right = (w.x + w.xSize / 2) / gridSize;
        int bottom = (w.x + w.ySize / 2) / gridSize;
        if (bottom >= gridHeight) {
            bottom = gridHeight - 1;
        }
        for (left; left <= right; left++) {
            for (top; top <= bottom; top++) {
                grid[top][left] = 1;
            }
             top = (w.y - w.ySize / 2) / gridSize;
        }
    }
}

void AISystem::printGrid() {
    printf("Chunk:\n");

    for (int r = 0; r < gridHeight; r++) {
        for (int c = 0; c < gridWidth; c++) {
            printf(" %x ", grid[r][c]);
        }
        printf("\n");
    }
    printf("--------\n");
}

struct CompareNodes {
    bool operator()(const Node* a, const Node* b) {
        return a->f() > b->f();
    }
};

int manhattanDistance(int x1, int y1, int x2, int y2) {
    return std::abs(x2 - x1) + std::abs(y2 - y1);
}

std::vector<std::pair<int, int>> AISystem::bestPath(Motion& eMotion, Motion& pMotion) {
    int startX = ceil(eMotion.position.x / gridSize) - 1;
    int startY = ceil(eMotion.position.y / gridSize) - 1;
    int targetX = ceil(pMotion.position.x / gridSize) - 1;
    int targetY = ceil(pMotion.position.y / gridSize) - 1;
    const int dx[4] = { 1, 0, -1, 0 };
    const int dy[4] = { 0, 1, 0, -1 };

    std::vector<std::pair<int, int>> path;
    int rows = gridHeight;
    int cols = gridWidth;

    std::vector<std::vector<bool>> closedList(rows, std::vector<bool>(cols, false));
    std::priority_queue<Node*, std::vector<Node*>, CompareNodes> openList;

    Node* start = new Node(startX, startY, 0, manhattanDistance(startX, startY, targetX, targetY), nullptr);
    openList.push(start);

    while (!openList.empty()) {
        Node* current = openList.top();
        openList.pop();
        if (current->x == targetX && current->y == targetY) {
            while (current != nullptr) {
                path.push_back({ current->x, current->y });
                current = current->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        closedList[current->y][current->x] = true;

        for (int i = 0; i < 4; ++i) {
            int nextX = current->x + dx[i];
            int nextY = current->y + dy[i];
            if (nextX >= 0 && nextX < cols && nextY >= 0 && nextY < rows && grid[nextY][nextX] == 0 && !closedList[nextY][nextX]) {
                int g = current->g + 1;
                int h = manhattanDistance(nextX, nextY, targetX, targetY);
                Node* next = new Node(nextX, nextY, g, h, current);
                openList.push(next);
            }
        }
    }
    return path;
}