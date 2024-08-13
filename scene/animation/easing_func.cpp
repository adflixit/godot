#include "easing_func.h"

real_t EasingFunc::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	real_t ret = 0.0;
	GDVIRTUAL_CALL(_ease, p_t, p_b, p_c, p_d, ret);
	return ret;
}

void EasingFunc::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ease", "t", "b", "c", "d"), &EasingFunc::ease);
	GDVIRTUAL_BIND(_ease, "t", "b", "c", "d");
}
