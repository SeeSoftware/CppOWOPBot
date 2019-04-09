//
// Created by Elias Daler
//

#ifndef IMGUI_SFML_H
#define IMGUI_SFML_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Joystick.hpp>

#if defined(WIN32)
	#define DllExport   __declspec( dllexport ) 
#else
	#define DllExport
#endif

namespace sf
{
    class Event;

    class RenderTarget;

    class RenderWindow;

    class Sprite;

    class Texture;

    class Window;
}

namespace ImGui
{
    namespace SFML
    {
		DllExport void Init(sf::RenderWindow &window, bool loadDefaultFont = true);
        DllExport void Init(sf::Window &window, sf::RenderTarget &target, bool loadDefaultFont = true);

        DllExport void ProcessEvent(const sf::Event &event);

        DllExport void Update(sf::RenderWindow &window, sf::Time dt);
        DllExport void Update(sf::Window &window, sf::RenderTarget &target, sf::Time dt);
        DllExport void Update(const sf::Vector2i &mousePos, const sf::Vector2f &displaySize, sf::Time dt);

        DllExport void Render(sf::RenderTarget &target);

        DllExport void Shutdown();

        DllExport void UpdateFontTexture();
        DllExport sf::Texture &GetFontTexture();

        // joystick functions
        DllExport void SetActiveJoystickId(unsigned int joystickId);
        DllExport void SetJoytickDPadThreshold(float threshold);
        DllExport void SetJoytickLStickThreshold(float threshold);

        DllExport void SetJoystickMapping(int action, unsigned int joystickButton);
        DllExport void SetDPadXAxis(sf::Joystick::Axis dPadXAxis, bool inverted = false);
        DllExport void SetDPadYAxis(sf::Joystick::Axis dPadYAxis, bool inverted = false);
        DllExport void SetLStickXAxis(sf::Joystick::Axis lStickXAxis, bool inverted = false);
        DllExport void SetLStickYAxis(sf::Joystick::Axis lStickYAxis, bool inverted = false);
    }

// custom ImGui widgets for SFML stuff

// Image overloads
    DllExport void Image(const sf::Texture &texture,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);
    DllExport void Image(const sf::Texture &texture, const sf::Vector2f &size,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);
    DllExport void Image(const sf::Texture &texture, const sf::FloatRect &textureRect,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);
    DllExport void Image(const sf::Texture &texture, const sf::Vector2f &size, const sf::FloatRect &textureRect,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);

    DllExport void Image(const sf::Sprite &sprite,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);
    DllExport void Image(const sf::Sprite &sprite, const sf::Vector2f &size,
               const sf::Color &tintColor = sf::Color::White,
               const sf::Color &borderColor = sf::Color::Transparent);

// ImageButton overloads
    DllExport bool ImageButton(const sf::Texture &texture, const int framePadding = -1,
                     const sf::Color &bgColor = sf::Color::Transparent,
                     const sf::Color &tintColor = sf::Color::White);
    DllExport bool ImageButton(const sf::Texture &texture, const sf::Vector2f &size, const int framePadding = -1,
                     const sf::Color &bgColor = sf::Color::Transparent, const sf::Color &tintColor = sf::Color::White);

    DllExport bool ImageButton(const sf::Sprite &sprite, const int framePadding = -1,
                     const sf::Color &bgColor = sf::Color::Transparent,
                     const sf::Color &tintColor = sf::Color::White);
    DllExport bool ImageButton(const sf::Sprite &sprite, const sf::Vector2f &size, const int framePadding = -1,
                     const sf::Color &bgColor = sf::Color::Transparent,
                     const sf::Color &tintColor = sf::Color::White);

// Draw_list overloads. All positions are in relative coordinates (relative to top-left of the current window)
    DllExport void DrawLine(const sf::Vector2f &a, const sf::Vector2f &b, const sf::Color &col, float thickness = 1.0f);
    DllExport void DrawRect(const sf::FloatRect &rect, const sf::Color &color, float rounding = 0.0f, int rounding_corners = 0x0F,
                  float thickness = 1.0f);
    DllExport void DrawRectFilled(const sf::FloatRect &rect, const sf::Color &color, float rounding = 0.0f,
                        int rounding_corners = 0x0F);
}

#endif //# IMGUI_SFML_H

