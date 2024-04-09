// internal
#include "ai_system.hpp"
#include "vector"
#include <level_manager.hpp>
#include <iostream>

const int gridSize = 30;

const int gridWidth = window_width_px / gridSize;

const int gridHeight = window_height_px / gridSize;

int grid[gridHeight][gridWidth];


void AISystem::init() {
    for (int i = 0; i < gridHeight; i++) {
        for (int j = 0; j < gridWidth; j++) {
            grid[i][j] = 0;
        }
    };
}

void AISystem::updateGrid(std::vector<InitWall> walls) {
    init();
    for (InitWall w : walls) {
        int left = (w.x - w.xSize / 2) / gridSize;
        int top = (w.y - w.ySize / 2) / gridSize;
        int right = (w.x + w.xSize / 2) / gridSize;
        int bottom = (w.y + w.ySize / 2) / gridSize;
        if (bottom >= gridHeight) {
            bottom = gridHeight - 1;
        }
        if (left <= 0) {
            left = 0;
        }
        if (right >= gridWidth) {
            right = gridWidth - 1;
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

    if (grid[targetY][targetX] == 1 || targetY >= gridHeight) {
        return {};
    }

    /*printf("still calculating?");*/

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
    while (!openList.empty()) {
        Node* temp = openList.top();
        openList.pop();
        delete temp;
    }

    return path;
}


vec2 calculateControlPoint(const glm::vec2& start, const glm::vec2& end, float height) {
    glm::vec2 midPoint = (start + end) / 2.0f;
    midPoint.y -= height; // Adjust the height to control the arc's steepness
    return midPoint;
}

AISystem::AISystem() {
    createAllDecisionTrees();
}

void AISystem::createAllDecisionTrees() {
    //boulder AI
    DecisionNode* chasePlayer = new DecisionNode{"chasePlayer", {}, nullptr, nullptr};
    DecisionNode* canSeePlayer = new DecisionNode{"canSeePlayer", {}, chasePlayer, nullptr };
    decisionTreeMap["boulder"] = canSeePlayer;

    //Paint Can AI

    DecisionNode* runFromPlayer = new DecisionNode{ "runFromPlayer", {}, nullptr, nullptr };
    DecisionNode* moveOnPlatform = new DecisionNode{ "moveOnPlatform", {},nullptr, nullptr };
    DecisionNode* playerTooClose = new DecisionNode{ "playerTooClose", {}, runFromPlayer, moveOnPlatform };
    decisionTreeMap["paintCan"] = playerTooClose;
    
    // Archer AI
    DecisionNode* shootProjectile = new DecisionNode{ "shootProjectile", {}, nullptr, nullptr };
    DecisionNode* playerInSight = new DecisionNode{ "playerInSight", {}, shootProjectile, nullptr };
    DecisionNode* patrolPlatform = new DecisionNode{ "patrolPlatform", {}, playerInSight, nullptr };
    decisionTreeMap["archer"] = patrolPlatform;

}

bool AISystem::boulderDecisionTreeSwitch(std::string choice, Entity& boulderEntity, const vec2& playerPosition) {
    auto& motions_registry = registry.motions;
    Motion& boulderMotion = motions_registry.get(boulderEntity);

    //top and bottom of the boulder
    vec2 boulderPositionTop = boulderMotion.position - vec2(0, boulderMotion.scale.y / 2.0f);
    vec2 boulderPositionBottom = boulderMotion.position + vec2(0, boulderMotion.scale.y / 2.0f);

    vec2 playerHeadPosition = playerPosition - vec2(0, boulderMotion.scale.y / 2.0f);
    if (choice == "canSeePlayer") {
        bool lineOfSightTop = hasLineOfSight(boulderPositionTop, playerHeadPosition);
        bool lineOfSightBottom = hasLineOfSight(boulderPositionBottom, playerHeadPosition);
        return lineOfSightTop || lineOfSightBottom;
    }
    else if (choice == "chasePlayer") {
        bool lineOfSightTop = hasLineOfSight(boulderPositionTop, playerHeadPosition);
        bool lineOfSightBottom = hasLineOfSight(boulderPositionBottom, playerHeadPosition);

        // Only move towards the player if there is a clear line of sight
        if (lineOfSightTop || lineOfSightBottom) {
            vec2 toPlayer = normalize(playerPosition - boulderMotion.position);
            vec2 targetVelocity = toPlayer * 200.f;

            // Move the boulder towards the player using linear interpolation
            boulderMotion.position.x = lerp<float>(boulderMotion.position.x, playerPosition.x, 0.002f);
            boulderMotion.velocity.x = lerp<float>(boulderMotion.velocity.x, targetVelocity.x, 0.2f);
        }
    }
    return true;
}

bool AISystem::paintCanDecisionTree(std::string choice, Entity& paintCanEntity, const vec2& playerPosition) {
    auto& paintCanRegistry = registry.paintCans;
    auto& platformContainer = registry.platforms;
    auto& motionRegistry = registry.motions;
    float safeDistance = 150.f;
    bool playerIsClose;
    Motion& paintCanMotion = motionRegistry.get(paintCanEntity);
    if (paintCanMotion.grounded) {
        if (paintCanMotion.velocity.x == 0) {
            paintCanMotion.velocity.x = (rand() % 2) == 0 ? -200.f : 200.f;
        }
    }
    if (choice == "playerTooClose") {
        playerIsClose = glm::distance(paintCanMotion.position, playerPosition) < safeDistance;
        // Move away from player if too close, ignoring platform edges
        return playerIsClose;
    }
    else if (choice == "runFromPlayer") {
        if (paintCanMotion.position.x < playerPosition.x) {
            paintCanMotion.velocity.x = -200.f;
        }
        else {
            paintCanMotion.velocity.x = 200.f;
        }
        if (checkLineCollision(paintCanMotion)) {
            paintCanMotion.velocity.x *= -1;
        }
    }
    else if (choice == "moveOnPlatform") {
        for (auto& platformEntity : platformContainer.entities) {
            Motion& platformMotion = motionRegistry.get(platformEntity);
            if (rectangleCollides(paintCanMotion, platformMotion)) {
                double platformLeftEdge = platformMotion.position.x - (platformMotion.scale.x / 2.0);
                double platformRightEdge = platformMotion.position.x + (platformMotion.scale.x / 2.0);

                // Check and reverse direction at platform edges 
                if (paintCanMotion.position.x <= platformLeftEdge + paintCanMotion.scale.x ||
                    paintCanMotion.position.x >= platformRightEdge - paintCanMotion.scale.x) {
                    paintCanMotion.velocity.x *= -1;
                }
                if (checkLineCollision(paintCanMotion)) {
                    paintCanMotion.velocity.x *= -1;
                }
                break;
            }
        }
    }
    return true;
}

bool AISystem::archerDecisionTree(std::string choice, Entity& archerEntity, const vec2& playerPosition, Motion& playerMotion) {
	auto& archerRegistry = registry.archers;
	auto& platformContainer = registry.platforms;
	auto& motionRegistry = registry.motions;
	float safeDistance = 4000.f;
	bool playerIsClose;
	Motion& archerMotion = motionRegistry.get(archerEntity);
    if (archerMotion.grounded) {
        if (archerMotion.velocity.x == 0) {
			archerMotion.velocity.x = (rand() % 2) == 0 ? -200.f : 200.f;
		}
        if (archerMotion.velocity.x > 0) {
            archerMotion.scale.x = abs(archerMotion.scale.x);
        }
        else {
            archerMotion.scale.x = -abs(archerMotion.scale.x);
        }
	}
    if (choice == "playerInSight") {
		playerIsClose = glm::distance(archerMotion.position, playerPosition) < safeDistance;
		return playerIsClose;
	}
    else if (choice == "shootProjectile") {
        auto& cooldownTimer = registry.arrowCooldowns.get(archerEntity);
        if (cooldownTimer.timeSinceLastShot >= cooldownTimer.cooldown) {
            vec2 startPosition = archerMotion.position + glm::vec2(0.f, -200);
            auto entity = Entity();
            auto& motion = registry.motions.emplace(entity);
            motion.position = startPosition;
            motion.scale = { 50.f, 50.f };
            // the projectile follows a bezier curve, so ignore gravity
            motion.gravityScale = 0.f;

            BezierProjectile& projectile = registry.projectiles.emplace(entity);
            projectile.targetPosition = {playerPosition.x - playerMotion.scale.x * 2, playerPosition.y + 100};
            projectile.elapsedTime = 0.0f;
            glm::vec2 endPosition = projectile.targetPosition;
            float distance = glm::distance(startPosition, endPosition);
            float arcHeight = distance * 0.5;
            projectile.controlPoint = calculateControlPoint(startPosition, endPosition, arcHeight);
            projectile.startPosition = startPosition;

            registry.deadlys.emplace(entity);
            registry.renderRequests.insert(
                entity,
                { TEXTURE_ASSET_ID::BEZIERPROJECTILE,
                         EFFECT_ASSET_ID::TEXTURED,
                         GEOMETRY_BUFFER_ID::SPRITE });
            cooldownTimer.timeSinceLastShot = 0.f;
        }
	}

    else if (choice == "patrolPlatform") {
        for (auto& platformEntity : platformContainer.entities) {
			Motion& platformMotion = motionRegistry.get(platformEntity);
            if (rectangleCollides(archerMotion, platformMotion)) {
				double platformLeftEdge = platformMotion.position.x - (platformMotion.scale.x / 2.0);
				double platformRightEdge = platformMotion.position.x + (platformMotion.scale.x / 2.0);


                double archerLeftEdge = archerMotion.position.x - abs(archerMotion.scale.x / 2.0);
                double archerRightEdge = archerMotion.position.x + abs(archerMotion.scale.x / 2.0);

                if (archerLeftEdge <= platformLeftEdge || archerRightEdge >= platformRightEdge || checkLineCollision(archerMotion)) {
                    archerMotion.velocity.x *= -1;
                    if ((archerMotion.velocity.x < 0 && archerMotion.scale.x > 0) ||
                        (archerMotion.velocity.x > 0 && archerMotion.scale.x < 0)) {
                        archerMotion.scale.x *= -1; // This should correctly flip the sprite
                    }
                }

				break;
			}
		}
	}
	return true;
}


void AISystem::step(float elapsed_ms) {
    elapsed_ms_since_last_update += elapsed_ms;

    const float update_frequency = 1.0f / 60.0f * 1000.0f; // Convert 60 FPS to milliseconds

    if (elapsed_ms_since_last_update < update_frequency) {
        return;
    }

    elapsed_ms_since_last_update = 0;
    auto& motions_registry = registry.motions;
    vec2 player_position;
    bool player_found = false;
    Entity player;

    for (uint i = 0; i < motions_registry.size(); i++) {
        Motion& motion = motions_registry.components[i];
        Entity entity = motions_registry.entities[i];

        if (registry.players.has(entity)) {
            player_position = motion.position;
            player_found = true;
            player = entity;
            break;
        }
    }

    if (player_found) {
        // Boulder AI
        auto& boulderRegistry = registry.boulders;
        for (auto& boulderEntity : boulderRegistry.entities) {
            DecisionNode* start = decisionTreeMap["boulder"];
            while (start != nullptr) {
                if (boulderDecisionTreeSwitch(start->condition, boulderEntity, player_position)) {
                    start = start->trueCase;
                }
                else {
                    start = start->falseCase;
                }
            }
        }
        // PaintCan AI
        auto& paintCanRegistry = registry.paintCans;
        auto& motionRegistry = registry.motions;

        for (auto& paintCanEntity : paintCanRegistry.entities) {
            Motion& paintCanMotion = motionRegistry.get(paintCanEntity);
            DecisionNode* start = decisionTreeMap["paintCan"];
            while (start != nullptr) {
                if (paintCanDecisionTree(start->condition, paintCanEntity, player_position)) {
                    start = start->trueCase;
                }
                else {
                    start = start->falseCase;
                }
            }
        }

        auto& archerRegistry = registry.archers;
        auto& playerMotion = motions_registry.get(player);
        for (auto& archerEntity : archerRegistry.entities) {
			DecisionNode* start = decisionTreeMap["archer"];
            while (start != nullptr) {
                if (archerDecisionTree(start->condition, archerEntity, player_position, playerMotion)) {
					start = start->trueCase;
				}
                else {
					start = start->falseCase;
				}
			}
		}
    }
}

bool AISystem::hasLineOfSight(const vec2& start, const vec2& end) {
    auto& motions_registry = registry.motions;

    // Check for intersection with platforms
    for (uint i = 0; i < motions_registry.size(); i++) {
        Motion& motion = motions_registry.components[i];
        Entity entity = motions_registry.entities[i];

        if (registry.platforms.has(entity)) {
            const vec2 platformPos = motion.position;
            const vec2 platformSize = motion.scale;
            const float platformLeft = platformPos.x - platformSize.x / 2.0f;
            const float platformRight = platformPos.x + platformSize.x / 2.0f;
            const float platformTop = platformPos.y + platformSize.y / 2.0f;
            const float platformBottom = platformPos.y - platformSize.y / 2.0f;

            // Check if the line intersects with the platform's bounding box
            if (line_intersects_box(start.x, start.y, end.x, end.y, platformLeft, platformBottom, platformRight, platformTop)) {
                return false;
            }
        }
    }

    // Check for intersection with drawn lines
    for (uint i = 0; i < motions_registry.size(); i++) {
        Motion& motion = motions_registry.components[i];
        Entity entity = motions_registry.entities[i];

        if (registry.drawnLines.has(entity)) {
            const DrawnLine& line = registry.drawnLines.get(entity);
            const vec2& line_p1 = registry.drawnPoints.get(line.p1).position;
            const vec2& line_p2 = registry.drawnPoints.get(line.p2).position;
            const float lineMinX = std::min(line_p1[0], line_p2[0]);
            const float lineMaxX = std::max(line_p1[0], line_p2[0]);
            const float lineMinY = std::min(line_p1[1], line_p2[1]);
            const float lineMaxY = std::max(line_p1[1], line_p2[1]);

            // Check if the line intersects with the drawn line's bounding box
            if (line_intersects_box(start.x, start.y, end.x, end.y, lineMinX, lineMinY, lineMaxX, lineMaxY)) {
                return false;
            }
        }
    }
    return true;
}

bool AISystem::line_intersects_box(float x1, float y1, float x2, float y2, float left, float bottom, float right, float top) {
    // Check if either endpoint of the line is inside the box
    if ((x1 >= left && x1 <= right && y1 >= bottom && y1 <= top) ||
        (x2 >= left && x2 <= right && y2 >= bottom && y2 <= top)) {
        return true;
    }

    float minX = std::min(x1, x2);
    float maxX = std::max(x1, x2);
    float minY = std::min(y1, y2);
    float maxY = std::max(y1, y2);

    if (maxX < left) {
        return false;
    }

    if (minX > right) {
        return false;
    }

    if (maxY < bottom) {
        return false;
    }

    if (minY > top) {
        return false;
    }

    return true;
}

bool AISystem::checkLineCollision(const Motion& collisionMotion) {
    auto& motions_registry = registry.motions;

    // Check for intersection with lines
    for (uint i = 0; i < motions_registry.size(); i++) {
        Motion& motion = motions_registry.components[i];
        Entity entity = motions_registry.entities[i];

        if (registry.drawnLines.has(entity)) {
            const DrawnLine& line = registry.drawnLines.get(entity);
            const vec2& line_p1 = registry.drawnPoints.get(line.p1).position;
            const vec2& line_p2 = registry.drawnPoints.get(line.p2).position;

            // Check if the PaintCan collides with the line
            if (line_intersects_box(collisionMotion.position.x - collisionMotion.scale.x / 2.0f,
                                    collisionMotion.position.y - collisionMotion.scale.y / 2.0f,
                                    collisionMotion.position.x + collisionMotion.scale.x / 2.0f,
                                    collisionMotion.position.y + collisionMotion.scale.y / 2.0f,
                                    std::min(line_p1[0], line_p2[0]),
                                    std::min(line_p1[1], line_p2[1]),
                                    std::max(line_p1[0], line_p2[0]),
                                    std::max(line_p1[1], line_p2[1]))) {
                return true;
            }
        }
    }
    return false; 
}

bool AISystem::rectangleCollides(const Motion& motion1, const Motion& motion2) {
    bool y_val = (motion1.position[1] - abs(motion1.scale.y) / 2.f) < (motion2.position[1] + abs(motion2.scale.y) / 2.f) &&
        (motion2.position[1] - abs(motion2.scale.y) / 2.f) < (motion1.position[1] + abs(motion1.scale.y) / 2.f);
    bool x_val = (motion1.position[0] - abs(motion1.scale.x / 2.f)) < (motion2.position[0] + abs(motion2.scale.x) / 2.f) &&
        (motion2.position[0] - abs(motion2.scale.x / 2.f) < (motion1.position[0] + abs(motion1.scale.x) / 2.f));
    return y_val && x_val;
}





