/***************************************************************************
 *
 * Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file random.h
 * @author tangye01(com@baidu.com)
 * @date 2015/02/11 16:08:52
 * @brief
 *
 **/




#ifndef  __RANDOM_H_
#define  __RANDOM_H_

#include <cmath>
/* use standard PRNG from stdlib
 */
#include <cstdlib>



namespace ty_random {
inline void seed(unsigned seed) {
    srand(seed);
}

// return a real number uniform in [0,1)
inline double next_double() {
    return static_cast<double>(rand()) / (static_cast<double>(RAND_MAX) + 1.0);
}

// return a real number uniform in (0,1)
inline double next_double2() {
    return (static_cast<double>(rand()) + 1.0) / (static_cast<double>(RAND_MAX) + 2.0);
}

// return a random number
inline unsigned next_int(void) {
    return (unsigned)rand();
}

// return a random number in n
inline unsigned next_int(unsigned n) {
    return (unsigned) floor(next_double() * n);
}

// return x~N(0,1) ? why?
inline double sample_normal() {
    double x, y, s;

    do {
        x = 2 * next_double2() - 1.0;
        y = 2 * next_double2() - 1.0;
        s = x * x + y * y;
    } while (s >= 1.0 || s == 0.0);

    return x * sqrt(-2.0 * log(s) / s);
}

// return x~N(mu, sigma^2)
inline double sample_normal(double mu, double sigma) {
    return sample_normal() * sigma + mu;
}
}












#endif  //__RANDOM_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
