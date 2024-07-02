#include "interpolator.h"

real_t Interpolator::interpolate(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	real_t ret = 0.0;
	GDVIRTUAL_CALL(_interpolate, p_t, p_b, p_c, p_d, ret);
	return ret;
}

void Interpolator::_bind_methods() {
	ClassDB::bind_method(D_METHOD("interpolate", "t", "b", "c", "d"), &Interpolator::interpolate);
	GDVIRTUAL_BIND(_interpolate, "t", "b", "c", "d");
}

real_t InterpolatorFuncWrapper::interpolate(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return func(p_t, p_b, p_c, p_d);
}

InterpolatorFuncWrapper::InterpolatorFuncWrapper(InterpolatorFunc p_func) {
	func = p_func;
}
