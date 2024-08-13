#ifndef EASING_FUNC_H
#define EASING_FUNC_H

#include "core/object/ref_counted.h"
#include "core/object/gdvirtual.gen.inc"

class EasingFunc : public RefCounted {
	GDCLASS(EasingFunc, RefCounted);

protected:
	static void _bind_methods();

	GDVIRTUAL4RC(real_t, _ease, real_t, real_t, real_t, real_t)

public:
	virtual real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const;

	EasingFunc() {}
};

#endif // EASING_FUNC_H
