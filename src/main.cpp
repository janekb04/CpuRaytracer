#define NOMINMAX

#include <iostream>
#include <iomanip>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <algorithm>

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
        bool should_run = wnd.update();
    	
		auto deltaTime = time_now() - time;
        time = time_now();
		auto framesPerSecond = 1.0 / deltaTime;
        std::cout << framesPerSecond << " FPS, " << "Real: " << deltaTime * 1000.0 << "ms, Prod: "
            << productive_frame_time * 1000.0 << "ms\n";
        productive_frame_time = 0.0;
        ++frameIdx;

        cam_controller.update(wnd, deltaTime);
    	if (wnd.resized())
    	{
            fb.update_size(wnd.width(), wnd.height());
    	}
    	
        worker_scanline_count = wnd.height() / worker_count() + (wnd.height() % worker_count() > 0);
        return should_run;
    }
public:
    render_scheduler() :
        wnd{ "CPU Raytracer", 800, 608 }
    {
        fb.update_size(wnd.width(), wnd.height());
    }

	void add(raytraceable* obj)
    {
        world_.add(obj);
    }
};


int main() {
    std::cout << std::setprecision(2) << std::fixed;

    render_scheduler mgr;

    lambertian_material concrete{ {0.7f, 0.7f, 0.7f} };
    lambertian_material floor{ {0.2f, 0.6f, 0.2f} };
    metallic_material shiny{ {	1,1,1 }, 0.f };
    emmisive_material light{ glm::vec3{1,1,1} };
    mgr.add(new sphere(light, transform{{0, -1.4, 0},{degToRad(45.0f), degToRad(45.0f), degToRad(45.0f)}, {-2, 1, 1}}));
    mgr.add(new single_sided<plane>(concrete, transform{}));

    const auto right_portal_trans = transform{ {3,-2.f,-1},{degToRad(-45.0f), 0, degToRad(-90.0f)},{2,3,1} };
    const auto left_portal_trans = transform{ {-3,-2.f,-1},{degToRad(45.0f+180), 0, degToRad(-90.0f)},{2,3,1} };
    portal_material right_portal_mat{ right_portal_trans, left_portal_trans };
    portal_material left_portal_mat{ left_portal_trans, right_portal_trans };
	
    mgr.add(new rectangle(right_portal_mat, right_portal_trans));
    mgr.add(new rectangle(left_portal_mat, left_portal_trans));
    mgr.run();

    return 0;
}