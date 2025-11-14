#ifndef CAMERA_H
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
  public:
    double aspect_ratio = 1.0;  // image的宽高比
    int    image_width  = 100;  // image宽度
    int    samples_per_pixel = 10;   // 每个像素的采样个数（抗锯齿）
    double vfov = 90;  // fov（摄像机上下边界角度，用于定义视口宽度）
    int    max_depth         = 10;   // 光线弹射次数
    point3 lookfrom = point3(0,0,0);   // 相机位置
    point3 lookat   = point3(0,0,-1);  // 相机看的位置
    vec3   vup      = vec3(0,1,0);     // 相机 up，用于构建相机坐标系
    double defocus_angle = 0;  // 配合焦距定义一个散焦圆盘角度，使用时是把摄像起点随机放在这个圆盘上（圆盘中心为摄像机位置），模拟景深效果
    double focus_dist = 10;    // 焦距，在这个距离上的物体是清晰的
    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rScanlines remaining: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) { // 每个像素
                color pixel_color(0,0,0);
                for(int sample = 0;sample < samples_per_pixel;sample++) { // 每个采样点
                    ray r = get_ray(i, j);
                    pixel_color += ray_color(r, max_depth, world);
                }
                write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }

        std::clog << "\rDone.                 \n";
    }

  private:
    int    image_height;   // Rendered image height
    point3 center;         // 摄像机位置
    point3 pixel00_loc;    // 像素0，0的位置
    vec3   pixel_delta_u;  // Offset to pixel to the right
    vec3   pixel_delta_v;  // Offset to pixel below
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    vec3   u, v, w;              // 相机坐标系基向量

    vec3   defocus_disk_u;       // Defocus disk horizontal radius
    vec3   defocus_disk_v;       // Defocus disk vertical radius
    

    void initialize() {
        image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        pixel_samples_scale = 1.0 / samples_per_pixel;

        center = lookfrom;

        // 根据fov和Near计算视口宽高
        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta/2);
        auto viewport_height = 2 * h * focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);
        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.  构建正交坐标系
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
        vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge


        // viewport宽高是定义在构建的虚拟世界坐标的，而image的宽高是最后输出图片的数据，用viewport的宽高/image的宽高就是把连续的虚拟世界离散化，计算每个像素在虚拟世界的大小
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // 左上角第一个像素位置
        auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors.
        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }
    color ray_color(const ray& r, int depth, const hittable& world) const {
        // If we've exceeded the ray bounce limit, no more light is gathered.
        if (depth <= 0)
            return color(0,0,0);

        hit_record rec;
        // 设置最小t是防止自击中（浮点数误差导致的）
        if (world.hit(r, interval(0.001, infinity), rec)) {
            ray scattered;
            color attenuation;
            if (rec.mat->scatter(r, rec, attenuation, scattered))  // 用击中点的材质进行散射
                return attenuation * ray_color(scattered, depth-1, world);
            return color(0,0,0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return (1.0-a)*color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);
    }
    ray get_ray(int i, int j) const {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.

        auto offset = sample_square();
        auto pixel_sample = pixel00_loc
                          + ((i + offset.x()) * pixel_delta_u)
                          + ((j + offset.y()) * pixel_delta_v);

        // 没有景深时起点就是摄像机位置，但是有景深时起点是摄像机为中心的散焦圆盘上的随机点
        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }
    point3 defocus_disk_sample() const {
        // Returns a random point in the camera defocus disk.
        auto p = random_in_unit_disk();
        return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
    }
    vec3 sample_square() const {
        // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
        return vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }
};

#endif