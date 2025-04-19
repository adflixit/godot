#ifndef TRAIL_MESH_H
#define TRAIL_MESH_H

#include "core/templates/local_vector.h"
#include "scene/resources/mesh.h"

class TrailMesh : public Mesh {
	GDCLASS(TrailMesh, Mesh)

	RID mesh;

	bool uses_colors = false;
	bool uses_normals = false;
	bool uses_tangents = false;
	bool uses_uvs = false;
	bool uses_uv2s = false;

	Color current_color;
	Vector3 current_normal;
	Plane current_tangent;
	Vector2 current_uv;
	Vector2 current_uv2;

	LocalVector<Color> colors;
	LocalVector<Vector3> normals;
	LocalVector<Plane> tangents;
	LocalVector<Vector2> uvs;
	LocalVector<Vector2> uv2s;
	LocalVector<Vector3> vertices;

	struct Surface {
		PrimitiveType primitive;
		Ref<Material> material;
		bool vertex_2d = false;
		int array_len = 0;
		uint64_t format = 0;
		AABB aabb;
	};

	LocalVector<Surface> surfaces;

	bool surface_active = false;
	Surface active_surface_data;

	Vector<uint8_t> surface_vertex_create_cache;
	Vector<uint8_t> surface_attribute_create_cache;

	const Vector3 SMALL_VEC3 = Vector3(CMP_EPSILON, CMP_EPSILON, CMP_EPSILON);

protected:
	static void _bind_methods();

public:
	void surface_begin(PrimitiveType p_primitive, const Ref<Material> &p_material = Ref<Material>());
	void surface_set_color(const Color &p_color);
	void surface_set_normal(const Vector3 &p_normal);
	void surface_set_tangent(const Plane &p_tangent);
	void surface_set_uv(const Vector2 &p_uv);
	void surface_set_uv2(const Vector2 &p_uv2);
	void surface_add_vertex(const Vector3 &p_vertex);
	void surface_add_vertex_2d(const Vector2 &p_vertex);
	void surface_end();

	void clear_surfaces();

	virtual int get_surface_count() const override;
	virtual int surface_get_array_len(int p_idx) const override;
	virtual int surface_get_array_index_len(int p_idx) const override;
	virtual Array surface_get_arrays(int p_surface) const override;
	virtual TypedArray<Array> surface_get_blend_shape_arrays(int p_surface) const override;
	virtual Dictionary surface_get_lods(int p_surface) const override;
	virtual BitField<ArrayFormat> surface_get_format(int p_idx) const override;
	virtual PrimitiveType surface_get_primitive_type(int p_idx) const override;
	virtual void surface_set_material(int p_idx, const Ref<Material> &p_material) override;
	virtual Ref<Material> surface_get_material(int p_idx) const override;
	virtual int get_blend_shape_count() const override;
	virtual StringName get_blend_shape_name(int p_index) const override;
	virtual void set_blend_shape_name(int p_index, const StringName &p_name) override;

	virtual AABB get_aabb() const override;

	virtual RID get_rid() const override;

	TrailMesh();
	~TrailMesh();
};

#endif // TRAIL_MESH_H
