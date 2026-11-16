#include "easing.h"

#include "core/math/cubic_bezier.h"

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
	&Linear::in,
	&Sine::in,
	&Sine::out,
	&Sine::in_out,
	&Sine::out_in,
	&Quint::in,
	&Quint::out,
	&Quint::in_out,
	&Quint::out_in,
	&Quart::in,
	&Quart::out,
	&Quart::in_out,
	&Quart::out_in,
	&Quad::in,
	&Quad::out,
	&Quad::in_out,
	&Quad::out_in,
	&Expo::in,
	&Expo::out,
	&Expo::in_out,
	&Expo::out_in,
	&Elastic::in,
	&Elastic::out,
	&Elastic::in_out,
	&Elastic::out_in,
	&Cubic::in,
	&Cubic::out,
	&Cubic::in_out,
	&Cubic::out_in,
	&Circ::in,
	&Circ::out,
	&Circ::in_out,
	&Circ::out_in,
	&Bounce::in,
	&Bounce::out,
	&Bounce::in_out,
	&Bounce::out_in,
	&Back::in,
	&Back::out,
	&Back::in_out,
	&Back::out_in,
	&Spring::in,
	&Spring::out,
	&Spring::in_out,
	&Spring::out_in,
};

void EquationEasing::set_equation(EquationEasing::Equation p_equation) {
	equation = p_equation;
	func = equations[equation];
}

EquationEasing::Equation EquationEasing::get_equation() const {
	return equation;
}

real_t EquationEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return func(p_t, p_b, p_c, p_d);
}

Ref<EquationEasing> EquationEasing::create(EquationEasing::Equation p_equation) {
	Ref<EquationEasing> ref = memnew(EquationEasing(p_equation));
	return ref;
}

EquationEasing::Equation EquationEasing::to_equation(EquationEasing::TransitionType p_trans_type, EquationEasing::EaseType p_ease_type) {
	return static_cast<EquationEasing::Equation>(CLAMP(p_trans_type * 4 + p_ease_type - 3, 0, static_cast<int>(EquationEasing::EQ_MAX)));
}

void EquationEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_equation", "equation"), &EquationEasing::set_equation);
	ClassDB::bind_method(D_METHOD("get_equation"), &EquationEasing::get_equation);

	ClassDB::bind_static_method("EquationEasing", D_METHOD("create", "equation"), &EquationEasing::create);
	ClassDB::bind_static_method("EquationEasing", D_METHOD("to_equation", "trans_type", "ease_type"), &EquationEasing::to_equation);

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

	BIND_ENUM_CONSTANT(TRANS_LINEAR);
	BIND_ENUM_CONSTANT(TRANS_SINE);
	BIND_ENUM_CONSTANT(TRANS_QUINT);
	BIND_ENUM_CONSTANT(TRANS_QUART);
	BIND_ENUM_CONSTANT(TRANS_QUAD);
	BIND_ENUM_CONSTANT(TRANS_EXPO);
	BIND_ENUM_CONSTANT(TRANS_ELASTIC);
	BIND_ENUM_CONSTANT(TRANS_CUBIC);
	BIND_ENUM_CONSTANT(TRANS_CIRC);
	BIND_ENUM_CONSTANT(TRANS_BOUNCE);
	BIND_ENUM_CONSTANT(TRANS_BACK);
	BIND_ENUM_CONSTANT(TRANS_SPRING);

	BIND_ENUM_CONSTANT(EASE_IN);
	BIND_ENUM_CONSTANT(EASE_OUT);
	BIND_ENUM_CONSTANT(EASE_IN_OUT);
	BIND_ENUM_CONSTANT(EASE_OUT_IN);
}

EquationEasing::EquationEasing(EquationEasing::Equation p_equation) {
	set_equation(p_equation);
}

EquationEasing::EquationEasing() {
	ERR_FAIL_MSG("EquationEasing can't be created directly. Use the create() method.");
}

void CubicBezierEasing::init(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	if (cubic_bezier.is_null()) {
		cubic_bezier.instantiate();
	}
	cubic_bezier->init(p_x1, p_y1, p_x2, p_y2);
}

real_t CubicBezierEasing::ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const {
	return p_c * cubic_bezier->solve(p_t / p_d) + p_b;
}

Ref<CubicBezierEasing> CubicBezierEasing::create(real_t p_x1, real_t p_y1, real_t p_x2, real_t p_y2) {
	Ref<CubicBezierEasing> ref;
	ref.instantiate();
	ref->init(p_x1, p_y1, p_x2, p_y2);
	return ref;
}

void CubicBezierEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "x1", "y1", "x2", "y2"), &CubicBezierEasing::init);

	ClassDB::bind_static_method("CubicBezierEasing", D_METHOD("create", "x1", "y1", "x2", "y2"), &CubicBezierEasing::create);
}

void CallableEasing::set_callable(const Callable &p_callable) {
	callable = p_callable;
}

Callable CallableEasing::get_callable() const {
	return callable;
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
		ERR_FAIL_V_MSG(0.0, vformat("Invalid return type in CallableEasing method. Expected float, got %s.", Variant::get_type_name(result.get_type())));
	}

	return result;
}

Ref<CallableEasing> CallableEasing::create(const Callable &p_callable) {
	Ref<CallableEasing> ref = memnew(CallableEasing(p_callable));
	return ref;
}

void CallableEasing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_callable", "callable"), &CallableEasing::set_callable);
	ClassDB::bind_method(D_METHOD("get_callable"), &CallableEasing::get_callable);

	ClassDB::bind_static_method("CallableEasing", D_METHOD("create", "callable"), &CallableEasing::create);
}

CallableEasing::CallableEasing(const Callable &p_callable) {
	set_callable(p_callable);
}

CallableEasing::CallableEasing() {
	ERR_FAIL_MSG("CallableEasing can't be created directly. Use the create() method.");
}
