#include "cpu_trail_2d.h"

void CPUTrail2D::set_emitting(bool p_emitting) {
	if (emitting == p_emitting) {
		return;
	}

	emitting = p_emitting;

	if (emitting) {
		initialize();
	}

	set_process_internal(emitting && parent);
	queue_redraw();
}

bool CPUTrail2D::is_emitting() const {
	return emitting;
}

void CPUTrail2D::set_lifetime(double p_lifetime) {
	ERR_FAIL_COND_MSG(p_lifetime <= 0, "Trail lifetime must be greater than 0.");
	lifetime = p_lifetime;
	queue_redraw();
}

double CPUTrail2D::get_lifetime() const {
	return lifetime;
}

void CPUTrail2D::set_max_points(int p_max_points) {
	max_points = MAX(2, p_max_points);
	_update_buffers();
	queue_redraw();
}

int CPUTrail2D::get_max_points() const {
	return max_points;
}

void CPUTrail2D::set_threshold(float p_threshold) {
	threshold = MAX(0.0, p_threshold);
	queue_redraw();
}

float CPUTrail2D::get_threshold() const {
	return threshold;
}

void CPUTrail2D::set_tail_width(float p_width) {
	tail_width = MAX(0.0, p_width);
	queue_redraw();
}

float CPUTrail2D::get_tail_width() const {
	return tail_width;
}

void CPUTrail2D::set_head_width(float p_width) {
	head_width = MAX(0.0, p_width);
	queue_redraw();
}

float CPUTrail2D::get_head_width() const {
	return head_width;
}

void CPUTrail2D::set_default_color(Color p_color) {
	default_color = p_color;
	queue_redraw();
}

Color CPUTrail2D::get_default_color() const {
	return default_color;
}

void CPUTrail2D::set_gradient(const Ref<Gradient> &p_gradient) {
	gradient = p_gradient;

	if (gradient.is_null()) {
		_reset_color_buffer();
	}
	queue_redraw();
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
	queue_redraw();
}

Ref<Texture2D> CPUTrail2D::get_texture() const {
	return texture;
}

void CPUTrail2D::set_texture_mode(const TrailTextureMode p_mode) {
	texture_mode = p_mode;
	queue_redraw();
}

CPUTrail2D::TrailTextureMode CPUTrail2D::get_texture_mode() const {
	return texture_mode;
}

void CPUTrail2D::initialize() {
	tail_index = 0;
	points_num = 0;
	time = 0.0;
	length = 0.0;
}

PackedStringArray CPUTrail2D::get_configuration_warnings() const {
	PackedStringArray warnings = Node2D::get_configuration_warnings();

	Node2D *node = Object::cast_to<Node2D>(get_parent());
	if (node == nullptr) {
		warnings.push_back(RTR("CPUTrail2D is only attachable to a Node2D derived node."));
	}

	return warnings;
}

void CPUTrail2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED: {
			parent = Object::cast_to<Node2D>(get_parent());
		} break;

		case NOTIFICATION_ENTER_TREE: {
			set_process_internal(emitting && parent);

			if (point_buffer.is_empty()) {
				_update_buffers();
			}
			if (gradient.is_null()) {
				_reset_color_buffer();
			}
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			_update_internal();
		} break;

		case NOTIFICATION_DRAW: {
			_draw();
		} break;
	}
}

void CPUTrail2D::_update_buffers() {
	point_buffer.resize(max_points);
	vertex_buffer.resize(max_points * 2);
	color_buffer.resize(max_points * 2);
	uv_buffer.resize(max_points * 2);
	index_buffer.resize((max_points - 1) * 6);
}

void CPUTrail2D::_reset_color_buffer() {
	color_buffer.fill(default_color);
}

// TODO: evenly fill the point span in case of a sudden jump.
void CPUTrail2D::_update_internal() {
	if (!parent) {
		return;
	}

	time += get_process_delta_time();

	Vector2 global_pos = get_global_position();

	if (points_num == 0) {
		points_num++;
		point_buffer[0].pos = global_pos;
		point_buffer[0].exp_time = time + lifetime;
	} else if (points_num > 1 && _get_point(0).exp_time < time) {
		tail_index++;
		points_num--;
		length -= _get_point(0).pos.distance_to(_get_point(1).pos);

		queue_redraw();
	}

	if (global_pos.distance_squared_to(_get_point(points_num - 1).pos) > threshold * threshold) {
		if (points_num == max_points) {
			length -= _get_point(0).pos.distance_to(_get_point(1).pos);
			tail_index++;
		} else {
			points_num++;
		}

		int index = (tail_index + points_num) % max_points;
		point_buffer[index].pos = global_pos;
		point_buffer[index].exp_time = time + lifetime;

		length += global_pos.distance_to(_get_point(points_num - 1).pos);

		queue_redraw();
	}
}

