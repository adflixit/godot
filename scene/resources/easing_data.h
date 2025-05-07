#pragma once

#include "core/io/resource.h"
#include "scene/animation/easing.h"

class EasingData : public Resource {
	GDCLASS(EasingData, Resource);

protected:
	Ref<Easing> easing;

	static void _bind_methods();

public:
	Ref<Easing> get_easing() const;

	EasingData();
};

class EquationEasingData : public EasingData {
	GDCLASS(EquationEasingData, EasingData);

	EquationEasing::TransitionType transition_type;
	EquationEasing::EaseType ease_type;

	void _update();

protected:
	static void _bind_methods();

public:
	void set_transition_type(EquationEasing::TransitionType p_trans);
	EquationEasing::TransitionType get_transition_type() const;

	void set_ease_type(EquationEasing::EaseType p_ease);
	EquationEasing::EaseType get_ease_type() const;

	EquationEasingData();
};

class CubicBezierEasingData : public EasingData {
	GDCLASS(CubicBezierEasingData, EasingData);

	Vector4 control_points;

	void _update();

protected:
	static void _bind_methods();

public:
	void set_control_points(const Vector4 &p_control_points);
	Vector4 get_control_points() const;

	CubicBezierEasingData();
};
