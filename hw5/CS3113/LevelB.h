#include "Scene.h"

#ifndef LEVELB_H
#define LEVELB_H

class LevelB : public Scene {
private:
    static constexpr int LEVEL_WIDTH  = 24;
    static constexpr int LEVEL_HEIGHT = 14;

    // Tileset indices (tileset.png, 8 cols × N rows, square cells):
    //   wall  = 10 (row 2 col 2, solid)
    //   floor = 13 (row 2 col 5)
    // Layout is a loose maze — internal wall blocks force the player
    // to weave up and down between corridors instead of running
    // straight east.  A 2-row-thick top wall gives the scene a proper
    // "back wall" when viewed top-down.
    // Trash decor (non-solid, cosmetic):
    //   24 = row 3 col 8, 29 = row 4 col 5, 30 = row 4 col 6
    unsigned int mLevelData[24 * 14] = {
        10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
        10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
        10,13,13,13,24,13,13,13,13,13,13,13,13,13,13,13,29,13,13,13,13,13,13,10,
        10,13,13,13,10,10,13,13,13,13,13,30,13,13,10,10,10,13,13,13,13,13,13,10,
        10,13,30,13,10,10,13,13,13,13,13,13,13,13,10,10,10,13,13,13,24,13,13,10,
        10,13,13,13,10,10,13,13,13,10,10,13,13,13,29,13,13,13,13,13,13,13,13,10,
        10,13,13,13,13,29,13,13,13,10,10,13,13,13,13,13,13,13,13,24,13,13,13,10,
        10,13,13,13,13,13,13,30,13,13,13,13,13,13,13,13,13,10,10,13,13,13,13,10,
        10,13,13,13,10,10,13,13,13,13,24,13,13,13,13,13,13,10,10,13,13,13,13,10,
        10,13,13,13,10,10,13,13,13,13,13,13,10,10,13,13,13,13,13,13,13,13,13,10,
        10,13,24,13,13,13,13,13,13,13,13,13,10,10,13,13,13,13,13,30,13,13,13,10,
        10,13,13,13,13,13,13,13,13,13,29,13,13,13,13,13,13,13,13,13,13,13,13,10,
        10,13,13,13,13,13,30,13,13,13,13,13,13,13,13,24,13,13,13,13,13,13,13,10,
        10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
    };

    static constexpr int HAZARD_COUNT = 5; // scripted hazards (bob/sweep/orbit)
    Vector2 mHazardAnchors[HAZARD_COUNT] = {0};
    int     mHazardMotions[HAZARD_COUNT] = {0}; // 0=bob, 1=sweep, 2=orbit
    float   mHazardTime = 0.0f;

public:
    static constexpr float TILE_DIMENSION = 75.0f;
    static constexpr int   ENEMY_COUNT    = HAZARD_COUNT + 1;

    LevelB();
    LevelB(Vector2 origin, const char *bgHexCode);
    ~LevelB();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
