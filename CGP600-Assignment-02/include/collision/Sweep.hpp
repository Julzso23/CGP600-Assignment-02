#pragma once

#include "collision\Hit.hpp"

struct Sweep
{
    Hit hit;
    DirectX::XMVECTOR position;
    float time;
};