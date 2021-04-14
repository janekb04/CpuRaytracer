#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "transform.h"
#include "window.h"
#include <cmath>

class orbit_camera_controller {
    float lookSpeedH = 400;
    float lookSpeedV = 400;
    float zoomSpeed = 0.1;
    float dragSpeed = 170000;
    float exponent = 2;
    float flySpeed = 0.3;

    float yaw = 0;
    float pitch = 0;

    transform& cam;
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
    explicit orbit_camera_controller(transform& cam) :
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
            cam.translate_local(glm::vec3{ -x, -y, 0 } *static_cast<float>(delta_time) * dragSpeed);
            moved = true;
        }
        if (wnd.is_mouse_button_pressed(MOUSE_RIGHT) && (x != 0 || y != 0)) {
            yaw += lookSpeedH * -x;
            pitch += lookSpeedV * y;

            cam.set_orientation({ pitch, yaw, 0 });
            moved = true;
        }

        const float scroll = wnd.get_scroll();
        if (scroll != 0) {
            cam.translate_local(glm::vec3{ 0, 0, -1 } * scroll * zoomSpeed);
            moved = true;
        }

        glm::vec3 flyDir{};
        if (wnd.is_key_pressed(mfb_key::KB_KEY_W)) flyDir.z += flySpeed;
        if (wnd.is_key_pressed(mfb_key::KB_KEY_S)) flyDir.z -= flySpeed;
        if (wnd.is_key_pressed(mfb_key::KB_KEY_A)) flyDir.x += flySpeed;
        if (wnd.is_key_pressed(mfb_key::KB_KEY_D)) flyDir.x -= flySpeed;
        if (wnd.is_key_pressed(mfb_key::KB_KEY_R)) flyDir.y += flySpeed;
        if (wnd.is_key_pressed(mfb_key::KB_KEY_F)) flyDir.y -= flySpeed;
    	if (flyDir.x != 0 || flyDir.y != 0 || flyDir.z != 0)
    	{
            cam.translate_local(-flyDir);
            moved = true;
    	}
    	
        if (moved) {
            frames_still_count = 0;
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
