#ifndef EASING_H
#define EASING_H

#include "core/io/resource.h"

class Easing : public Resource {
	GDCLASS(Easing, Resource);

protected:
	static void _bind_methods();

public:
	virtual real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const = 0;

	Easing() {}
};

class LinearEasing : public Easing {
	GDCLASS(LinearEasing, Easing);

protected:
	static void _bind_methods() {}

public:
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	LinearEasing() {}
};

#endif // EASING_H
