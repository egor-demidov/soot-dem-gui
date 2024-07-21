#ifndef FSOLVE_H
#define FSOLVE_H

#include <functional>

double fsolve(std::function<double(double)> const & f, double x0);

#endif // FSOLVE_H
