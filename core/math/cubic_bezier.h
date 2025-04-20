#pragma once

#include "core/object/ref_counted.h"

class CubicBezier : public RefCounted {
	GDCLASS(CubicBezier, RefCounted);

	static const int SPLINE_SAMPLES = 11;
	static const int MAX_NEWTON_ITERATIONS = 4;
	static constexpr real_t BEZIER_EPSILON = 1e-7;

	real_t ax = 0.0;
	real_t bx = 0.0;
	real_t cx = 0.0;

	real_t ay = 0.0;
	real_t by = 0.0;
	real_t cy = 0.0;

	real_t start_gradient = 0.0;
	real_t end_gradient = 0.0;

	real_t range_min = 0.0;
	real_t range_max = 0.0;

	real_t spline_samples[SPLINE_SAMPLES];

	void _init_coefficients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void _init_gradients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void _init_range(real_t p_y1, real_t p_y2);
	void _init_spline();

protected:
	static void _bind_methods();

public:
	void init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);

	_FORCE_INLINE_ real_t sample_curve_x(real_t p_t) const { return ((ax * p_t + bx) * p_t + cx) * p_t; }
	_FORCE_INLINE_ real_t sample_curve_y(real_t p_t) const { return ((ay * p_t + by) * p_t + cy) * p_t; }
	_FORCE_INLINE_ real_t sample_curve_derivative_x(real_t p_t) const { return (3.0 * ax * p_t + 2.0 * bx) * p_t + cx; }
	_FORCE_INLINE_ real_t sample_curve_derivative_y(real_t p_t) const { return (3.0 * ay * p_t + 2.0 * by) * p_t + cy; }

	real_t solve_curve_x(real_t p_x, real_t p_epsilon) const;
	real_t solve_with_epsilon(real_t p_x, real_t p_epsilon) const;
	_FORCE_INLINE_ real_t solve(real_t p_x) const { return solve_with_epsilon(p_x, BEZIER_EPSILON); }

	real_t slope_with_epsilon(real_t p_x, real_t p_epsilon) const;
	_FORCE_INLINE_ real_t slope(real_t p_x) const { return slope_with_epsilon(p_x, BEZIER_EPSILON); }

	real_t get_range_min() const;
	real_t get_range_max() const;

	CubicBezier() {}
};
