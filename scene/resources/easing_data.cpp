#include "easing_data.h"

#include "scene/animation/cubic_bezier_easing.h"

Ref<Easing> EasingData::get_easing() const {
	return easing;
}

void EasingData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_easing"), &EasingData::get_easing);
}

EasingData::EasingData() {}

void EquationEasingData::_update() {
	EquationEasing::Equation equation = EquationEasing::to_equation(transition_type, ease_type);
	if (easing.is_null()) {
		easing = EquationEasing::create(equation);
	} else {
		Ref<EquationEasing> equation_easing = Ref<EquationEasing>(easing);
		equation_easing->set_equation(equation);
	}
}

void EquationEasingData::set_transition_type(EquationEasing::TransitionType p_transition_type) {
	transition_type = p_transition_type;
	_update();
}

EquationEasing::TransitionType EquationEasingData::get_transition_type() const {
	return transition_type;
}

void EquationEasingData::set_ease_type(EquationEasing::EaseType p_ease_type) {
	ease_type = p_ease_type;
	_update();
}

EquationEasing::EaseType EquationEasingData::get_ease_type() const {
	return ease_type;
}

void EquationEasingData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_transition_type", "transition_type"), &EquationEasingData::set_transition_type);
	ClassDB::bind_method(D_METHOD("get_transition_type"), &EquationEasingData::get_transition_type);
	ClassDB::bind_method(D_METHOD("set_ease_type", "ease_type"), &EquationEasingData::set_ease_type);
	ClassDB::bind_method(D_METHOD("get_ease_type"), &EquationEasingData::get_ease_type);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "transition_type", PROPERTY_HINT_ENUM, "Linear,Sine,Quint,Quart,Quad,Expo,Elastic,Cubic,Circ,Bounce,Back,Spring"), "set_transition_type", "get_transition_type");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ease_type", PROPERTY_HINT_ENUM, "In,Out,InOut,OutIn"), "set_ease_type", "get_ease_type");
}

EquationEasingData::EquationEasingData() {}

void CubicBezierEasingData::_update() {
	if (!control_points.is_empty()) {
		Vector<double> args = control_points.split_floats(",");
		if (args.size() < 4) {
			if (easing.is_null()) {
				easing = CubicBezierEasing::create(args[0], args[1], args[2], args[3]);
			} else {
				Ref<CubicBezierEasing> cubic_bezier_easing = Ref<CubicBezierEasing>(easing);
				cubic_bezier_easing->init(args[0], args[1], args[2], args[3]);
			}
		}
	}
}

void CubicBezierEasingData::set_control_points(String p_control_points) {
	control_points = p_control_points;
	_update();
}

String CubicBezierEasingData::get_control_points() const {
	return control_points;
}

void CubicBezierEasingData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_control_points", "control_points"), &CubicBezierEasingData::set_control_points);
	ClassDB::bind_method(D_METHOD("get_control_points"), &CubicBezierEasingData::get_control_points);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "control_points"), "set_control_points", "get_control_points");
}

CubicBezierEasingData::CubicBezierEasingData() {}
