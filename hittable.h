#ifndef HITTABLE_H
#define HITTABLE_H

class hit_record {
  public:
    point3 p;
    vec3 normal;
    double t;
    bool front_face;
    // 用于区分外法线还是内法线，因为光线可能从球体内部击中球得表面，这时候的法线应该用指向圆心的法线（取反）
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the hit record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};
// 表示可击中
class hittable {
  public:
    virtual ~hittable() = default;

    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
};

#endif