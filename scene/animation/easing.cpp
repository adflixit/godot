#include "easing.h"

#include "scene/animation/easing_equations.h"

real_t Easing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	real_t ret = 0.0;
	GDVIRTUAL_CALL(_ease, p_t, p_b, p_c, p_d, ret);
	return ret;
}

void Easing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ease", "t", "b", "c", "d"), &Easing::ease);

	GDVIRTUAL_BIND(_ease, "t", "b", "c", "d")
}

EquationEasing::EasingFunc EquationEasing::equations[EquationEasing::EQ_MAX] = {
	&linear::in,
	&sine::in,
	&sine::out,
	&sine::in_out,
	&sine::out_in,
	&quint::in,
	&quint::out,
	&quint::in_out,
	&quint::out_in,
	&quart::in,
	&quart::out,
	&quart::in_out,
	&quart::out_in,
	&quad::in,
	&quad::out,
	&quad::in_out,
	&quad::out_in,
	&expo::in,
	&expo::out,
	&expo::in_out,
	&expo::out_in,
	&elastic::in,
	&elastic::out,
	&elastic::in_out,
	&elastic::out_in,
	&cubic::in,
	&cubic::out,
	&cubic::in_out,
	&cubic::out_in,
	&circ::in,
	&circ::out,
	&circ::in_out,
	&circ::out_in,
	&bounce::in,
	&bounce::out,
	&bounce::in_out,
	&bounce::out_in,
	&back::in,
	&back::out,
	&back::in_out,
	&back::out_in,
	&spring::in,
	&spring::out,
	&spring::in_out,
	&spring::out_in,
};

Ref<EquationEasing> EquationEasing::create(Equation p_equation) {
	Ref<EquationEasing> ref = memnew(EquationEasing(p_equation));
	return ref;
}

real_t EquationEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return func(p_t, p_b, p_c, p_d);
}

void EquationEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ease", "t", "b", "c", "d"), &EquationEasing::ease);
	ClassDB::bind_static_method("EquationEasing", D_METHOD("create", "equation"), &EquationEasing::create);

	BIND_ENUM_CONSTANT(EQ_LINEAR);
	BIND_ENUM_CONSTANT(EQ_SINE_IN);
	BIND_ENUM_CONSTANT(EQ_SINE_OUT);
	BIND_ENUM_CONSTANT(EQ_SINE_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_SINE_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_QUINT_IN);
	BIND_ENUM_CONSTANT(EQ_QUINT_OUT);
	BIND_ENUM_CONSTANT(EQ_QUINT_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_QUINT_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_QUART_IN);
	BIND_ENUM_CONSTANT(EQ_QUART_OUT);
	BIND_ENUM_CONSTANT(EQ_QUART_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_QUART_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_QUAD_IN);
	BIND_ENUM_CONSTANT(EQ_QUAD_OUT);
	BIND_ENUM_CONSTANT(EQ_QUAD_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_QUAD_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_EXPO_IN);
	BIND_ENUM_CONSTANT(EQ_EXPO_OUT);
	BIND_ENUM_CONSTANT(EQ_EXPO_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_EXPO_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_ELASTIC_IN);
	BIND_ENUM_CONSTANT(EQ_ELASTIC_OUT);
	BIND_ENUM_CONSTANT(EQ_ELASTIC_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_ELASTIC_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_CUBIC_IN);
	BIND_ENUM_CONSTANT(EQ_CUBIC_OUT);
	BIND_ENUM_CONSTANT(EQ_CUBIC_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_CUBIC_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_CIRC_IN);
	BIND_ENUM_CONSTANT(EQ_CIRC_OUT);
	BIND_ENUM_CONSTANT(EQ_CIRC_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_CIRC_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_BOUNCE_IN);
	BIND_ENUM_CONSTANT(EQ_BOUNCE_OUT);
	BIND_ENUM_CONSTANT(EQ_BOUNCE_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_BOUNCE_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_BACK_IN);
	BIND_ENUM_CONSTANT(EQ_BACK_OUT);
	BIND_ENUM_CONSTANT(EQ_BACK_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_BACK_OUT_IN);
	BIND_ENUM_CONSTANT(EQ_SPRING_IN);
	BIND_ENUM_CONSTANT(EQ_SPRING_OUT);
	BIND_ENUM_CONSTANT(EQ_SPRING_IN_OUT);
	BIND_ENUM_CONSTANT(EQ_SPRING_OUT_IN);
}

EquationEasing::EquationEasing(Equation p_equation) {
	func = equations[p_equation];
}

EquationEasing::EquationEasing() {
	ERR_FAIL_MSG("EquationEasing can't be created directly. Use the create() method.");
}

Ref<CallableEasing> CallableEasing::create(const Callable &p_callable) {
	Ref<CallableEasing> ref = memnew(CallableEasing(p_callable));
	return ref;
}

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

void CallableEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("ease", "t", "b", "c", "d"), &CallableEasing::ease);
	ClassDB::bind_static_method("CallableEasing", D_METHOD("create", "callable"), &CallableEasing::create);
}

CallableEasing::CallableEasing(const Callable &p_callable) {
	callable = p_callable;
}

CallableEasing::CallableEasing() {
	ERR_FAIL_MSG("CallableEasing can't be created directly. Use the create() method.");
}
