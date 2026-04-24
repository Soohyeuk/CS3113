#include "Scene.h"

#ifndef LEVELA_H
#define LEVELA_H

class LevelA : public Scene {
private:
    static constexpr int LEVEL_WIDTH  = 22;
    static constexpr int LEVEL_HEIGHT = 10;

    // Tileset atlas indices (1-based) — `tileset.png`, 8 columns × N rows,
    // every cell square.
    //   wall    = 9   (row 2 col 1 — blue bathroom tile, solid)
    //   floor   = 6   (row 1 col 6 — gray tile)
    //   blood   = 33,34,35,41,42,43 (rows 5-6 cols 1-3 — decor only)
    //   bathtub = 185-187 / 193-195 / 201-203 (rows 24-26 cols 1-3, 3×3
    //             solid block, ticks cortisol on contact)
    // Top two rows of the grid are wall so the scene reads like a real
    // bathroom (back wall painted at top, floor in front of the player).
    unsigned int mLevelData[22 * 10] = {
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 6, 6, 6, 6, 6,33, 6, 6,185,186,187,6, 6, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 6, 6,41, 6, 6, 6, 6, 6,193,194,195,6, 6, 6, 6, 6, 6, 6,42, 6, 9,
        9, 6, 6, 6, 6, 6, 6, 6, 6,201,202,203,6,43, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 6, 6, 6, 6, 6, 6, 6,35, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    };

    static constexpr int HAZARD_COUNT = 3; // scripted: bob, sweep, orbit
    Vector2 mHazardAnchors[HAZARD_COUNT] = {0};
    float   mHazardTime = 0.0f;

public:
    static constexpr float TILE_DIMENSION = 75.0f;
    // 3 scripted hazards + 1 john_sterling follower.
    static constexpr int   ENEMY_COUNT    = HAZARD_COUNT + 1;

    LevelA();
    LevelA(Vector2 origin, const char *bgHexCode);
    ~LevelA();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
