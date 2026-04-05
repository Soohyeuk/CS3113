#include "Scene.h"

#ifndef LEVELB_H
#define LEVELB_H

class LevelB : public Scene {
private:
    static constexpr int LEVEL_WIDTH  = 25;
    static constexpr int LEVEL_HEIGHT = 8;

    unsigned int mLevelData[25 * 8] = {
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 4,
        4, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 2, 2, 3, 3, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 3, 3, 2, 2, 0, 0, 0, 2, 2, 4,
        4, 3, 3, 3, 3, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 3, 3, 4,
    };

public:
    static constexpr float TILE_DIMENSION          = 75.0f;
    static constexpr float ACCELERATION_OF_GRAVITY = 981.0f;
    static constexpr float FALL_THRESHOLD          = 800.0f;
    static constexpr int   ENEMY_COUNT             = 2;

    LevelB();
    LevelB(Vector2 origin, const char *bgHexCode);
    ~LevelB();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
