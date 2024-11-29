#include "easing.h"

void Easing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ease", "t", "b", "c", "d"), &Easing::ease);
}

Ref<LinearEasing> LinearEasing::create() {
	Ref<LinearEasing> ref;
	ref.instantiate();
	return ref;
}

real_t LinearEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return p_c * p_t / p_d + p_b;
}
