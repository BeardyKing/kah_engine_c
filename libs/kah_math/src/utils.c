//===INCLUDES==================================================================
#include <kah_math/utils.h>
//=============================================================================


//===DEFINES===================================================================
#define KAH_PI 3.14159265358979323846264338327950288f
#define KAH_PI_FLOAT 3.1415927f
#define KAH_PI_DOUBLE 3.141592653589793f
//=============================================================================

//===API=======================================================================
float clamp_f(float value, float min, float max)
{
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    return value;
}

float as_degrees_f(float radians)
{
    return radians * (180.0f / KAH_PI_FLOAT);
}

float as_radians_f(float degrees)
{
    return degrees * (KAH_PI_DOUBLE / 180.0f);
}

double clamp_d(double value, double min, double max)
{
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    return value;
}

double as_degrees_d(double radians)
{
    return radians * (180.0 / KAH_PI_DOUBLE);
}

double as_radians_d(double degrees)
{
    return degrees * (KAH_PI_DOUBLE / 180.0);
}
//=============================================================================