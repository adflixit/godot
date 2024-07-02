#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "core/object/ref_counted.h"
#include "core/object/gdvirtual.gen.inc"

class Interpolator : public RefCounted {
	GDCLASS(Interpolator, RefCounted);

protected:
	static void _bind_methods();

	GDVIRTUAL4RC(real_t, _interpolate, real_t, real_t, real_t, real_t)

public:
	virtual real_t interpolate(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const;

	Interpolator() {}
};

class InterpolatorFuncWrapper : public Interpolator {
	GDCLASS(InterpolatorFuncWrapper, Interpolator);

	typedef real_t (*InterpolatorFunc)(real_t t, real_t b, real_t c, real_t d);
	InterpolatorFunc func;

public:
	real_t interpolate(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	InterpolatorFuncWrapper() {}
	InterpolatorFuncWrapper(InterpolatorFunc p_func);
};

#endif // INTERPOLATOR_H
