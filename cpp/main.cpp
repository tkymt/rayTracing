﻿#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"

#include <iostream>

color ray_color(ray r, const hittable &world)
{
	hit_record rec;
	long long absorption = 1;
	while (world.hit(r, 0.001, infinity, rec))
	{
		point3 target = rec.p + rec.normal + vec3::random_in_unit_sphere();
		r = ray(rec.p, target - rec.p);
		++absorption;
	}
	vec3 unit_direction = vec3::unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return ((1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0)) * std::pow(0.5, absorption);
}

color ray_color(const ray &r, const hittable &world, int depth)
{
	hit_record rec;

	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec))
	{
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
	vec3 unit_direction = vec3::unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene()
{
	hittable_list world;

	auto ground_material = std::make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(std::make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				std::shared_ptr<material> sphere_material;

				if (choose_mat < 0.8)
				{
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = std::make_shared<lambertian>(albedo);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = std::make_shared<metal>(albedo, fuzz);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					// glass
					sphere_material = std::make_shared<dielectric>(1.5);
					world.add(std::make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = std::make_shared<dielectric>(1.5);
	world.add(std::make_shared<sphere>(point3(0, 1, 0), 1.0, material));

	auto material2 = std::make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(std::make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = std::make_shared<metal>(color(0.7, 0.5, 0.5) 0.0);
	world.add(std::make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

int main()
{

	// Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 384;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	// World

	auto R = std::cos(pi / 4);
	hittable_list world;

	auto material_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = std::make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material_left = std::make_shared<dielectric>(1.5);
	auto material_right = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	world.add(std::make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	world.add(std::make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	world.add(std::make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
	world.add(std::make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));

	// Camera
	point3 lookfrom(3, 3, 2);
	point3 lookat(0, 0, -1);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = (lookfrom - lookat).length();
	auto aperture = 2.0;

	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	// Render
	std::cout << "P3\n"
			  << image_width << ' ' << image_height << "\n255\n";
	for (int j = image_height - 1; j >= 0; --j)
	{
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i)
		{
			color pixel_color{};
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
				//pixel_color += ray_color(r, world);
			}
			write_color(std::cout, pixel_color, samples_per_pixel);
		}
	}

	std::cerr << "\nDone.\n";
}
