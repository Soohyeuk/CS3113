#ifndef RUN_STATE_H
#define RUN_STATE_H

struct RunState
{
    int   cortisol          = 0;
    float hitCooldown       = 0.0f;
    bool  emergency         = false;
    float emergencyTimeLeft = 0.0f;
};

#endif
