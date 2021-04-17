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
#include "framebuffer.h"
#include "scheduler.h"
#include "world.h"

class render_scheduler : public scheduler<render_scheduler> {
    friend class scheduler<render_scheduler>;

	window wnd;
    framebuffer fb;
    world world_;
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
        return { init_seed() ^ static_cast<int>(worker_idx * 1321) };
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
        auto wnd_buffer = wnd.buffer();
        auto fb_buffer = fb.buffer();
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

                const glm::vec3 newColor = world_.raytrace(r, 4, data.offset_seed);
                const glm::vec4 oldColor{ fb_buffer[y][x] };
                auto finalColor = glm::vec4(newColor * weightNew, 1.0) + oldColor * weightOld;

                wnd_buffer[y][x] = pixel{ finalColor };
                fb_buffer[y][x] = finalColor;
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
        fb.update_size(wnd.width(), wnd.height());
    	
        worker_scanline_count = wnd.height() / worker_count();
        return wnd.update();
    }
public:
    render_scheduler() :
        wnd{ "test", 800, 608 }
    {
        world_.add(new sphere{glm::vec3(0, -1, 0), 1});
        world_.add(new plane);
    }
};


int main() {
    std::cout << std::setprecision(2) << std::fixed;

    render_scheduler mgr{};
    mgr.run();

    return 0;
}