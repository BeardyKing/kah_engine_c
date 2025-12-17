//===INCLUDES==================================================================
#include <kah_math/utils.h>
//=============================================================================

//===API=======================================================================
float as_degrees_f(float radians){
    return radians * (180.0f / KAH_PI_FLOAT);
}

float as_radians_f(float degrees){
    return degrees * (KAH_PI_FLOAT / 180.0f);
}

double as_degrees_d(double radians){
    return radians * (180.0 / KAH_PI_DOUBLE);
}

double as_radians_d(double degrees){
    return degrees * (KAH_PI_DOUBLE / 180.0);
}
//=============================================================================
