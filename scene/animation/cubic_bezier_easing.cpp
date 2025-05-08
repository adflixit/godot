#include "cubic_bezier_easing.h"

Ref<CubicBezierEasing> CubicBezierEasing::create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	Ref<CubicBezierEasing> ref;
	ref.instantiate();
	ref->init(p_x1, p_y1, p_x2, p_y2);
	return ref;
}

Vector<double> CubicBezierEasing::parse(const String &p_string) {
	ERR_FAIL_COND_V(p_string.is_empty(), Vector<double>());
	Vector<double> vector = p_string.split_floats(",");

	ERR_FAIL_COND_V(vector.size() < 4, Vector<double>());
	return vector;
}

void CubicBezierEasing::init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	if (cubic_bezier.is_null()) {
		cubic_bezier.instantiate();
	}
	cubic_bezier->init(p_x1, p_y1, p_x2, p_y2);
}

real_t CubicBezierEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return p_c * cubic_bezier->solve(p_t / p_d) + p_b;
}

void CubicBezierEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "x1", "y1", "x2", "y2"), &CubicBezierEasing::init);

	ClassDB::bind_static_method("CubicBezierEasing", D_METHOD("create", "x1", "y1", "x2", "y2"), &CubicBezierEasing::create);
	ClassDB::bind_static_method("CubicBezierEasing", D_METHOD("parse", "string"), &CubicBezierEasing::parse);
}
