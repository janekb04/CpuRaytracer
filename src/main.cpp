#define NOMINMAX

#include <iostream>
#include <iomanip>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <algorithm>
#include <array>
#include <filesystem>

#include "window.h"
#include "camera_controller.h"
#include "ray.h"
#include "utility.h"
#include "camera.h"
#include "framebuffer.h"
#include "scheduler.h"
#include "world.h"
#include "save_render_dialog.h"

class render_scheduler : public scheduler<render_scheduler> {
    friend class scheduler<render_scheduler>;

	window wnd;
    framebuffer fb;
    world world_;
    camera cam;
    orbit_camera_controller cam_controller{cam};
    size_t frameIdx = 0;
    std::atomic<double> productive_frame_time;
    double time = time_now();
    int worker_scanline_count{};
	
	struct worker_data
    {
        int offset_seed;
    };
    worker_data worker_init(size_t worker_idx)
    {
        return { init_seed() ^ static_cast<int>(worker_idx * 63748) };
    }
	void worker_run(size_t worker_idx, worker_data& data)
    {
	    auto time0 = time_now();
        const unsigned yBegin = worker_scanline_count * worker_idx;
    	const auto yEnd = std::min(yBegin + worker_scanline_count, wnd.height());
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

                const glm::vec3 newColor = world_.raytrace(r, 32, data.offset_seed);
                const glm::vec3 oldColor{ fb_buffer[y][x] };
                auto finalColor = glm::vec4{ newColor * weightNew + oldColor * weightOld, 1.0f };

                wnd_buffer[y][x] = pixel{ finalColor };
                fb_buffer[y][x] = finalColor;
            }
        }
    	
        productive_frame_time += (time_now() - time0);
    }
	
    bool main_run()
	{
		const bool should_run = wnd.update();

		const auto deltaTime = time_now() - time;
    	// Update frame time
        {
            time = time_now();
            const auto framesPerSecond = 1.0 / deltaTime;
            std::cout << framesPerSecond << " FPS, " << "Real: " << deltaTime * 1000.0 << "ms, Prod total: "
                << productive_frame_time * 1000.0 << "ms, Prod per thread: " << productive_frame_time * 1000.0 / worker_count() << "ms\n";
            productive_frame_time = 0.0;
            ++frameIdx;
        }
        // Update window and view
        {
            cam_controller.update(wnd, deltaTime);
            if (wnd.resized())
            {
                fb.update_size(wnd.width(), wnd.height());
            }
            worker_scanline_count = wnd.height() / worker_count() + (wnd.height() % worker_count() > 0);
        }
        // Save dialog
        if (wnd.is_key_pressed('p')) {
            save_render_dialog(fb);
        }
    	// Disable synchronization
        enable_synchronization = cam_controller.frames_still() <= 20;
    	
        return should_run;
    }
public:
    render_scheduler() :
        wnd{ "CPU Raytracer", 800, 608 }
    {
        fb.update_size(wnd.width(), wnd.height());
        if (NFD::Init() != NFD_OKAY)
        {
            throw std::runtime_error("Failed to initialize File Dialog library");
        }
    }

	void add(raytraceable* obj)
    {
        world_.add(obj);
    }
};


int main() {
    std::cout << std::setprecision(2) << std::fixed;

    render_scheduler mgr;

    const lambertian_material floor{ {0.7, 0.7, 0.7} };
    mgr.add(new single_sided<plane>(floor, transform{ {0, 0.05, 0}, {0,0,0}, {1,1,1} }));

    const dielectric_material glass{ 1.5f };
    mgr.add(new sphere(glass, transform{ {1.1, -1, 0},{0, 0, 0}, {1, 1, 1} }));
    mgr.add(new inverted_facing<sphere>(glass, transform{ {1.1, -1, 0},{0, 0, 0}, {0.95, 0.95, 0.95} }));

    const metallic_material gold{ {1.0f, 0.84f, 0.0f}, 0.0f };
    mgr.add(new sphere{ gold, {{ -1.1, -1, 0 }, { 0,0,0 }, { 1,1,1 }} });

    const lambertian_material wall1{ {0.7, 0.3, 0.3} };
    mgr.add(new rectangle(wall1, {{ 3, -1.45, -2 }, { degToRad(90.0f),degToRad(-45.0f),0 }, { 1,1,1.5 }}));
    const metallic_material wall2{ {0.95, 0.95, 0.95}, 0.03f };
    mgr.add(new rectangle(wall2, { { -3, -1.45, -2 }, { degToRad(90.0f),degToRad(45.0f),0 }, { 1,1,1.5 } }));

    const lambertian_material blue{ {0.2, 0.2, 0.6} };
    mgr.add(new sphere(blue, {{ 0, -5, -10 }, { 0, 0, 0 }, { 5, 5, 5 }}));

	mgr.run();
	
    return 0;
}