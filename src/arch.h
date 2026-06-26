#pragma once


#if defined(SAGITTAR_32_BIT)
    #define SAGITTAR_HAS_BMI2 0
#else
    #if defined(__BMI2__) && defined(__x86_64__)
        #define SAGITTAR_HAS_BMI2 1
    #else
        #define SAGITTAR_HAS_BMI2 0
    #endif
#endif
