#include "header.hpp"
#include <sstream>
#include <cmath>
#include <iomanip>
#include <SFML/Window.hpp>

#if defined(__unix__) || defined(__unix) || __APPLE__
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>

extern "C" {
#include <xdo.h>
}
#else
#include <windows.h>
#endif

#define TOTAl_INPUT_TABLE_SIZE 256

namespace input {
int horizontal, vertical;
int osu_x, osu_y, osu_h, osu_v;
bool is_letterbox, is_left_handed;

std::string debugMessage;
std::string debugBitMessage;

sf::RectangleShape debugBackground;
sf::Font debugFont;
sf::Text debugText;

#if defined(__unix__) || defined(__unix) || __APPLE__
xdo_t* xdo;
Display* dpy;
Window foreground_window;

static int _XlibErrorHandler(Display *display, XErrorEvent *event) {
    return true;
}
#endif

int INPUT_KEY_TABLE[TOTAl_INPUT_TABLE_SIZE];

void init(std::shared_ptr<Cat> cat) {
    for (int i = 0; i < TOTAl_INPUT_TABLE_SIZE; i++) {
        if (i >= 48 && i <= 57) {           // number
            INPUT_KEY_TABLE[i] = i - 48 + (int)sf::Keyboard::Key::Num0;
        } else if (i >= 65 && i <= 90) {    // english alphabet
            INPUT_KEY_TABLE[i] = i - 65 + (int)sf::Keyboard::Key::A;
        } else if (i >= 96 && i <= 105) {   // numpad
            INPUT_KEY_TABLE[i] = i - 96 + (int)sf::Keyboard::Key::Numpad0;
        } else if (i >= 112 && i <= 126) {  // function
            INPUT_KEY_TABLE[i] = i - 112 + (int)sf::Keyboard::Key::F1;
        } else {
            INPUT_KEY_TABLE[i] = (int)sf::Keyboard::Key::Unknown;
        }
    }

    INPUT_KEY_TABLE[27] = (int)sf::Keyboard::Key::Escape;
    INPUT_KEY_TABLE[17] = (int)sf::Keyboard::Key::LControl;
    INPUT_KEY_TABLE[16] = (int)sf::Keyboard::Key::LShift;
    INPUT_KEY_TABLE[18] = (int)sf::Keyboard::Key::LAlt;
    INPUT_KEY_TABLE[17] = (int)sf::Keyboard::Key::RControl;
    INPUT_KEY_TABLE[16] = (int)sf::Keyboard::Key::RShift;
    INPUT_KEY_TABLE[18] = (int)sf::Keyboard::Key::RAlt;
    INPUT_KEY_TABLE[93] = (int)sf::Keyboard::Key::Menu;
    INPUT_KEY_TABLE[219] = (int)sf::Keyboard::Key::LBracket;
    INPUT_KEY_TABLE[221] = (int)sf::Keyboard::Key::RBracket;
    INPUT_KEY_TABLE[186] = (int)sf::Keyboard::Key::Semicolon;
    INPUT_KEY_TABLE[188] = (int)sf::Keyboard::Key::Comma;
    INPUT_KEY_TABLE[190] = (int)sf::Keyboard::Key::Period;
    INPUT_KEY_TABLE[222] = (int)sf::Keyboard::Key::Quote;
    INPUT_KEY_TABLE[191] = (int)sf::Keyboard::Key::Slash;
    INPUT_KEY_TABLE[220] = (int)sf::Keyboard::Key::Backslash;
    INPUT_KEY_TABLE[192] = (int)sf::Keyboard::Key::Tilde;
    INPUT_KEY_TABLE[187] = (int)sf::Keyboard::Key::Equal;
    INPUT_KEY_TABLE[189] = (int)sf::Keyboard::Key::Hyphen;
    INPUT_KEY_TABLE[32] = (int)sf::Keyboard::Key::Space;
    INPUT_KEY_TABLE[13] = (int)sf::Keyboard::Key::Enter;
    INPUT_KEY_TABLE[8] = (int)sf::Keyboard::Key::Backspace;
    INPUT_KEY_TABLE[9] = (int)sf::Keyboard::Key::Tab;
    INPUT_KEY_TABLE[33] = (int)sf::Keyboard::Key::PageUp;
    INPUT_KEY_TABLE[34] = (int)sf::Keyboard::Key::PageDown;
    INPUT_KEY_TABLE[35] = (int)sf::Keyboard::Key::End;
    INPUT_KEY_TABLE[36] = (int)sf::Keyboard::Key::Home;
    INPUT_KEY_TABLE[45] = (int)sf::Keyboard::Key::Insert;
    INPUT_KEY_TABLE[46] = (int)sf::Keyboard::Key::Delete;
    INPUT_KEY_TABLE[107] = (int)sf::Keyboard::Key::Add;
    INPUT_KEY_TABLE[109] = (int)sf::Keyboard::Key::Subtract;
    INPUT_KEY_TABLE[106] = (int)sf::Keyboard::Key::Multiply;
    INPUT_KEY_TABLE[111] = (int)sf::Keyboard::Key::Divide;
    INPUT_KEY_TABLE[37] = (int)sf::Keyboard::Key::Left;
    INPUT_KEY_TABLE[39] = (int)sf::Keyboard::Key::Right;
    INPUT_KEY_TABLE[38] = (int)sf::Keyboard::Key::Up;
    INPUT_KEY_TABLE[40] = (int)sf::Keyboard::Key::Down;
    INPUT_KEY_TABLE[19] = (int)sf::Keyboard::Key::Pause;
    INPUT_KEY_TABLE[189] = (int)sf::Keyboard::Key::Dash;

    is_letterbox = data::cfg["resolution"]["letterboxing"].asBool();
    osu_x = data::cfg["resolution"]["width"].asInt();
    osu_y = data::cfg["resolution"]["height"].asInt();
    osu_h = data::cfg["resolution"]["horizontalPosition"].asInt();
    osu_v = data::cfg["resolution"]["verticalPosition"].asInt();
    is_left_handed = data::cfg["decoration"]["leftHanded"].asBool();

#if defined(__unix__) || defined(__unix) || __APPLE__
    // Set x11 error handler
    XSetErrorHandler(_XlibErrorHandler);

    // Get desktop resolution
    int num_sizes;
    Rotation current_rotation;

    dpy = XOpenDisplay(NULL);
    Window root = RootWindow(dpy, 0);
    XRRScreenSize *xrrs = XRRSizes(dpy, 0, &num_sizes);

    XRRScreenConfiguration *conf = XRRGetScreenInfo(dpy, root);
    SizeID current_size_id = XRRConfigCurrentConfiguration(conf, &current_rotation);

    int current_width = xrrs[current_size_id].width;
    int current_height = xrrs[current_size_id].height;

    horizontal = current_width;
    vertical = current_height;

    xdo = xdo_new(NULL);
#else
    // getting resolution
    RECT desktop;
    const HWND h_desktop = GetDesktopWindow();
    GetWindowRect(h_desktop, &desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
#endif

    // loading font
    if (!debugFont.loadFromFile("font/RobotoMono-Bold.ttf")) {
        data::error_msg("Cannot find the font : RobotoMono-Bold.ttf", "Error loading font");
        exit(1);
    }

    // initialize debug resource
    debugBackground.setSize(sf::Vector2f(cat->window_width, cat->window_height));
    debugBackground.setFillColor(sf::Color(0, 0, 0, 128));

    debugText.setFont(debugFont);
    debugText.setCharacterSize(14);
    debugText.setFillColor(sf::Color::White);
    debugText.setPosition(10.0f, 4.0f);
    debugText.setString(debugMessage);
}

sf::Keyboard::Key ascii_to_key(int key_code) {
    if (key_code < 0 || key_code >= TOTAl_INPUT_TABLE_SIZE) {
        // out of range
        return sf::Keyboard::Unknown;
    } else {
        return (sf::Keyboard::Key)(INPUT_KEY_TABLE[key_code]);
    }
}

// for some special cases of num dot and such
bool is_pressed_fallback(int key_code) {
#if defined(__unix__) || defined(__unix) || __APPLE__ // code snippet from SFML
    KeyCode keycode = XKeysymToKeycode(dpy, key_code);
    if (keycode != 0) {
        char keys[32];
        XQueryKeymap(dpy, keys);
        return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
    }
    else {
        return false;
    }
#else
    return (GetAsyncKeyState(key_code) & 0x8000) != 0;
#endif
}

bool is_pressed(int key_code) {
    if (key_code == 16) {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);
    } else if (key_code == 17) {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl);
    } else {
        sf::Keyboard::Key selected = ascii_to_key(key_code);
        if (selected != sf::Keyboard::Key::Unknown) {
            return sf::Keyboard::isKeyPressed(selected);
        } else {
            return is_pressed_fallback(key_code);
        }
    }
}


