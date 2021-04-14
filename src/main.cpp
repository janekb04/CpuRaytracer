#define NOMINMAX

#include <iostream>
#include <iomanip>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <thread>
#include <barrier>
#include <mutex>
#include <optional>

#include "window.h"
#include "camera_controller.h"
#include "duplex.h"
#include "ray.h"
#include "utility.h"
#include "viewport.h"

// Config
#define DO_THREAD_SYNC 1
#define DO_MAIN_THREAD_SYNC 1

static std::optional<float> hit_plane(const glm::vec3 &n, const glm::vec3 &p0, const ray &r) {
    float denom = glm::dot(n, glm::normalize(r.direction));
    if (denom > 1e-6) {
        glm::vec3 p0l0 = p0 - r.origin;
        float t = glm::dot(p0l0, n) / denom;
        if (t >= 0) {
            return t;
        }
        return std::nullopt;
    }

    return std::nullopt;
}

static std::optional<float> hit_sphere(const glm::vec3 &center, double radius, const ray &r) {
    glm::vec3 oc = r.origin - center;
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
        glm::vec3 N = glm::normalize(r.at(ts.value()) - glm::vec3(0, 0, -1));
        return 0.5f * glm::vec4(N.x + 1, N.y + 1, N.z + 1, 1.0);
    } else if (tp) {
        glm::vec3 hit_point = abs(r.at(tp.value()));
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
    glm::vec3 unit_direction = glm::normalize(r.direction);
    float t = 0.5 * (unit_direction.y + 1.0);
    return {(1.0f - t) * glm::vec3(1.0, 1.0, 1.0) + t * glm::vec3(0.5, 0.7, 1.0), 1.0};
}

struct render_task_manager {
    std::vector<std::thread> workers;
    size_t worker_count;
    window *wnd;
    transform trans;
    orbit_camera_controller cam{trans};
    size_t frameIdx = 0;
    bool should_run = true;
    duplex render_main_sync;
    std::atomic<double> productive_frame_time;
    double time = time_now();
    int worker_scanline_count;

    void render_task(size_t idx) {
#if DO_THREAD_SYNC == 1
        static std::barrier frame_border(worker_count, [&]() noexcept {
#if DO_MAIN_THREAD_SYNC == 1
            render_main_sync.release();
            render_main_sync.acquire();
#endif
        });
#endif

        int offset_seed = 0x00269ec3;
        while (should_run) {
            double time0 = time_now();
            const int yBegin = worker_scanline_count * idx;
            const int yEnd = yBegin + worker_scanline_count;
            const float yMax = wnd->height() - 1;
            const int xBegin = 0;
            const int xEnd = wnd->width();
            const float xMax = wnd->width() - 1;
            auto buffer = wnd->buffer();
            const auto VP = trans.to_mat4();
            const float weightNew = 1.0f / float(cam.frames_still() + 1);
            const float weightOld = 1.0f - weightNew;
            const float pixelWidth = 1.0f / xMax;
            const float pixelHeight = 1.0f / yMax;
            for (int y = yBegin; y < yEnd; ++y) {
                for (int x = xBegin; x < xEnd; ++x) {
                    const float off = sfrand(offset_seed) * weightOld;
                    const float u = x / xMax + off * pixelWidth;
                    const float v = y / yMax + off * pixelHeight;

                    ray r{
                    	trans.get_position(),
                    	glm::normalize(viewport.ray_from_uv(u, v))
                    };
                    r.direction = VP * glm::vec4{r.direction, 0.0};

                    const glm::vec3 newColor = ray_color(r);
                    const glm::vec4 oldColor{buffer[y][x]};
                    const glm::vec4 finalColor = glm::vec4(newColor * weightNew, 1.0) + oldColor * weightOld;

                    buffer[y][x] = pixel{finalColor};
                }
            }
            productive_frame_time += (time_now() - time0);
#if DO_THREAD_SYNC == 1
            frame_border.arrive_and_wait();
#endif
        }
    }

    void start(window &w) {
        worker_count = std::thread::hardware_concurrency();
        wnd = &w;
        worker_scanline_count = wnd->height() / worker_count;

        workers.reserve(worker_count);
        for (int i = 0; i < worker_count; ++i) {
            workers.emplace_back(&render_task_manager::render_task, this, i);
        }

        while (should_run) {
#if DO_THREAD_SYNC == 1 && DO_MAIN_THREAD_SYNC == 1
            std::unique_lock lk(render_main_sync);
#endif
            double deltaTime = time_now() - time;
            time = time_now();
            double framesPerSecond = 1.0 / deltaTime;
            //std::cout << framesPerSecond << " FPS, " << "Real: " << deltaTime * 1000.0 << "ms, Prod: "
            //    << productive_frame_time * 1000.0 << "ms\n";
            productive_frame_time = 0.0;
            ++frameIdx;

            cam.update(*wnd, deltaTime);
            viewport.resize(wnd->width(), wnd->height());

            should_run = wnd->update();
            worker_scanline_count = wnd->height() / worker_count;
        }

        for (auto &worker : workers) {
            worker.join();
        }
    }
};


int main() {
    std::cout << std::setprecision(2) << std::fixed;

    window wnd("test", 800, 608);

    render_task_manager mgr;
    mgr.start(wnd);

    return 0;
}