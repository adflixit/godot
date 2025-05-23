#pragma once

#include "core/object/ref_counted.h"
#include "core/object/gdvirtual.gen.inc"

class Easing : public RefCounted {
	GDCLASS(Easing, RefCounted);

protected:
	static void _bind_methods();

	GDVIRTUAL4RC_REQUIRED(real_t, _ease, real_t, real_t, real_t, real_t)

public:
	virtual real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const;

	Easing() {}
	virtual ~Easing() {}
};

class EquationEasing : public Easing {
	GDCLASS(EquationEasing, Easing);

public:
	enum Equation {
		EQ_LINEAR,
		EQ_SINE_IN,
		EQ_SINE_OUT,
		EQ_SINE_IN_OUT,
		EQ_SINE_OUT_IN,
		EQ_QUINT_IN,
		EQ_QUINT_OUT,
		EQ_QUINT_IN_OUT,
		EQ_QUINT_OUT_IN,
		EQ_QUART_IN,
		EQ_QUART_OUT,
		EQ_QUART_IN_OUT,
		EQ_QUART_OUT_IN,
		EQ_QUAD_IN,
		EQ_QUAD_OUT,
		EQ_QUAD_IN_OUT,
		EQ_QUAD_OUT_IN,
		EQ_EXPO_IN,
		EQ_EXPO_OUT,
		EQ_EXPO_IN_OUT,
		EQ_EXPO_OUT_IN,
		EQ_ELASTIC_IN,
		EQ_ELASTIC_OUT,
		EQ_ELASTIC_IN_OUT,
		EQ_ELASTIC_OUT_IN,
		EQ_CUBIC_IN,
		EQ_CUBIC_OUT,
		EQ_CUBIC_IN_OUT,
		EQ_CUBIC_OUT_IN,
		EQ_CIRC_IN,
		EQ_CIRC_OUT,
		EQ_CIRC_IN_OUT,
		EQ_CIRC_OUT_IN,
		EQ_BOUNCE_IN,
		EQ_BOUNCE_OUT,
		EQ_BOUNCE_IN_OUT,
		EQ_BOUNCE_OUT_IN,
		EQ_BACK_IN,
		EQ_BACK_OUT,
		EQ_BACK_IN_OUT,
		EQ_BACK_OUT_IN,
		EQ_SPRING_IN,
		EQ_SPRING_OUT,
		EQ_SPRING_IN_OUT,
		EQ_SPRING_OUT_IN,
		EQ_MAX
	};

	enum TransitionType {
		TRANS_LINEAR,
		TRANS_SINE,
		TRANS_QUINT,
		TRANS_QUART,
		TRANS_QUAD,
		TRANS_EXPO,
		TRANS_ELASTIC,
		TRANS_CUBIC,
		TRANS_CIRC,
		TRANS_BOUNCE,
		TRANS_BACK,
		TRANS_SPRING,
		TRANS_MAX
	};

	enum EaseType {
		EASE_IN,
		EASE_OUT,
		EASE_IN_OUT,
		EASE_OUT_IN,
		EASE_MAX
	};

private:
	typedef real_t (*EasingFunc)(real_t p_t, real_t p_b, real_t p_c, real_t p_d);
	static EasingFunc equations[EQ_MAX];

	Equation equation;
	EasingFunc func = nullptr;

protected:
	static void _bind_methods();

public:
	static Ref<EquationEasing> create(Equation p_equation);
	static Equation to_equation(TransitionType p_trans, EaseType p_ease);
	void set_equation(Equation p_equation);
	Equation get_equation() const;
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	EquationEasing(Equation p_equation);
	EquationEasing();
};

VARIANT_ENUM_CAST(EquationEasing::Equation);
VARIANT_ENUM_CAST(EquationEasing::TransitionType);
VARIANT_ENUM_CAST(EquationEasing::EaseType);

class CallableEasing : public Easing {
	GDCLASS(CallableEasing, Easing);

	Callable callable;

protected:
	static void _bind_methods();

public:
	static Ref<CallableEasing> create(const Callable &p_callable);
	real_t ease(real_t p_t, real_t p_b, real_t p_c, real_t p_d) const override;

	CallableEasing(const Callable &p_callable);
	CallableEasing();
};
