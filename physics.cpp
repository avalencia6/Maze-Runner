#include "physics.h"
#include "math_extra.h"

void forward_euler(Physics* state, float delta)
{ 
    // TODO: Implement proper forward euler updates for position and velocity
    float dx = clamp(state->vx + state->ax * delta, 25);
    float dy = clamp(state->vy + state->ay * delta, 25);

    // Update position and velocity
    state->px = state->px + dx * delta;
    state->py = state->py + dy * delta;
    state->vx = dx;
    state->vy = dy;
}
