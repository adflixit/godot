#pragma once

#include "node_2d.h"

class CPUTrail2D : public Node2D {
	GDCLASS(CPUTrail2D, Node2D);

public:
	enum TrailTextureMode {
		TRAIL_TEXTURE_NONE = 0,
		TRAIL_TEXTURE_TILE,
		TRAIL_TEXTURE_STRETCH
	};

	struct Point {
		Point() {}

		Vector2 pos;
		double exp_time = 0.0;
	};

	void initialize();
	_FORCE_INLINE_ float get_length() const { return length; }

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void set_lifetime(double p_lifetime);
	double get_lifetime() const;

	void set_max_points(int p_max_points);
	int get_max_points() const;

	void set_threshold(float p_threshold);
	float get_threshold() const;

	void set_tail_width(float p_width);
	float get_tail_width() const;

	void set_head_width(float p_width);
	float get_head_width() const;

	void set_default_color(Color p_color);
	Color get_default_color() const;

	void set_gradient(const Ref<Gradient> &p_gradient);
	Ref<Gradient> get_gradient() const;

	void set_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_texture() const;

	void set_texture_mode(const TrailTextureMode p_mode);
	TrailTextureMode get_texture_mode() const;

	PackedStringArray get_configuration_warnings() const override;

	CPUTrail2D();
	~CPUTrail2D();

protected:
	void _notification(int p_what);
	
	static void _bind_methods();

private:
	_FORCE_INLINE_ const Point &_get_point(int p_index) const { return point_buffer[(tail_index + p_index) % max_points]; }

	void _update_buffers();
	void _reset_color_buffer();
	void _update_internal();
	void _draw();

private:
	LocalVector<Point> point_buffer;
	Vector<Vector2> vertex_buffer;
	Vector<Color> color_buffer;
	Vector<Vector2> uv_buffer;
	Vector<int> index_buffer;

	Node2D *parent = nullptr;
	int tail_index = 0;
	int points_num = 0;
	double time = 0.0;
	double next_update = 0.0;
	float length = 0.0;

	bool enabled = true;
	double lifetime = 1.0;
	int max_points = 64;
	float threshold = 2.0;
	float head_width = 10.0;
	float tail_width = 10.0;
	Color default_color = Color(1, 1, 1);
	Ref<Gradient> gradient;
	Ref<Texture2D> texture;
	RID texture_rid;
	float tile_aspect = 1.0;
	TrailTextureMode texture_mode = TRAIL_TEXTURE_NONE;
};

VARIANT_ENUM_CAST(CPUTrail2D::TrailTextureMode)
