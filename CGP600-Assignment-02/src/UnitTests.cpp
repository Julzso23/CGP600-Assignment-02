#include "UnitTests.hpp"
#include "Utility.hpp"
#include "collision/AABB.hpp"

namespace UnitTests
{
    using namespace DirectX;

    bool split()
    {
        bool result = true;

        std::vector<std::string> splitString;
        splitString = Utility::split("test,string,something", ',');
        if (splitString[0] != "test")
        {
            result = false;
        }
        if (splitString[1] != "string")
        {
            result = false;
        }
        if (splitString[2] != "something")
        {
            result = false;
        }

        splitString = Utility::split("string.test,something", '.');
        if (splitString[0] != "string")
        {
            result = false;
        }
        if (splitString[1] != "test,something")
        {
            result = false;
        }

        printf("Split test: %s\n", successString(result));
        return result;
    }

    bool trim()
    {
        bool result = true;

        if (Utility::trim("   test string        ") != "test string")
        {
            result = false;
        }
        if (Utility::trim("\ttest string    ") != "test string")
        {
            result = false;
        }

        printf("Trim test: %s\n", successString(result));
        return result;
    }

    bool clamp()
    {
        bool result = true;

        const int min = 1;
        const int max = 3;

        if (Utility::clamp(min - 1, min, max) != min)
        {
            result = false;
        }
        if (Utility::clamp(min, min, max) != min)
        {
            result = false;
        }
        if (Utility::clamp((min + max) / 2, min, max) != (min + max) / 2)
        {
            result = false;
        }
        if (Utility::clamp(max, min, max) != max)
        {
            result = false;
        }
        if (Utility::clamp(max + 1, min, max) != max)
        {
            result = false;
        }

        printf("Clamp test: %s\n", successString(result));
        return result;
    }

    bool sign()
    {
        bool result = true;

        if (Utility::sign(-7) != -1)
        {
            result = false;
        }
        if (Utility::sign(-3) != -1)
        {
            result = false;
        }
        if (Utility::sign(5) != 1)
        {
            result = false;
        }
        if (Utility::sign(4) != 1)
        {
            result = false;
        }

        printf("Sign test: %s\n", successString(result));
        return result;
    }

    bool max()
    {
        bool result = true;

        if (Utility::max(1, 2) != 2)
        {
            result = false;
        }
        if (Utility::max(5, 1, 3) != 5)
        {
            result = false;
        }
        if (Utility::max(7, 4) != 7)
        {
            result = false;
        }
        if (Utility::max(4, 6, 2, 3) != 6)
        {
            result = false;
        }

        printf("Max test: %s\n", successString(result));
        return result;
    }

    bool min()
    {
        bool result = true;

        if (Utility::min(1, 2) != 1)
        {
            result = false;
        }
        if (Utility::min(5, 1, 3) != 1)
        {
            result = false;
        }
        if (Utility::min(7, 4) != 4)
        {
            result = false;
        }
        if (Utility::min(4, 6, 2, 3) != 2)
        {
            result = false;
        }

        printf("Min test: %s\n", successString(result));
        return result;
    }

