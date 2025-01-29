
#include "toolgui/NodeMacros.h"


#include "boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp"
#include "boost/numeric/odeint/stepper/controlled_runge_kutta.hpp"
#include "boost/numeric/odeint/stepper/dense_output_runge_kutta.hpp"
#include "boost/numeric/odeint/stepper/controlled_step_result.hpp"
#include "boost/container/flat_set.hpp"

#include <memory_resource>
#include <vector>
#include <functional>
#include <span>
#include <memory>
#include <cassert>
#include <algorithm>
#include <queue>
#include <iostream>
#include <iomanip>
#include <cmath>
