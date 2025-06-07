#include "cpu_trail_2d.h"

#include "core/core_string_names.h"
#include "core/math/geometry_2d.h"

void CPUTrail2D::set_enabled(bool p_enabled) {
	if (enabled == p_enabled) {
		return;
	}

	enabled = p_enabled;

	if (enabled) {
		points[0].pos = parent->get_global_position();
	} else {
		tail = 0;
		head = 0;
		time = 0.0;
	}

	set_process_internal(p_enabled);
}

bool CPUTrail2D::is_enabled() const {
	return enabled;
}

void CPUTrail2D::set_lifetime(double p_lifetime) {
	ERR_FAIL_COND_MSG(p_lifetime <= 0, "Trail lifetime must be greater than 0.");
	lifetime = p_lifetime;
}

double CPUTrail2D::get_lifetime() const {
	return lifetime;
}

void CPUTrail2D::set_max_points(int p_max_points) {
	max_points = MAX(2, p_max_points);
	points.resize(max_points);
	vertex_buffer.resize(max_points * 2);
	color_buffer.resize(max_points * 2);
	uv_buffer.resize(max_points * 2);
	index_buffer.resize((max_points - 1) * 6);
}

int CPUTrail2D::get_max_points() const {
	return max_points;
}

void CPUTrail2D::set_threshold(float p_threshold) {
	threshold = MAX(0.0, p_threshold);
}

float CPUTrail2D::get_threshold() const {
	return threshold;
}

void CPUTrail2D::set_head_width(float p_width) {
	head_width = MAX(0.0, p_width);
}

float CPUTrail2D::get_head_width() const {
	return head_width;
}

void CPUTrail2D::set_tail_width(float p_width) {
	tail_width = MAX(0.0, p_width);
}

float CPUTrail2D::get_tail_width() const {
	return tail_width;
}

void CPUTrail2D::set_default_color(Color p_color) {
	default_color = p_color;
}

Color CPUTrail2D::get_default_color() const {
	return default_color;
}

void CPUTrail2D::set_gradient(const Ref<Gradient> &p_gradient) {
	gradient = p_gradient;

	if (gradient.is_null()) {
		_reset_color_buffer();
	}
}

Ref<Gradient> CPUTrail2D::get_gradient() const {
	return gradient;
}

void CPUTrail2D::set_texture(const Ref<Texture2D> &p_texture) {
	texture = p_texture;

	if (texture.is_valid()) {
		texture_rid = texture->get_rid();
		tile_aspect = texture->get_size().aspect();
	}
}

Ref<Texture2D> CPUTrail2D::get_texture() const {
	return texture;
}

void CPUTrail2D::set_texture_mode(const TrailTextureMode p_mode) {
	texture_mode = p_mode;
}

CPUTrail2D::TrailTextureMode CPUTrail2D::get_texture_mode() const {
	return texture_mode;
}

void CPUTrail2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			parent = Object::cast_to<Node2D>(get_parent());
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			_update_internal();
		} break;

		case NOTIFICATION_DRAW: {
			_draw();
		} break;
	}
}

// TODO: record the tail slope for consistency.
void CPUTrail2D::_draw() {
	ERR_FAIL_COND(_size() < 2 || head_width + tail_width <= 0.0 || tile_aspect <= 0.0);

	const float tail_half_width = tail_width / 2.0;
	const float head_half_width = head_width / 2.0;
	
	float half_width = tail_half_width;
	float distance = 0.0;
	float uvx = 0.0;

	{
		const Point &p0 = _get_point(0);
		const Point &p1 = _get_point(1);

		Vector2 dir = (p1.pos - p0.pos).normalized();
		Vector2 v1 = p1.pos + Vector2(dir.y, -dir.x) * half_width;
		Vector2 v2 = p1.pos + Vector2(-dir.y, dir.x) * half_width;

		vertex_buffer.write[0] = v1;
		vertex_buffer.write[1] = v2;

		if (gradient.is_valid()) {
			Color color = gradient->get_color(0);
			color_buffer.write[0] = color;
			color_buffer.write[1] = color;
		}

		uv_buffer.write[0] = Vector2(0.0, 0.0);
		uv_buffer.write[1] = Vector2(0.0, 1.0);
	}

	for (int i = 1; i < _size(); i++) {
		const Point &p0 = _get_point(i - 1);
		const Point &p1 = _get_point(i);

		distance += p0.pos.distance_to(p1.pos);
		half_width = Math::lerp(tail_half_width, head_half_width, distance / length);

		Vector2 dir = (p1.pos - p0.pos).normalized();
		Vector2 v1 = p1.pos + Vector2(dir.y, -dir.x) * half_width;
		Vector2 v2 = p1.pos + Vector2(-dir.y, dir.x) * half_width;

		const int j = i * 2;

		vertex_buffer.write[j] = v1;
		vertex_buffer.write[j + 1] = v2;

		if (gradient.is_valid()) {
			Color color = gradient->get_color_at_offset(distance / length);
			color_buffer.write[j] = color;
			color_buffer.write[j + 1] = color;
		}

		if (texture_mode == TRAIL_TEXTURE_TILE) {
			uvx = distance / (half_width * 2.0 * tile_aspect);
		} else if (texture_mode == TRAIL_TEXTURE_STRETCH) {
			uvx = distance / length;
		}

		if (texture_mode != TRAIL_TEXTURE_NONE) {
			uv_buffer.write[j] = Vector2(uvx, 0.0);
			uv_buffer.write[j + 1] = Vector2(uvx, 1.0);
		}

		int k = (i - 1) * 6;

        index_buffer.write[k++] = j - 2;
        index_buffer.write[k++] = j + 1;
        index_buffer.write[k++] = j - 1;

        index_buffer.write[k++] = j - 2;
        index_buffer.write[k++] = j;
        index_buffer.write[k++] = j + 1;
	}

	RS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			index_buffer,
			vertex_buffer,
			color_buffer,
			uv_buffer, Vector<int>(), Vector<float>(),
			texture_rid);
}

