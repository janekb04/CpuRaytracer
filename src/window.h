//
// Created by Jan on 12-Apr-21.
//

#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <MiniFB.h>
#include "array_wrapper.h"

struct pixel {
    uint8_t b, g, r, a;

    constexpr pixel() = default;

    constexpr pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept:
            b{b},
            g{g},
            r{r},
            a{a} {
    }

    constexpr explicit pixel(glm::vec4 col) noexcept:
            b{static_cast<uint8_t>(col.b * 255.999f)},
            g{static_cast<uint8_t>(col.g * 255.999f)},
            r{static_cast<uint8_t>(col.r * 255.999f)},
            a{static_cast<uint8_t>(col.a * 255.999f)} {
    }

    explicit operator glm::vec4() const noexcept {
        return glm::vec4{r, g, b, a} / 255.999f;
    }
};

class window {
private:
    uint32_t m_width;
    uint32_t m_height;
    std::unique_ptr<pixel[]> m_buffer;
    mfb_window *m_handle;
    bool mouse_button_pressed[3]{};
    float scrollDx{}, scrollDy{};

    static void resize(mfb_window *handle, int width, int height) {
        window &self = *static_cast<window *>(mfb_get_user_data(handle));
        self.m_width = width;
        self.m_height = height;
        self.m_buffer = std::make_unique<pixel[]>(width * height);
    }

    static void mouse_button(mfb_window *handle, mfb_mouse_button btn, mfb_key_mod mod, bool is_pressed) {
        window &self = *static_cast<window *>(mfb_get_user_data(handle));
        if (btn > 0 && btn < 4) {
            self.mouse_button_pressed[btn - 1] = is_pressed;
        }
    }

    static void mouse_scroll(mfb_window *handle, mfb_key_mod mod, float dx, float dy) {
        window &self = *static_cast<window *>(mfb_get_user_data(handle));
        self.scrollDx = dx;
        self.scrollDy = dy;
    }

public:

    window(const char *title, uint32_t width, uint32_t height, mfb_window_flags flags = WF_RESIZABLE) :
            m_width{width},
            m_height{height},
            m_buffer{std::make_unique<pixel[]>(width * height)},
            m_handle{mfb_open_ex(title, width, height, flags)} {
        if (!m_handle) {
            throw std::runtime_error("Cannot open window");
        }
        mfb_set_resize_callback(m_handle, resize);
        mfb_set_mouse_button_callback(m_handle, mouse_button);
        mfb_set_mouse_scroll_callback(m_handle, mouse_scroll);
        mfb_set_user_data(m_handle, this);
    }

    bool update() {
        scrollDx = scrollDy = 0.0f;
        mfb_update_state state = mfb_update_ex(m_handle, m_buffer.get(), m_width, m_height);
        return state == STATE_OK;
    }

    void sync() const {
        mfb_wait_sync(m_handle);
    }

    [[nodiscard]] uint32_t width() const { return m_width; };

    [[nodiscard]] uint32_t height() const { return m_height; };

    [[nodiscard]] auto size() const { return std::tuple{m_width, m_height}; }

    [[nodiscard]] auto mouse_pos() const { return std::tuple{mfb_get_mouse_x(m_handle), mfb_get_mouse_y(m_handle)}; }

    [[nodiscard]] bool is_mouse_button_pressed(size_t button) const {
        assert(button <= 3);
        return mouse_button_pressed[button];
    }

    [[nodiscard]] float get_scroll() const { return scrollDy; }

    [[nodiscard]] auto buffer() const {
        return array_wrapper<pixel, 2>(m_buffer.get(), m_height, m_width);
    };
};

#endif //ENGINE_WINDOW_H
