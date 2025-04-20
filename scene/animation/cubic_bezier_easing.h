#pragma once

#include "core/math/cubic_bezier.h"
#include "scene/animation/easing.h"

class CubicBezierEasing : public Easing {
	GDCLASS(CubicBezierEasing, Easing);

	Ref<CubicBezier> cubic_bezier;

protected:
	static void _bind_methods();

public:
	static Ref<CubicBezierEasing> create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	void init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2);
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	CubicBezierEasing() {}
};
