#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <cmath>
#include "window.h"
#include "camera.h"

class orbit_camera_controller {
    float lookSpeedH = 400;
    float lookSpeedV = 400;
    float fovChangeSpeed = 5.0;
    float dragSpeed = 170000;
    float exponent = 2;
    float flySpeed = 0.3;

    float yaw = 0;
    float pitch = 0;

    float fov = 70.0f;
	
    camera& cam;
    size_t frames_still_count = 0;

    static double sgn(double x) {
        if (x < 0.0) {
            return -1;
        }
        if (x > 0.0) {
            return 1.0;
        }
        return 0.0;
    }

public:
    explicit orbit_camera_controller(camera& cam) :
        cam{ cam } {
    }

    void update(const window& wnd, double delta_time) {
        bool moved = false;

        static auto [lx, ly] = wnd.mouse_pos();
        auto [nx, ny] = wnd.mouse_pos();
        double x = nx - lx, y = ny - ly;
        lx = nx;
        ly = ny;;

        auto [w, h] = wnd.size();
        x = sgn(x) * pow(x / h, exponent);
        y = sgn(y) * pow(y / h, exponent);

        if (wnd.is_mouse_button_pressed(MOUSE_MIDDLE) && (x != 0 || y != 0)) {
            cam.trans.translate_local(glm::vec3{ -x, -y, 0 } * static_cast<float>(delta_time) * dragSpeed);
            moved = true;
        }
        if (wnd.is_mouse_button_pressed(MOUSE_RIGHT) && (x != 0 || y != 0)) {
            yaw += lookSpeedH * -x;
            pitch += lookSpeedV * y;

            cam.trans.set_orientation({ pitch, yaw, 0 });
            moved = true;
        }

        const float scroll = wnd.get_scroll();
        if (scroll != 0) {
            fov -= scroll * fovChangeSpeed;
            moved = true;
        }

        glm::vec3 flyDir{};
        if (wnd.is_key_pressed('w')) flyDir.z += flySpeed;
        if (wnd.is_key_pressed('s')) flyDir.z -= flySpeed;
        if (wnd.is_key_pressed('a')) flyDir.x += flySpeed;
        if (wnd.is_key_pressed('d')) flyDir.x -= flySpeed;
        if (wnd.is_key_pressed('r')) flyDir.y += flySpeed;
        if (wnd.is_key_pressed('f')) flyDir.y -= flySpeed;
    	if (flyDir.x != 0 || flyDir.y != 0 || flyDir.z != 0)
    	{
            cam.trans.translate_local(-flyDir);
            moved = true;
    	}
    	
        if (moved || wnd.resized()) {
            frames_still_count = 0;
            cam.update(fov, wnd.width() / static_cast<float>(wnd.height()));
        }
        else {
            ++frames_still_count;
        }
    }

    [[nodiscard]] size_t frames_still() const {
        return frames_still_count;
    }
};

#endif //CAMERA_CONTROLLER_H
