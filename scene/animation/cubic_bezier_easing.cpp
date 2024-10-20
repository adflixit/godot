#include "cubic_bezier_easing.h"

void CubicBezierEasing::_init_coefficients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	cx = 3.0 * p_x1;
	bx = 3.0 * (p_x2 - p_x1) - cx;
	ax = 1.0 - cx - bx;

	cy = 3.0 * p_y1;
	by = 3.0 * (p_y2 - p_y1) - cy;
	ay = 1.0 - cy - by;
}

void CubicBezierEasing::_init_gradients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	if (p_x1 > 0) {
		start_gradient = p_y1 / p_x1;
	} else if (!p_y1 && p_x2 > 0) {
		start_gradient = p_y2 / p_x2;
	} else if (!p_y1 && !p_y2) {
		start_gradient = 1;
	} else {
		start_gradient = 0;
	}

	if (p_x2 < 1) {
		end_gradient = (p_y2 - 1) / (p_x2 - 1);
	} else if (p_y2 == 1 && p_x1 < 1) {
		end_gradient = (p_y1 - 1) / (p_x1 - 1);
	} else if (p_y2 == 1 && p_y1 == 1) {
		end_gradient = 1;
	} else {
		end_gradient = 0;
	}
}

void CubicBezierEasing::_init_range(real_t p_y1, real_t p_y2) {
	range_min = 0;
	range_max = 1;

	if (0 <= p_y1 && p_y1 < 1 && 0 <= p_y2 && p_y2 <= 1) {
		return;
	}

	const real_t p_epsilon = BEZIER_EPSILON;

	const real_t a = 3.0 * ay;
	const real_t b = 2.0 * by;
	const real_t c = cy;

	if (Math::abs(a) < p_epsilon && Math::abs(b) < p_epsilon) {
		return;
	}

	real_t t1 = 0;
	real_t t2 = 0;

	if (Math::abs(a) < p_epsilon) {
		t1 = -c / b;
	} else {
		real_t discriminant = b * b - 4 * a * c;
		if (discriminant < 0) {
			return;
		}

		real_t discriminant_sqrt = Math::sqrt(discriminant);
		t1 = (-b + discriminant_sqrt) / (2 * a);
		t2 = (-b - discriminant_sqrt) / (2 * a);
	}

	real_t sol1 = 0;
	real_t sol2 = 0;

	if (0 < t1 && t1 < 1) {
		sol1 = _sample_curve_y(t1);
	}

	if (0 < t2 && t2 < 1) {
		sol2 = _sample_curve_y(t2);
	}

	range_min = MIN(MIN(range_min, sol1), sol2);
	range_max = MAX(MAX(range_max, sol1), sol2);
}

void CubicBezierEasing::_init_spline() {
	real_t delta_t = 1.0 / (SPLINE_SAMPLES - 1);
	for (int i = 0; i < SPLINE_SAMPLES; i++) {
		spline_samples[i] = _sample_curve_x(i * delta_t);
	}
}

real_t CubicBezierEasing::_solve_curve_x(real_t p_x, real_t p_epsilon) const {
	real_t t0;
	real_t t1;
	real_t t2 = p_x;
	real_t x2;
	real_t d2;
	int i;

	real_t delta_t = 1.0 / (SPLINE_SAMPLES - 1);
	for (i = 1; i < SPLINE_SAMPLES; i++) {
		if (p_x <= spline_samples[i]) {
			t1 = delta_t * i;
			t0 = t1 - delta_t;
			t2 = t0 + (t1 - t0) * (p_x - spline_samples[i - 1]) /
									(spline_samples[i] - spline_samples[i - 1]);
			break;
		}
	}

	real_t newton_epsilon = MIN(BEZIER_EPSILON, p_epsilon);
	for (i = 0; i < MAX_NEWTON_ITERATIONS; i++) {
		x2 = _sample_curve_x(t2) - p_x;
		if (Math::abs(x2) < newton_epsilon) {
			return t2;
		}
		d2 = _sample_curve_derivative_x(t2);
		if (Math::abs(d2) < BEZIER_EPSILON) {
			break;
		}
		t2 = t2 - x2 / d2;
	}

	if (Math::abs(x2) < p_epsilon) {
		return t2;
	}

	while (t0 < t1) {
		x2 = _sample_curve_x(t2);
		if (Math::abs(x2 - p_x) < p_epsilon) {
			return t2;
		}
		if (p_x > x2) {
			t0 = t2;
		} else {
			t1 = t2;
		}
		t2 = (t1 + t0) * 0.5;
	}

	return t2;
}

real_t CubicBezierEasing::_solve_with_epsilon(real_t p_x, real_t p_epsilon) const {
	if (p_x < 0.0) {
		return 0.0 + start_gradient * p_x;
	}
	if (p_x > 1.0) {
		return 1.0 + end_gradient * (p_x - 1.0);
	}
	return _sample_curve_y(_solve_curve_x(p_x, p_epsilon));
}

Ref<CubicBezierEasing> CubicBezierEasing::create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	Ref<CubicBezierEasing> ref;
	ref.instantiate();
	ref->init(p_x1, p_y1, p_x2, p_y2);
	return ref;
}

void CubicBezierEasing::init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	_init_coefficients(p_x1, p_y1, p_x2, p_y2);
	_init_gradients(p_x1, p_y1, p_x2, p_y2);
	_init_range(p_y1, p_y2);
	_init_spline();
}

real_t CubicBezierEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return p_c * solve(p_t / p_d) + p_b;
}

void CubicBezierEasing::_bind_methods() {
	ClassDB::bind_static_method("CubicBezierEasing", D_METHOD("create", "x1", "y1", "x2", "y2"), &CubicBezierEasing::create);

	ClassDB::bind_method(D_METHOD("init", "x1", "y1", "x2", "y2"), &CubicBezierEasing::init);
	ClassDB::bind_method(D_METHOD("solve", "x"), &CubicBezierEasing::solve);
}