    bool aabbAabb()
    {
        bool result = true;

        AABB boxA;
        boxA.setSize(XMVectorSet(2.f, 2.f, 2.f, 0.f));
        boxA.setPosition(XMVectorZero());

        AABB boxB;
        boxB.setSize(XMVectorSet(4.f, 2.f, 1.f, 0.f));

        boxB.setPosition(XMVectorSet(-3.f, 0.f, 0.f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(3.f, 0.f, 0.f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(0.f, 2.f, 0.f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(0.f, -2.f, 0.f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(0.f, 0.f, 1.5f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(0.f, 0.f, -1.5f, 1.f));
        if (boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        boxB.setPosition(XMVectorSet(-1.5f, 1.f, 0.5f, 1.f));
        if (!boxA.testIntersection(boxB).hit)
        {
            result = false;
        }

        printf("AABB -> AABB collision test: %s\n", successString(result));
        return result;
    }

    bool pointAabb()
    {
        bool result = true;

        AABB box;
        box.setSize(XMVectorSet(2.f, 2.f, 2.f, 0.f));
        box.setPosition(XMVectorZero());

        if (box.testIntersection(XMVectorSet(1.f, 0.f, 0.f, 1.f)).hit)
        {
            result = false;
        }
        if (box.testIntersection(XMVectorSet(-1.f, 0.f, 0.f, 1.f)).hit)
        {
            result = false;
        }
        if (box.testIntersection(XMVectorSet(0.f, 1.f, 0.f, 1.f)).hit)
        {
            result = false;
        }
        if (box.testIntersection(XMVectorSet(0.f, -1.f, 0.f, 1.f)).hit)
        {
            result = false;
        }
        if (box.testIntersection(XMVectorSet(0.f, 0.f, 1.f, 1.f)).hit)
        {
            result = false;
        }
        if (box.testIntersection(XMVectorSet(0.f, 0.f, -1.f, 1.f)).hit)
        {
            result = false;
        }
        if (!box.testIntersection(XMVectorSet(0.5f, -0.5f, 0.3f, 1.f)).hit)
        {
            result = false;
        }

        printf("Point -> AABB collision test: %s\n", successString(result));
        return result;
    }

    bool segmentAabb()
    {
        bool result = true;

        AABB box;
        box.setSize(XMVectorSet(2.f, 2.f, 2.f, 0.f));
        box.setPosition(XMVectorZero());

        if (!box.testIntersection({ XMVectorSet(0.f, 0.f, -1.f, 0.f), XMVectorSet(1.f, 0.f, 1.f, 0.f) }).hit)
        {
            result = false;
        }

        if (box.testIntersection({ XMVectorSet(-2.f, 0.f, -1.1f, 0.f), XMVectorSet(2.f, 0.f, 0.f, 0.f) }).hit)
        {
            result = false;
        }

        printf("Segment -> AABB collision test: %s\n", successString(result));
        return result;
    }

    bool sweptAabbAabb()
    {
        bool result = true;

        AABB boxA;
        boxA.setSize(XMVectorSet(2.f, 2.f, 2.f, 0.f));
        boxA.setPosition(XMVectorZero());

        AABB boxB;
        boxB.setSize(XMVectorSet(4.f, 2.f, 1.f, 0.f));
        boxB.setPosition(XMVectorSet(-3.f, 0.f, 0.f, 1.f));

        if (!boxA.sweepIntersection(boxB, XMVectorSet(6.f, 0.f, 0.f, 0.f)).hit.hit)
        {
            result = false;
        }

        printf("Swept AABB -> AABB collision test: %s\n", successString(result));
        return result;
    }

    bool hierarchy()
    {
        bool result = true;

        Transformable objectA;
        objectA.setPosition(XMVectorZero());
        Transformable objectB;
        objectB.setPosition(XMVectorZero());
        objectB.setLocalPosition(XMVectorSet(1.f, 0.f, 0.f, 1.f));
        objectA.addChild(&objectB);

        objectA.setPosition(XMVectorSet(0.f, 2.f, 0.f, 1.f));

        if (XMVector3NotEqual(objectB.getPosition(), XMVectorSet(1.f, 2.f, 0.f, 1.f)))
        {
            result = false;
        }

        printf("Hierarchy test: %s\n", successString(result));
        return result;
    }

    char* successString(bool success)
    {
        return success ? "SUCCESS" : "FAILURE";
    }

    void runTest(bool(*function)(), bool* result)
    {
        if (!function())
        {
            *result = false;
        }
    }

    bool runTests()
    {
        bool result = true;

        // Utility
        runTest(split, &result);
        runTest(trim, &result);
        runTest(clamp, &result);
        runTest(sign, &result);
        runTest(max, &result);
        runTest(min, &result);

        // Collision
        runTest(aabbAabb, &result);
        runTest(pointAabb, &result);
        runTest(segmentAabb, &result);
        runTest(sweptAabbAabb, &result);

        // Transform
        runTest(hierarchy, &result);

        printf("\n\tFinal result: %s\n", successString(result));
        return result;
    }
}
