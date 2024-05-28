#ifndef CUBIC_BEZIER_H
#define CUBIC_BEZIER_H

#include "core/object/ref_counted.h"

#define CUBIC_BEZIER_SPLINE_SAMPLES 11

class CubicBezier : public RefCounted {
	GDCLASS(CubicBezier, RefCounted);

	const int kMaxNewtonIterations = 4;
	const real_t kBezierEpsilon = 1e-7;

	real_t ax;
	real_t bx;
	real_t cx;

	real_t ay;
	real_t by;
	real_t cy;

	real_t start_gradient;
	real_t end_gradient;

	real_t range_min;
	real_t range_max;

	real_t spline_samples[CUBIC_BEZIER_SPLINE_SAMPLES];

	void init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void init_coefficients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void init_gradients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void init_range(real_t p_y1, real_t p_y2);
	void init_spline();

	_FORCE_INLINE_ real_t sample_curve_x(real_t p_t) const { return ((ax * p_t + bx) * p_t + cx) * p_t; }
	_FORCE_INLINE_ real_t sample_curve_y(real_t p_t) const { return ((ay * p_t + by) * p_t + cy) * p_t; }
	_FORCE_INLINE_ real_t sample_curve_derivative_x(real_t p_t) const { return (3.0 * ax * p_t + 2.0 * bx) * p_t + cx; }
	_FORCE_INLINE_ real_t sample_curve_derivative_y(real_t p_t) const { return (3.0 * ay * p_t + 2.0 * by) * p_t + cy; }

	real_t solve_curve_x(real_t p_x, real_t p_epsilon) const;
	real_t solve_with_epsilon(real_t p_x, real_t p_epsilon) const;

protected:
	static void _bind_methods();

public:
	static Ref<CubicBezier> create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	_FORCE_INLINE_ real_t solve(real_t p_x) const { return solve_with_epsilon(p_x, kBezierEpsilon); }

	CubicBezier() {}
	CubicBezier(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	~CubicBezier() {}
};

#endif // CUBIC_BEZIER_H
