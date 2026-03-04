#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include <cmath>

static const float ENEMY_START_HP = 100.0f;
static const float BOSS_HP = 320.0f;

bool isWalkable(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];

    // paredes
    if (c == '1' || c == '2') return false;

    // 'X' (portal) é caminhável
    return true;
}

static void dropKeyAt(float x, float z)
{
    auto& lvl = gameLevel();

    Item k;
    k.type = ITEM_KEY;
    k.x = x;
    k.z = z;
    k.active = true;
    k.respawnTimer = 0.0f;

    lvl.items.push_back(k);
}

void updateEntities(float dt)
{
    auto& g = gameContext();
    auto& lvl = gameLevel();
    auto& audio = gameAudio();

    // ---------- inimigos ----------
    for (auto& en : lvl.enemies)
    {
        if (en.state == STATE_DEAD)
        {
            en.respawnTimer -= dt;
            if (en.respawnTimer <= 0.0f)
            {
                en.state = STATE_IDLE;

                en.hp = en.isBoss ? BOSS_HP : ENEMY_START_HP;

                en.x = en.startX;
                en.z = en.startZ;
                en.hurtTimer = 0.0f;
            }
            continue;
        }

        if (en.hurtTimer > 0.0f) en.hurtTimer -= dt;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < 0.0001f) dist = 0.0001f;

        // boss: um pouco mais agressivo
        float viewDist = en.isBoss ? (ENEMY_VIEW_DIST + 6.0f) : ENEMY_VIEW_DIST;
        float atkDist  = en.isBoss ? (ENEMY_ATTACK_DIST + 0.5f) : ENEMY_ATTACK_DIST;
        float speed    = en.isBoss ? (ENEMY_SPEED + 0.7f) : ENEMY_SPEED;

        switch (en.state)
        {
        case STATE_IDLE:
            if (dist < viewDist) en.state = STATE_CHASE;
            break;

        case STATE_CHASE:
            if (dist < atkDist)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.4f;
            }
            else if (dist > viewDist * 1.5f)
            {
                en.state = STATE_IDLE;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                float moveStep = speed * dt;

                float nextX = en.x + dirX * moveStep;
                if (isWalkable(nextX, en.z)) en.x = nextX;

                float nextZ = en.z + dirZ * moveStep;
                if (isWalkable(en.x, nextZ)) en.z = nextZ;
            }
            break;

        case STATE_ATTACK:
            if (dist > atkDist)
            {
                en.state = STATE_CHASE;
            }
            else
            {
                en.attackCooldown -= dt;
                if (en.attackCooldown <= 0.0f)
                {
                    g.player.health -= en.isBoss ? 18 : 10;
                    en.attackCooldown = en.isBoss ? 0.75f : 1.0f;

                    g.player.damageAlpha = 1.0f;
                    audioPlayHurt(audio);
                }
            }
            break;

        default:
            break;
        }
    }

    // ---------- itens ----------
    for (auto& item : lvl.items)
    {
        if (!item.active)
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f) item.active = true;
            continue;
        }

        float dx = camX - item.x;
        float dz = camZ - item.z;

        if (dx * dx + dz * dz < 1.0f)
        {
            item.active = false;

            if (item.type == ITEM_HEALTH)
            {
                item.respawnTimer = 15.0f;
                g.player.health += 50;
                if (g.player.health > 100) g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_AMMO)
            {
                item.respawnTimer = 999999.0f;
                g.player.reserveAmmo = 20;
            }
            else if (item.type == ITEM_KEY)
            {
                item.respawnTimer = 999999.0f;
                g.player.keys += 1; // NOVO
            }
            else if (item.type == ITEM_SPECIAL)
            {
                item.respawnTimer = 999999.0f;
                g.player.altUnlocked = true;  // NOVO
                g.player.altAmmo = 5;         // ganha 5 no pickup
            }
        }
    }

    // ---------- detecta mortes e dropa chave ----------
    // (faz depois do update pra capturar mortes por tiro)
    for (auto& en : lvl.enemies)
    {
        // se acabou de morrer, dropa chave 1 vez
        // usa respawnTimer == 0 como gatilho "acabou de morrer"
        if (en.state == STATE_DEAD && en.respawnTimer > 0.0f)
            continue;
    }

    // Melhor: você dropa a chave no exato momento que define STATE_DEAD (no playerTryAttack).
    // Aí não duplica. Eu vou te dar essa mudança no player.cpp abaixo.
}