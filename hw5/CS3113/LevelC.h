#include "Scene.h"

#ifndef LEVELC_H
#define LEVELC_H

class LevelC : public Scene {
private:
    static constexpr int LEVEL_WIDTH  = 28;
    static constexpr int LEVEL_HEIGHT = 16;

    // Tileset indices (tileset.png, 8 cols × N rows, square cells):
    //   wall  = 8  (row 1 col 8, solid)
    //   floor = 5  (row 1 col 5)
    // A larger, mazier layout — the player has to pick their way up and
    // down through vertical corridors before reaching the bottom-right exit.
    // Trash decor (non-solid, cosmetic): 24, 29, 30 scattered across the floor.
    unsigned int mLevelData[28 * 16] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 5, 5,24, 5, 5, 5, 5, 5,29, 5, 5, 5, 5, 5, 5, 5, 5,30, 5, 5, 5, 5, 5,24, 5, 5, 8,
        8, 5, 5, 5, 8, 8, 5, 5, 5, 5, 5, 8, 8, 5,29, 5, 5, 5, 5, 8, 8, 5, 5, 5, 5, 5, 5, 8,
        8, 5,30, 5, 8, 8, 5, 5, 5, 5, 5, 8, 8, 5, 5, 5,24, 5, 5, 8, 8, 5, 5, 5, 5,30, 5, 8,
        8, 5, 5, 5, 8, 8, 5,24, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,29, 5, 5, 5, 5, 8,
        8, 5,24, 5, 5, 5, 5, 5, 5, 8, 8, 5, 5, 5, 5,30, 5, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 8,
        8, 5, 5, 5, 5,29, 5, 5, 5, 8, 8, 5, 5, 5, 5, 5, 5, 8, 8, 5, 5, 5, 5, 5, 5,24, 5, 8,
        8, 5, 5, 5, 5, 5, 5,30, 5, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5,29, 5, 5, 5, 5, 5, 5, 8,
        8, 5, 5, 5, 8, 8, 5, 5, 5, 5,24, 5, 5, 8, 8, 5, 5, 5, 5, 5, 5, 8, 8, 5, 5,30, 5, 8,
        8, 5,29, 5, 8, 8, 5, 5, 5, 5, 5, 5, 5, 8, 8, 5, 5,24, 5, 5, 5, 8, 8, 5, 5, 5, 5, 8,
        8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,30, 5, 5, 5, 5, 5, 5, 5,24, 5, 5, 8,
        8, 5, 5,30, 5, 5, 8, 8, 5, 5, 5, 5,29, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 8,
        8, 5, 5, 5, 5, 5, 8, 8, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,24, 5, 5, 5, 5,30, 5, 5, 5, 8,
        8, 5, 5, 5, 5,24, 5, 5, 5, 5,29, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    };

    static constexpr int HAZARD_COUNT = 6; // scripted hazards (bob/sweep/orbit)
    Vector2 mHazardAnchors[HAZARD_COUNT] = {0};
    int     mHazardMotions[HAZARD_COUNT] = {0}; // 0=bob, 1=sweep, 2=orbit
    float   mHazardTime = 0.0f;

public:
    static constexpr float TILE_DIMENSION = 75.0f;
    static constexpr int   ENEMY_COUNT    = HAZARD_COUNT + 1;

    LevelC();
    LevelC(Vector2 origin, const char *bgHexCode);
    ~LevelC();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
