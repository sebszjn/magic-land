#pragma once
#include "level/maploader.h"
#include "level/levelmetrics.h"
#include "core/entities.h" // <--- ADICIONE ISSO
#include <vector>          // <--- ADICIONE ISSO

struct Level
{
    MapLoader map;
    LevelMetrics metrics;
    std::vector<Enemy> enemies;
    std::vector<Item> items;

    float exitX = 0.0f;
    float exitZ = 0.0f;
    bool hasExit = false;
};

bool loadLevel(Level &lvl, const char *mapPath, float tileSize);
void applySpawn(const Level &lvl, float &camX, float &camZ);
