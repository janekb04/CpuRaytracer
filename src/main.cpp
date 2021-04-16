#define NOMINMAX

#include <iostream>
#include <iomanip>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>

#include "window.h"
#include "camera_controller.h"
#include "ray.h"
#include "utility.h"
#include "camera.h"
#include "scheduler.h"

static std::optional<float> hit_plane(const glm::vec3 &n, const glm::vec3 &p0, const ray &r) {
	auto denom = glm::dot(n, glm::normalize(r.direction));
    if (denom > 1e-6) {
	    auto p0l0 = p0 - r.origin;
	    auto t = glm::dot(p0l0, n) / denom;
        if (t >= 0) {
            return t;
        }
        return std::nullopt;
    }

    return std::nullopt;
}

static std::optional<float> hit_sphere(const glm::vec3 &center, double radius, const ray &r) {
	auto oc = r.origin - center;
    auto a = 1.0f;
    auto half_b = dot(oc, r.direction);
    auto c = glm::dot(oc, oc) - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0) {
        return std::nullopt;
    } else {
        return (-half_b - sqrt(discriminant)) / a;
    }
}

static glm::vec4 ray_color(const ray &r) {
    auto ts = hit_sphere(glm::vec3(0, -1, -1), 0.5, r);
    auto tp = hit_plane(glm::vec3{0, 1, 0}, glm::vec3{0, 0, 0}, r);
    if (ts && (!tp || ts.value() < tp.value()) && ts.value() > 0) {
	    auto N = glm::normalize(r.at(ts.value()) - glm::vec3(0, 0, -1));
        return 0.5f * glm::vec4(N.x + 1, N.y + 1, N.z + 1, 1.0);
    } else if (tp) {
	    auto hit_point = abs(r.at(tp.value()));
        hit_point -= glm::ivec3{hit_point};
        hit_point = abs(hit_point);

        glm::vec4 ret{
                static_cast<float>(hit_point.x >= 0.9 || hit_point.x <= 0.1),
                0.0,
                static_cast<float>(hit_point.z >= 0.9 || hit_point.z <= 0.1),
                1.0
        };
        if (ret.r != 0 || ret.b != 0) return ret;
    }
    auto unit_direction = glm::normalize(r.direction);
    float t = 0.5 * (unit_direction.y + 1.0);
    return {(1.0f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0), 1.0};
}

class render_scheduler : public scheduler<render_scheduler> {
    friend class scheduler<render_scheduler>;

	window wnd;
    camera cam;
    orbit_camera_controller cam_controller{cam.trans};
    size_t frameIdx = 0;
    std::atomic<double> productive_frame_time;
    double time = time_now();
    int worker_scanline_count;
	
	struct worker_data
    {
        int offset_seed;
    };
    worker_data worker_init(size_t worker_idx)
    {
        return { 0x00269ec3 };
    }
	void worker_run(size_t worker_idx, worker_data& data)
    {
	    auto time0 = time_now();
        const int yBegin = worker_scanline_count * worker_idx;
        const auto yEnd = yBegin + worker_scanline_count;
        const float yMax = wnd.height() - 1;
        const auto xBegin = 0;
        const int xEnd = wnd.width();
        const float xMax = wnd.width() - 1;
        auto buffer = wnd.buffer();
        const auto weightNew = 1.0f / static_cast<float>(cam_controller.frames_still() + 1);
        const auto weightOld = 1.0f - weightNew;
        const auto pixelWidth = 1.0f / xMax;
        const auto pixelHeight = 1.0f / yMax;
        for (auto y = yBegin; y < yEnd; ++y) {
            for (auto x = xBegin; x < xEnd; ++x) {
                const auto off = sfrand(data.offset_seed) * weightOld;
                const auto u = x / xMax + off * pixelWidth;
                const auto v = y / yMax + off * pixelHeight;

                auto r = cam.get_ray(u, v);

                const glm::vec3 newColor = ray_color(r);
                const glm::vec4 oldColor{ buffer[y][x] };
                const auto finalColor = glm::vec4(newColor * weightNew, 1.0) + oldColor * weightOld;

                buffer[y][x] = pixel{ finalColor };
            }
        }
        productive_frame_time += (time_now() - time0);
    }
	
    bool main_run()
	{
		auto deltaTime = time_now() - time;
        time = time_now();
		auto framesPerSecond = 1.0 / deltaTime;
        std::cout << framesPerSecond << " FPS, " << "Real: " << deltaTime * 1000.0 << "ms, Prod: "
            << productive_frame_time * 1000.0 << "ms\n";
        productive_frame_time = 0.0;
        ++frameIdx;

        cam_controller.update(wnd, deltaTime);
        cam.update(70.0f, wnd.width() / static_cast<float>(wnd.height()));

        worker_scanline_count = wnd.height() / worker_count();
        return wnd.update();
    }
public:
    render_scheduler() :
        wnd{ "test", 800, 608 }
    {
    }
};


int main() {
    std::cout << std::setprecision(2) << std::fixed;

    render_scheduler mgr{};
    mgr.run();

    return 0;
}