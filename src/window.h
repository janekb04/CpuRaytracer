#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <MiniFB.h>
#include "array_wrapper.h"
#include "pixel.h"

class window {
    uint32_t m_width;
    uint32_t m_height;
    std::unique_ptr<pixel[]> m_buffer;
    mfb_window* m_handle;
    bool mouse_button_pressed[8]{};
    float scrollDx{}, scrollDy{};
    bool pressed_keys[KB_KEY_LAST]{};
    bool m_resized = false;

    static void resize(mfb_window* handle, int width, int height) {
        window& self = *static_cast<window*>(mfb_get_user_data(handle));
        self.m_width = width;
        self.m_height = height;
        self.m_buffer = std::make_unique<pixel[]>(width * height);
        self.m_resized = true;
    }

    static void mouse_button(mfb_window* handle, mfb_mouse_button btn, mfb_key_mod mod, bool is_pressed) {
        window& self = *static_cast<window*>(mfb_get_user_data(handle));
        self.mouse_button_pressed[btn] = is_pressed;
    }

    static void mouse_scroll(mfb_window* handle, mfb_key_mod mod, float dx, float dy) {
        window& self = *static_cast<window*>(mfb_get_user_data(handle));
        self.scrollDx = dx;
        self.scrollDy = dy;
    }

    static void char_input(mfb_window* handle, unsigned char_code)
    {
        window& self = *static_cast<window*>(mfb_get_user_data(handle));
        self.pressed_keys[char_code] = true;
    }

public:

    window(const char* title, uint32_t width, uint32_t height, mfb_window_flags flags = WF_RESIZABLE) :
        m_width{ width },
        m_height{ height },
        m_buffer{ std::make_unique<pixel[]>(width * height) },
        m_handle{ mfb_open_ex(title, width, height, flags) } {
        if (!m_handle) {
            throw std::runtime_error("Cannot open window");
        }
        mfb_set_resize_callback(m_handle, resize);
        mfb_set_mouse_button_callback(m_handle, mouse_button);
        mfb_set_mouse_scroll_callback(m_handle, mouse_scroll);
        mfb_set_char_input_callback(m_handle, char_input);
        mfb_set_user_data(m_handle, this);
    }

    bool update() {
        m_resized = false;
        scrollDx = scrollDy = 0.0f;
        for (bool& key : pressed_keys) key = false;
        mfb_update_state state = mfb_update_ex(m_handle, m_buffer.get(), m_width, m_height);
        return state == STATE_OK;
    }

    void sync() const {
        mfb_wait_sync(m_handle);
    }

    [[nodiscard]] uint32_t width() const { return m_width; }

    [[nodiscard]] uint32_t height() const { return m_height; }

    [[nodiscard]] bool resized() const { return m_resized; }

    [[nodiscard]] auto size() const { return std::tuple{ m_width, m_height }; }

    [[nodiscard]] auto mouse_pos() const { return std::tuple{ mfb_get_mouse_x(m_handle), mfb_get_mouse_y(m_handle) }; }

    [[nodiscard]] bool is_mouse_button_pressed(mfb_mouse_button button) const {
        return mouse_button_pressed[button];
    }

	[[nodiscard]] bool is_key_pressed(int key) const
    {
        return pressed_keys[key];
    }

    [[nodiscard]] float get_scroll() const { return scrollDy; }

    [[nodiscard]] auto buffer() const {
        return array_wrapper<pixel, 2>(m_buffer.get(), m_height, m_width);
    };
};

#endif //ENGINE_WINDOW_H
