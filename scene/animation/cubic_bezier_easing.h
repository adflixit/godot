#ifndef CUBIC_BEZIER_EASING_H
#define CUBIC_BEZIER_EASING_H

#include "scene/animation/easing_func.h"

class CubicBezierEasing : public EasingFunc {
	GDCLASS(CubicBezierEasing, EasingFunc);

	static const int SPLINE_SAMPLES = 11;
	static const int MAX_NEWTON_ITERATIONS = 4;
	static constexpr real_t BEZIER_EPSILON = 1e-7;

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

	real_t spline_samples[SPLINE_SAMPLES];

	
	void _init_coefficients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void _init_gradients(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void _init_range(real_t p_y1, real_t p_y2);
	void _init_spline();

	_FORCE_INLINE_ real_t _sample_curve_x(real_t p_t) const { return ((ax * p_t + bx) * p_t + cx) * p_t; }
	_FORCE_INLINE_ real_t _sample_curve_y(real_t p_t) const { return ((ay * p_t + by) * p_t + cy) * p_t; }
	_FORCE_INLINE_ real_t _sample_curve_derivative_x(real_t p_t) const { return (3.0 * ax * p_t + 2.0 * bx) * p_t + cx; }
	_FORCE_INLINE_ real_t _sample_curve_derivative_y(real_t p_t) const { return (3.0 * ay * p_t + 2.0 * by) * p_t + cy; }

	real_t _solve_curve_x(real_t p_x, real_t p_epsilon) const;
	real_t _solve_with_epsilon(real_t p_x, real_t p_epsilon) const;

protected:
	static void _bind_methods();

public:
	static Ref<CubicBezierEasing> create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);

	void init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	_FORCE_INLINE_ real_t solve(real_t p_x) const { return _solve_with_epsilon(p_x, BEZIER_EPSILON); }
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	CubicBezierEasing() {}
};

#endif // CUBIC_BEZIER_EASING_H