std::pair<double, double> get_xy() {
#if defined(__unix__) || defined(__unix) || __APPLE__
    double letter_x, letter_y, s_height, s_width;
    bool found_window = (xdo_get_focused_window_sane(xdo, &foreground_window) == 0);

    if (found_window) {
        unsigned char* name_ret;
        int name_len_ret;
        int name_type;

        xdo_get_window_name(xdo, foreground_window, &name_ret, &name_len_ret, &name_type);
        bool can_get_name = (name_len_ret > 0);

        if (can_get_name) {

            std::string title = "";

            if (name_ret != NULL)
            {
                std::string foreground_title(reinterpret_cast<char*>(name_ret));
                title = foreground_title;
            }

            if (title.find("osu!") == 0) {
                if (!is_letterbox) {

                    int x_ret;
                    int y_ret;
                    unsigned int width_ret;
                    unsigned int height_ret;

                    bool can_get_location = (xdo_get_window_location(xdo, foreground_window, &x_ret, &y_ret, NULL) == 0);
                    bool can_get_size = (xdo_get_window_size(xdo, foreground_window, &width_ret, &height_ret) == 0);

                    bool can_get_rect = (can_get_location && can_get_size);

                    bool is_fullscreen_window = (horizontal == width_ret) && (vertical == height_ret);
                    bool should_not_resize_screen = (!can_get_rect || is_fullscreen_window);

                    if (should_not_resize_screen) {
                        s_width = horizontal;
                        s_height = vertical;

                        letter_x = 0;
                        letter_y = 0;
                    }
                    else {
                        s_height = osu_y * 0.8;
                        s_width = s_height * 4 / 3;

                        long left = x_ret;
                        long top = y_ret;
                        long right = left + width_ret;
                        long bottom = top + height_ret;

                        letter_x = left + ((right - left) - s_width) / 2;
                        letter_y = top + osu_y * 0.117;
                    }
                }
                else {
                    s_height = osu_y * 0.8;
                    s_width = s_height * 4 / 3;

                    double l = (horizontal - osu_x) * (osu_h + 100) / 200.0;
                    double r = l + osu_x;
                    letter_x = l + ((r - l) - s_width) / 2;
                    letter_y = (vertical - osu_y) * (osu_v + 100) / 200.0 + osu_y * 0.117;
                }
            }
            else {
                s_width = horizontal;
                s_height = vertical;
                letter_x = 0;
                letter_y = 0;
            }
        }
        else {
            s_width = horizontal;
            s_height = vertical;
            letter_x = 0;
            letter_y = 0;
        }
    }
    else {
        s_width = horizontal;
        s_height = vertical;
        letter_x = 0;
        letter_y = 0;
    }

    double x = 0, y = 0;
    int px = 0, py = 0;

    if (xdo_get_mouse_location(xdo, &px, &py, NULL) == 0) {

        if (!is_letterbox) {
            letter_x = floor(1.0 * px / osu_x) * osu_x;
            letter_y = floor(1.0 * py / osu_y) * osu_y;
        }

        double fx = (1.0 * px - letter_x) / s_width;

        if (is_left_handed) {
            fx = 1 - fx;
        }

        double fy = (1.0 * py - letter_y) / s_height;

        fx = std::min(fx, 1.0);
        fx = std::max(fx, 0.0);

        fy = std::min(fy, 1.0);
        fy = std::max(fy, 0.0);

        x = -97 * fx + 44 * fy + 184;
        y = -76 * fx - 40 * fy + 324;
    }
#else
    // getting device resolution
    double letter_x, letter_y, s_height, s_width;

    HWND handle = GetForegroundWindow();
    if (handle) {
        TCHAR w_title[256];
        GetWindowText(handle, w_title, GetWindowTextLength(handle));
        std::string title = w_title;
        if (title.find("osu!") == 0) {
            RECT oblong;
            GetWindowRect(handle, &oblong);
            s_height = osu_y * 0.8;
            s_width = s_height * 4 / 3;
            if (!is_letterbox) {
                letter_x = oblong.left + ((oblong.right - oblong.left) - s_width) / 2;
                letter_y = oblong.top + osu_y * 0.117;
            } else {
                double l = (horizontal - osu_x) * (osu_h + 100) / 200.0;
                double r = l + osu_x;
                letter_x = l + ((r - l) - s_width) / 2;
                letter_y = (vertical - osu_y) * (osu_v + 100) / 200.0 + osu_y * 0.117;
            }
        } else {
            s_width = horizontal;
            s_height = vertical;
            letter_x = 0;
            letter_y = 0;
        }
    } else {
        s_width = horizontal;
        s_height = vertical;
        letter_x = 0;
        letter_y = 0;
    }
    double x, y;
    POINT point;
    if (GetCursorPos(&point)) {
        if (!is_letterbox) {
            letter_x = floor(1.0 * point.x / osu_x) * osu_x;
            letter_y = floor(1.0 * point.y / osu_y) * osu_y;
        }
        double fx = (1.0 * point.x - letter_x) / s_width;
        if (is_left_handed) {
            fx = 1 - fx;
        }
        double fy = (1.0 * point.y - letter_y) / s_height;
        fx = std::min(fx, 1.0);
        fx = std::max(fx, 0.0);
        fy = std::min(fy, 1.0);
        fy = std::max(fy, 0.0);
        x = -97 * fx + 44 * fy + 184;
        y = -76 * fx - 40 * fy + 324;
    }
#endif

    return std::make_pair(x, y);
}

void drawDebugPanel() {
    std::stringstream result;
    
    result << "Joystick connected : " << std::endl;
    result << "Support button : " << std::endl;
 
    debugText.setString(result.str());

    window.draw(debugBackground);
    window.draw(debugText);
}

void cleanup() {
#if defined(__unix__) || defined(__unix) || __APPLE__
    delete xdo;
    XCloseDisplay(dpy);
#endif
}

};