// TODO: record the tail slope for consistency.
void CPUTrail2D::_draw() {
	if (points_num < 2 || head_width + tail_width <= 0.0 || tile_aspect <= 0.0) {
		return;
	}

	const float tail_half_width = tail_width / 2.0;
	const float head_half_width = head_width / 2.0;

	Vector2 offset = _get_point(points_num - 1).pos + get_position();

	float half_width = tail_half_width;
	float distance = 0.0;
	float uvx = 0.0;

	for (int i = 0; i < points_num; i++) {
		Vector2 p0 = _get_point(i < points_num - 1 ? i : i - 1).pos - offset;
		Vector2 p1 = _get_point(i < points_num - 1 ? i + 1 : i).pos - offset;

		if (i < points_num - 1) {
			distance += p0.distance_to(p1);
		}
		half_width = Math::lerp(tail_half_width, head_half_width, distance / length);

		Vector2 dir = (i < points_num - 1 ? p1 - p0 : p0 - p1).normalized();
		Vector2 v1 = p0 + Vector2(dir.y, -dir.x) * half_width;
		Vector2 v2 = p0 + Vector2(-dir.y, dir.x) * half_width;

		const int j = i * 2;

		{
			Vector2 *w = vertex_buffer.ptrw();
			w[j] = v1;
			w[j + 1] = v2;
		}

		if (gradient.is_valid()) {
			Color color = gradient->get_color_at_offset(distance / length);
			Color *w = color_buffer.ptrw();
			w[j] = color;
			w[j + 1] = color;
		}

		if (texture_mode == TRAIL_TEXTURE_TILE) {
			uvx = distance / (half_width * 2.0 * tile_aspect);
		} else if (texture_mode == TRAIL_TEXTURE_STRETCH) {
			uvx = distance / length;
		}

		if (texture_mode != TRAIL_TEXTURE_NONE) {
			Vector2 *w = uv_buffer.ptrw();
			w[j] = Vector2(uvx, 0.0);
			w[j + 1] = Vector2(uvx, 1.0);
		}

		if (i < points_num - 1) {
			int k = i * 6;
			int *w = index_buffer.ptrw();

			w[k++] = j - 2;
			w[k++] = j + 1;
			w[k++] = j - 1;

			w[k++] = j - 2;
			w[k++] = j;
			w[k++] = j + 1;
		}
	}

	{
		const int last_index = (points_num - 1) * 6;
		int *w = index_buffer.ptrw();

		for (int i = last_index + 1; i < index_buffer.size(); i++) {
			w[i] = index_buffer[last_index];
		}
	}

	RS::get_singleton()->canvas_item_add_triangle_array(
			get_canvas_item(),
			index_buffer,
			vertex_buffer,
			color_buffer,
			uv_buffer, Vector<int>(), Vector<float>(),
			texture_rid);
}

void CPUTrail2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize"), &CPUTrail2D::get_length);
	ClassDB::bind_method(D_METHOD("get_length"), &CPUTrail2D::get_length);

	ClassDB::bind_method(D_METHOD("set_emitting", "emitting"), &CPUTrail2D::set_emitting);
	ClassDB::bind_method(D_METHOD("is_emitting"), &CPUTrail2D::is_emitting);

	ClassDB::bind_method(D_METHOD("set_lifetime", "secs"), &CPUTrail2D::set_lifetime);
	ClassDB::bind_method(D_METHOD("get_lifetime"), &CPUTrail2D::get_lifetime);

	ClassDB::bind_method(D_METHOD("set_max_points", "max_points"), &CPUTrail2D::set_max_points);
	ClassDB::bind_method(D_METHOD("get_max_points"), &CPUTrail2D::get_max_points);

	ClassDB::bind_method(D_METHOD("set_threshold", "threshold"), &CPUTrail2D::set_threshold);
	ClassDB::bind_method(D_METHOD("get_threshold"), &CPUTrail2D::get_threshold);

	ClassDB::bind_method(D_METHOD("set_tail_width", "width"), &CPUTrail2D::set_tail_width);
	ClassDB::bind_method(D_METHOD("get_tail_width"), &CPUTrail2D::get_tail_width);

	ClassDB::bind_method(D_METHOD("set_head_width", "width"), &CPUTrail2D::set_head_width);
	ClassDB::bind_method(D_METHOD("get_head_width"), &CPUTrail2D::get_head_width);

	ClassDB::bind_method(D_METHOD("set_default_color", "color"), &CPUTrail2D::set_default_color);
	ClassDB::bind_method(D_METHOD("get_default_color"), &CPUTrail2D::get_default_color);

	ClassDB::bind_method(D_METHOD("set_gradient", "gradient"), &CPUTrail2D::set_gradient);
	ClassDB::bind_method(D_METHOD("get_gradient"), &CPUTrail2D::get_gradient);

	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &CPUTrail2D::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &CPUTrail2D::get_texture);

	ClassDB::bind_method(D_METHOD("set_texture_mode", "mode"), &CPUTrail2D::set_texture_mode);
	ClassDB::bind_method(D_METHOD("get_texture_mode"), &CPUTrail2D::get_texture_mode);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "emitting"), "set_emitting", "is_emitting");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "lifetime", PROPERTY_HINT_RANGE, "0.01,600.0,0.01,or_greater,suffix:s"), "set_lifetime", "get_lifetime");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_points", PROPERTY_HINT_RANGE, "2,256,1"), "set_max_points", "get_max_points");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "threshold", PROPERTY_HINT_NONE, "suffix:px"), "set_threshold", "get_threshold");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tail_width", PROPERTY_HINT_NONE, "suffix:px"), "set_tail_width", "get_tail_width");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "head_width", PROPERTY_HINT_NONE, "suffix:px"), "set_head_width", "get_head_width");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "default_color"), "set_default_color", "get_default_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "gradient", PROPERTY_HINT_RESOURCE_TYPE, "Gradient"), "set_gradient", "get_gradient");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "texture_mode", PROPERTY_HINT_ENUM, "None,Tile,Stretch"), "set_texture_mode", "get_texture_mode");

	BIND_ENUM_CONSTANT(TRAIL_TEXTURE_NONE);
	BIND_ENUM_CONSTANT(TRAIL_TEXTURE_TILE);
	BIND_ENUM_CONSTANT(TRAIL_TEXTURE_STRETCH);
}

CPUTrail2D::CPUTrail2D() {
}

CPUTrail2D::~CPUTrail2D() {
}
