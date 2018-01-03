#pragma once

namespace UnitTests
{
    // Utility
    bool split();
    bool trim();
    bool clamp();
    bool sign();
#undef max
    bool max();
#undef min
    bool min();

    // Collision
    bool aabbAabb();
    bool pointAabb();
    bool segmentAabb();
    bool sweptAabbAabb();

    // Transform
    bool hierarchy();

    char* successString(bool success);
    void runTest(bool (*function)(), bool* result);
    bool runTests();
}
