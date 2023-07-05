#include "header.hpp"
#include <memory>
#define BONGO_ERROR 1

#if defined(__unix__) || defined(__unix) || __APPLE__
#include <unistd.h>
#include <limits.h>

extern "C" {
#include <SDL2/SDL.h>
}
#else
#include <windows.h>
#endif

const char *default_conf_string = 
R"V0G0N({
})V0G0N";

namespace data {
Json::Value cfg;
std::map<std::string, sf::Texture> img_holder;

void create_config() {
    std::string error;
    Json::CharReaderBuilder cfg_builder;
    Json::CharReader *cfg_reader = cfg_builder.newCharReader();
    cfg_reader->parse(default_conf_string, default_conf_string + strlen(default_conf_string), &cfg, &error);
    delete cfg_reader;
}

void error_msg(std::string error, std::string title) {
#if defined(__unix__) || defined(__unix) || __APPLE__

    SDL_MessageBoxButtonData buttons[] = {
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Retry" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
    };

    SDL_MessageBoxColorScheme colorScheme = {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 255, 255, 255 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            { 0, 0, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 0, 0, 0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            { 255, 255, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 128, 128, 128 }
        }
    };

    SDL_MessageBoxData messagebox_data = {
    	SDL_MESSAGEBOX_ERROR,
    	NULL,
    	title.c_str(),
    	error.c_str(),
    	SDL_arraysize(buttons),
    	buttons,
    	&colorScheme
    };

    int button_id;

    SDL_ShowMessageBox(&messagebox_data, &button_id);

    if (button_id == -1 || button_id == 1) {
#else
    if (MessageBoxA(NULL, error.c_str(), title.c_str(), MB_ICONERROR | MB_RETRYCANCEL) == IDCANCEL) {
#endif
        exit(BONGO_ERROR);
    }
}

bool update(Json::Value &cfg_default, Json::Value &cfg) {
    bool is_update = true;
    for (const auto &key : cfg.getMemberNames()) {
        if (cfg_default.isMember(key)) {
            if (cfg_default[key].type() != cfg[key].type()) {
                error_msg("Value type error in config.json", "Error reading configs");
                return false;
            }
            if (cfg_default[key].isArray() && !cfg_default[key].empty()) {
                for (Json::Value &v : cfg[key]) {
                    if (v.type() != cfg_default[key][0].type()) {
                        error_msg("Value type in array error in config.json", "Error reading configs");
                        return false;
                    }
                }
            }
            if (cfg_default[key].isObject()) {
                is_update &= update(cfg_default[key], cfg[key]);
            } else {
                cfg_default[key] = cfg[key];
            }
        } else {
            cfg_default[key] = cfg[key];
        }
    }
    return is_update;
}

sf::Texture &load_texture(std::string path) {
    if (img_holder.find(path) == img_holder.end()) {
        while (!img_holder[path].loadFromFile(path)) {
            error_msg("Cannot find file " + path, "Error importing images");
        }
    }
    return img_holder[path];
}
}; // namespace data