void CPUTrail2D::_reset_color_buffer() {
	color_buffer.fill(default_color);
}

// TODO: evenly fill the point span in case of a sudden jump.
void CPUTrail2D::_update_internal() {
	time += get_process_delta_time();

	// Tail
	if (_size() > 0 && _get_point(0).exp_time <= time) {
		length -= _get_point(0).pos.distance_to(_get_point(1).pos);
		tail++;
		queue_redraw();
	}

	// Head
	Vector2 global_pos = parent->get_global_position();

	if (_size() < max_points && global_pos.distance_squared_to(_get_point(_size() - 1).pos) > threshold * threshold) {
		head++;

		int index = head % max_points;
		points[index].pos = global_pos;
		points[index].exp_time = time + lifetime;

		length += _get_point(_size() - 2).pos.distance_to(_get_point(_size() - 1).pos);

		if (_size() == max_points) {
			length -= _get_point(0).pos.distance_to(_get_point(1).pos);
			tail++;
		}

		queue_redraw();
	}
}

void CPUTrail2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_length"), &CPUTrail2D::get_length);

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &CPUTrail2D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &CPUTrail2D::is_enabled);

	ClassDB::bind_method(D_METHOD("set_lifetime", "secs"), &CPUTrail2D::set_lifetime);
	ClassDB::bind_method(D_METHOD("get_lifetime"), &CPUTrail2D::get_lifetime);

	ClassDB::bind_method(D_METHOD("set_max_points", "max_points"), &CPUTrail2D::set_max_points);
	ClassDB::bind_method(D_METHOD("get_max_points"), &CPUTrail2D::get_max_points);

	ClassDB::bind_method(D_METHOD("set_threshold", "threshold"), &CPUTrail2D::set_threshold);
	ClassDB::bind_method(D_METHOD("get_threshold"), &CPUTrail2D::get_threshold);

	ClassDB::bind_method(D_METHOD("set_head_width", "width"), &CPUTrail2D::set_head_width);
	ClassDB::bind_method(D_METHOD("get_head_width"), &CPUTrail2D::get_head_width);

	ClassDB::bind_method(D_METHOD("set_tail_width", "width"), &CPUTrail2D::set_tail_width);
	ClassDB::bind_method(D_METHOD("get_tail_width"), &CPUTrail2D::get_tail_width);

	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &CPUTrail2D::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &CPUTrail2D::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "gradient"), &CPUTrail2D::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &CPUTrail2D::get_gradient);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &CPUTrail2D::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &CPUTrail2D::get_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &CPUTrail2D::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &CPUTrail2D::get_texture_mode);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lifetime", PROPERTY_HINT_RANGE, "0.01,600.0,0.01,or_greater,suffix:s"), "set_lifetime", "get_lifetime");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_points", PROPERTY_HINT_RANGE, "2,256,1"), "set_max_points", "get_max_points");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "threshold", PROPERTY_HINT_NONE, "suffix:px"), "set_threshold", "get_threshold");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_width", PROPERTY_HINT_NONE, "suffix:px"), "set_head_width", "get_head_width");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tail_width", PROPERTY_HINT_NONE, "suffix:px"), "set_tail_width", "get_tail_width");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Stretch"), "set_texture_mode", "get_texture_mode");

	BIND_ENUM_CONSTANT(TRAIL_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(TRAIL_TEXTURE_STRETCH);
}

CPUTrail2D::CPUTrail2D() {
	_reset_color_buffer();
	set_as_top_level(true);
}

CPUTrail2D::~CPUTrail2D() {
}
