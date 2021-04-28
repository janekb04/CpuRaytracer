#ifndef SAVE_RENDER_DIALOG_H
#define SAVE_RENDER_DIALOG_H
#include <filesystem>
#include <string>
#include <nfd.hpp>
#include <array>
#include <stb_image_write.h>
#include <boxer/boxer.h>

#include "framebuffer.h"
#include "pixel.h"

inline bool save_render_dialog(const framebuffer& fb)
{
    struct file_format
    {
        using func_t = bool(*)(const std::string&, const framebuffer&);
        const char* friendly_name;
    	const char* extension_list;
        func_t write_func;
    };
    constexpr static auto to_pixels = [](const framebuffer& fb)
    {
        auto pixels = std::make_unique_for_overwrite<pixel[]>(fb.height() * fb.width());
        for (auto pixel_idx = 0; pixel_idx < fb.width() * fb.height(); ++pixel_idx)
        {
            pixels[pixel_idx] = pixel{ fb.buffer().data[pixel_idx] }.to_rgba();
        }
        return pixels;
    };
    constexpr static std::array<file_format, 5> formats{ {
        {
            "Portable Network Graphics",
            "png",
            [](const std::string& path, const framebuffer& fb) -> bool
            {
                return stbi_write_png(path.c_str(), fb.width(), fb.height(), 4, to_pixels(fb).get(),0);
            }
        },
        {
			"Bitmap",
            "bmp,dib",
            [](const std::string& path, const framebuffer& fb) -> bool
            {
                return stbi_write_bmp(path.c_str(), fb.width(), fb.height(), 4, to_pixels(fb).get());
            }
        },
        {
			"TARGA",
            "tga,icb,vda,vst",
            [](const std::string& path, const framebuffer& fb) -> bool
            {
                return stbi_write_tga(path.c_str(), fb.width(), fb.height(), 4, to_pixels(fb).get());
            }
        },
        {
			"RGBE",
            "hdr",
            [](const std::string& path, const framebuffer& fb) -> bool
            {
                return stbi_write_hdr(path.c_str(), fb.width(), fb.height(), 4, &fb.buffer().data->x);
            }
        },
        {
            "JPEG",
            "jpg,jpeg,jpe,jif,jfif,jfi",
            [](const std::string& path, const framebuffer& fb) -> bool
            {
                return stbi_write_jpg(path.c_str(), fb.width(), fb.height(), 4, to_pixels(fb).get(), 100);
            }
        }
    } };
	// MSVC complains about constexpr, but Clang compiles fine
    /* constexpr */ static auto supported_extensions = []()
    {
        std::array<nfdfilteritem_t, formats.size()> supported_extensions{};
        for (int i = 0; i < formats.size(); ++i)
        {
            supported_extensions[i] = { formats[i].friendly_name, formats[i].extension_list };
        }
        return supported_extensions;
    }();
	
    NFD::UniquePathU8 save_path_string;
    while (SaveDialog(save_path_string, supported_extensions.data(), supported_extensions.size(), nullptr, "render.png") == NFD_OKAY)
    {
        std::filesystem::path save_path{ save_path_string.get() };
        const auto extension = save_path.extension().string().substr(1);
        for (const auto& format : formats)
        {
            if (std::strstr(format.extension_list, extension.c_str()) != nullptr)
            {
                return format.write_func(save_path.string(), fb);
            }
        }
        const auto selection = show("Unsupported image format chosen. Please choose one of the supported image formats", "Unsupported format", boxer::Style::Warning, boxer::Buttons::OKCancel);
        if (selection == boxer::Selection::Cancel)
            break;
    }
    return false;
}
#endif // SAVE_RENDER_DIALOG_H
