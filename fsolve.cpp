#include "fsolve.h"

#define DELTA (1e-3)
#define N_ITER 50

double fsolve(std::function<double(double)> const & f, double x0) {
    double y, dy;
    for (int i = 0; i < N_ITER; i ++) {
        y = f(x0);
        dy = (f(x0+DELTA)-y)/DELTA;
        x0 -= y/dy;
    }
    return x0;
}
