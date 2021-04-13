//
// Created by Jan on 12-Apr-21.
//

#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include "transform.h"
#include "window.h"
#include <cmath>

class orbit_camera {
private:
    float lookSpeedH = 400;
    float lookSpeedV = 400;
    float zoomSpeed = 0.1;
    float dragSpeed = 170000;
    float exponent = 2;

    float yaw = 0;
    float pitch = 0;

    transform &cam;
    size_t frames_still_count = 0;

private:
    static double sgn(double x) {
        if (x < 0.0) {
            return -1;
        } else if (x == 0) {
            return 0.0;
        }
        return 1.0;
    }

public:
    explicit orbit_camera(transform &cam) :
            cam{cam} {
    }

public:
    void update(const window &wnd, double delta_time) {
        bool moved = false;

        static auto[lx, ly] = wnd.mouse_pos();
        auto[nx, ny] = wnd.mouse_pos();
        double x = nx - lx, y = ny - ly;
        lx = nx;
        ly = ny;;

        auto[w, h] = wnd.size();
        x = sgn(x) * pow(x / h, exponent);
        y = sgn(y) * pow(y / h, exponent);

        if (wnd.is_mouse_button_pressed(2)) {
            cam.translate_local(glm::vec3{-x, -y, 0} * (float) delta_time * dragSpeed);
            moved = true;
        }
        if (wnd.is_mouse_button_pressed(1)) {
            yaw += lookSpeedH * -x;
            pitch += lookSpeedV * y;

            cam.set_orientation({pitch, yaw, 0});
            moved = true;
        }

        auto scroll = wnd.get_scroll();
        if (scroll != 0) {
            cam.translate_local(glm::vec3{0, 0, -1} * (float) scroll * zoomSpeed);
            moved = true;
        }
        if (moved) {
            frames_still_count = 0;
        } else {
            ++frames_still_count;
        }
    }

    [[nodiscard]] size_t frames_still() const {
        return frames_still_count;
    }
};

#endif //ENGINE_CAMERA_H
