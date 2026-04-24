#ifndef HUD_H
#define HUD_H

#include "cs3113.h"
#include "RunState.h"
#include "PhoneMinigame.h"

class HUD
{
private:
    int mScreenWidth;
    int mScreenHeight;
    int mMaxCortisol;

    void renderCortisolBar(int cortisol)          const;
    void renderEmergencyOverlay(float timeLeft)   const;
    void renderPhoneBadge(const PhoneMinigame &phone) const;

public:
    HUD(int screenWidth, int screenHeight, int maxCortisol);

    void render(const RunState &run, const PhoneMinigame &phone) const;
};

#endif // HUD_H
