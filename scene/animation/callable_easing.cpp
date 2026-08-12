#include "callable_easing.h"

real_t CallableEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	ERR_FAIL_COND_V_MSG(!callable.is_valid(), 0.0, vformat(R"(Invalid callable "%s".)", callable));

	Variant result;
	Callable::CallError ce;
	Variant t = p_t, b = p_b, c = p_c, d = p_d;
	const Variant *args[4] = { &t, &b, &c, &d };

	callable.callp(args, 4, result, ce);
	if (ce.error != Callable::CallError::CALL_OK) {
		ERR_FAIL_V_MSG(0.0, vformat("Error calling method from CallableEasing: %s.", Variant::get_callable_error_text(callable, args, 4, ce)));
	} else if (result.get_type() != Variant::FLOAT) {
		ERR_FAIL_V_MSG(0.0, vformat("Wrong return type in CallableEasing method. Expected float, got %s.", Variant::get_type_name(result.get_type())));
	}

	return result;
}

Ref<CallableEasing> CallableEasing::create(const Callable &p_callable) {
	Ref<CallableEasing> ref = memnew(CallableEasing(p_callable));
	return ref;
}

void CallableEasing::_bind_methods() {
	ClassDB::bind_static_method("CallableEasing", D_METHOD("create", "callable"), &CallableEasing::create);
}

CallableEasing::CallableEasing() {
	ERR_FAIL_MSG("CallableEasing can't be created directly. Use the create() method.");
}

CallableEasing::CallableEasing(const Callable &p_callable) {
	callable = p_callable;
}
