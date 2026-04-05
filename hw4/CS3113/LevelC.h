#include "Scene.h"

#ifndef LEVELC_H
#define LEVELC_H

class LevelC : public Scene {
private:
    static constexpr int LEVEL_WIDTH  = 30;
    static constexpr int LEVEL_HEIGHT = 8;

    unsigned int mLevelData[30 * 8] = {
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 4,
        4, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 2, 0, 4,
        4, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 4,
        4, 2, 2, 3, 0, 0, 0, 2, 2, 0, 3, 0, 0, 0, 3, 0, 0, 2, 2, 3, 0, 3, 0, 2, 0, 0, 0, 2, 2, 4,
        4, 3, 3, 3, 0, 0, 0, 3, 3, 0, 3, 0, 0, 0, 3, 0, 0, 3, 3, 3, 0, 3, 0, 3, 0, 0, 0, 3, 3, 4,
    };

public:
    static constexpr float TILE_DIMENSION          = 75.0f;
    static constexpr float ACCELERATION_OF_GRAVITY = 981.0f;
    static constexpr float FALL_THRESHOLD          = 800.0f;
    static constexpr int   ENEMY_COUNT             = 5;

    LevelC();
    LevelC(Vector2 origin, const char *bgHexCode);
    ~LevelC();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif
