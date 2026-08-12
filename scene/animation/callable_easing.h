#pragma once

#include "scene/animation/easing.h"

class CallableEasing : public Easing {
	GDCLASS(CallableEasing, Easing);

	Callable callable;

protected:
	static void _bind_methods();

public:
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;
	static Ref<CallableEasing> create(const Callable &p_callable);

	CallableEasing();
	CallableEasing(const Callable &p_callable);
};
