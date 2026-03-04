#include "core/player.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include "core/entities.h"
#include <cmath>

constexpr int MAX_MAGAZINE = 12;

static constexpr float HIT_RADIUS = 0.55f;
static constexpr float MAX_RANGE  = 17.0f;

static bool rayCircleIntersectXZ(
    float ox, float oz,
    float dx, float dz, // normalizado
    float cx, float cz,
    float r,
    float &outT
)
{
    float fx = cx - ox;
    float fz = cz - oz;

    float proj = fx * dx + fz * dz;
    if (proj < 0.0f) return false;

    float px = ox + dx * proj;
    float pz = oz + dz * proj;

    float ex = cx - px;
    float ez = cz - pz;
    float d2 = ex * ex + ez * ez;

    float r2 = r * r;
    if (d2 > r2) return false;

    float thc = std::sqrt(r2 - d2);
    float t0 = proj - thc;
    float t1 = proj + thc;

    outT = (t0 >= 0.0f) ? t0 : t1;
    return outT >= 0.0f;
}

static void dropKeyAt(float x, float z)
{
    auto &lvl = gameLevel();

    Item k;
    k.type = ITEM_KEY;
    k.x = x;
    k.z = z;
    k.active = true;
    k.respawnTimer = 0.0f;

    lvl.items.push_back(k);
}

static void doShoot(int damage)
{
    auto &g = gameContext();
    auto &lvl = gameLevel();

    float radYaw = yaw * 3.14159f / 180.0f;
    float dirX = std::sin(radYaw);
    float dirZ = -std::cos(radYaw);

    float len = std::sqrt(dirX * dirX + dirZ * dirZ);
    if (len <= 0.0f) return;
    dirX /= len; dirZ /= len;

    int bestIdx = -1;
    float bestT = MAX_RANGE;

    for (int i = 0; i < (int)lvl.enemies.size(); ++i)
    {
        auto &en = lvl.enemies[i];
        if (en.state == STATE_DEAD) continue;

        float tHit = 0.0f;
        if (rayCircleIntersectXZ(camX, camZ, dirX, dirZ, en.x, en.z, HIT_RADIUS, tHit))
        {
            if (tHit <= bestT)
            {
                bestT = tHit;
                bestIdx = i;
            }
        }
    }

    if (bestIdx < 0) return;

    auto &en = lvl.enemies[bestIdx];
    en.hp -= damage;
    en.hurtTimer = 0.5f;

    if (en.hp <= 0)
    {
        en.state = STATE_DEAD;
        en.respawnTimer = en.isBoss ? 999999.0f : 15.0f; // boss não respawna

        // dropa 1 chave
        dropKeyAt(en.x, en.z);

        // opcional: drop de ammo
        Item drop;
        drop.type = ITEM_AMMO;
        drop.x = en.x;
        drop.z = en.z;
        drop.active = true;
        drop.respawnTimer = 0.0f;
        lvl.items.push_back(drop);
    }
}

void playerTryReload()
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    if (g.weapon.state != WeaponState::W_IDLE) return;
    if (g.player.currentAmmo >= MAX_MAGAZINE) return;
    if (g.player.reserveAmmo <= 0) return;

    g.weapon.state = WeaponState::W_RELOAD_1;
    g.weapon.timer = 0.50f;

    audioPlayReload(audio);
}

void playerTryAttack()
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    if (g.weapon.state != WeaponState::W_IDLE) return;
    if (g.player.currentAmmo <= 0) return;

    g.player.currentAmmo--;

    audioOnPlayerShot(audio);
    audioPlayShot(audio);

    g.weapon.state = WeaponState::W_FIRE_1;
    g.weapon.timer = 0.08f;

    doShoot(30);
}

void playerTryAltAttack()
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    if (!g.player.altUnlocked) return;
    if (g.player.altAmmo <= 0) return;
    if (g.weapon.state != WeaponState::W_IDLE) return;

    g.player.altAmmo--;

    // som pode ser o mesmo por enquanto
    audioOnPlayerShot(audio);
    audioPlayShot(audio);

    g.weapon.state = WeaponState::W_FIRE_1;
    g.weapon.timer = 0.08f;

    doShoot(60); // dano dobrado
}

void updateWeaponAnim(float dt)
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    const float TIME_FRAME_2 = 0.12f;
    const float RELOAD_T2 = 0.85f;
    const float RELOAD_T3 = 0.25f;

    if (g.weapon.state == WeaponState::W_IDLE) return;

    g.weapon.timer -= dt;
    if (g.weapon.timer > 0.0f) return;

    if (g.weapon.state == WeaponState::W_FIRE_1)
    {
        g.weapon.state = WeaponState::W_FIRE_2;
        g.weapon.timer = TIME_FRAME_2;
    }
    else if (g.weapon.state == WeaponState::W_FIRE_2)
    {
        g.weapon.state = WeaponState::W_PUMP;
        g.weapon.timer = AudioTuning::PUMP_TIME;
        audioPlayPumpClick(audio);
    }
    else if (g.weapon.state == WeaponState::W_RETURN)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;
    }
    else if (g.weapon.state == WeaponState::W_PUMP)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_1)
    {
        g.weapon.state = WeaponState::W_RELOAD_2;
        g.weapon.timer = RELOAD_T2;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_2)
    {
        g.weapon.state = WeaponState::W_RELOAD_3;
        g.weapon.timer = RELOAD_T3;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_3)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;

        int needed = MAX_MAGAZINE - g.player.currentAmmo;
        if (needed > g.player.reserveAmmo) needed = g.player.reserveAmmo;

        g.player.currentAmmo += needed;
        g.player.reserveAmmo -= needed;
    }
}