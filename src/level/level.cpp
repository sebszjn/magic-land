#include "level/level.h"
#include "core/config.h"
#include <cstdio>

// Configurações básicas para spawn
static const float ENEMY_START_HP = 100.0f;

bool loadLevel(Level &lvl, const char *mapPath, float tileSize)
{
    // 1) Carrega o mapa de texto
    if (!lvl.map.load(mapPath))
    {
        std::printf("ERRO: nao foi possivel carregar o mapa: %s\n", mapPath);
        return false;
    }

    lvl.metrics = LevelMetrics::fromMap(lvl.map, tileSize);

    // Limpa entidades antigas
    lvl.enemies.clear();
    lvl.items.clear();

    // Reset da saída
    lvl.hasExit = false;
    lvl.exitX = 0.0f;
    lvl.exitZ = 0.0f;

    // 2) Escaneia o mapa procurando Entidades (J, T, M, G, K, H, A, X)
    const auto &data = lvl.map.data();
    int H = lvl.map.getHeight();

    for (int z = 0; z < H; z++)
    {
        const std::string &row = data[z];
        for (int x = 0; x < (int)row.size(); x++)
        {
            char c = row[x];
            float wx, wz;
            lvl.metrics.tileCenter(x, z, wx, wz);

            // --- Inimigos ---
            int enemyType = -1;
            if (c == 'G')
                enemyType = 0;
            else if (c == 'M')
                enemyType = 1;
            else if (c == 'N')
                enemyType = 2;
            else if (c == 'B')
                enemyType = 3;
            else if (c == 'T')
                enemyType = 4;

            if (enemyType != -1)
            {
                Enemy e;
                e.type = enemyType;

                e.x = wx;
                e.z = wz;

                e.startX = wx;
                e.startZ = wz;
                e.respawnTimer = 0.0f;

                e.hp = ENEMY_START_HP;
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f;

                lvl.enemies.push_back(e);
            }
            else if (c == 'H') // Health
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_HEALTH;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
            }
            else if (c == 'A') // Ammo
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_AMMO;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
            }
            else if (c == 'X') // SAÍDA
            {
                lvl.exitX = wx;
                lvl.exitZ = wz;
                lvl.hasExit = true;
            }
            else if (c == 'S') // SPECIAL pickup (fase 2)
            {
                Item i;
                i.x = wx;
                i.z = wz;
                i.type = ITEM_SPECIAL;
                i.active = true;
                i.respawnTimer = 0.0f;
                lvl.items.push_back(i);
            }
            else if (c == 'B') // BOSS
            {
                Enemy e;
                e.type = 4; // usa o inimigo 4 como base (muda se quiser)
                e.isBoss = true;

                e.x = wx;
                e.z = wz;
                e.startX = wx;
                e.startZ = wz;
                e.respawnTimer = 0.0f;

                e.hp = 320.0f;
                e.state = STATE_IDLE;
                e.animFrame = 0;
                e.animTimer = 0;
                e.hurtTimer = 0.0f;
                e.attackCooldown = 0.0f;

                lvl.enemies.push_back(e);
            }
        }
    }

    // Logs úteis
    if (!lvl.hasExit)
        std::printf("[LEVEL] AVISO: mapa %s NAO tem X (saida)\n", mapPath);
    else
        std::printf("[LEVEL] Saida X em (%.2f, %.2f) no mapa %s\n", lvl.exitX, lvl.exitZ, mapPath);

    std::printf("[LEVEL] Enemies=%d Items=%d\n", (int)lvl.enemies.size(), (int)lvl.items.size());

    return true;
}

void applySpawn(const Level &lvl, float &camX, float &camZ)
{
    lvl.metrics.spawnPos(lvl.map, camX, camZ);
}