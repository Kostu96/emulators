#pragma once
#include "emu_common/graphics/texture.hpp"
#include "emu_common/gui/button.hpp"

#include <SDL.h>
#include <algorithm>

using EmuCommon::Vec2i;

class LabeledLED
{
public:
    LabeledLED(const char* text, EmuCommon::SDLFont& font, EmuCommon::SDLTexture& ledTexture) :
        m_label{ font, text },
        m_LEDTexture{ ledTexture } {
        m_label.setColor({ 0xF2, 0xF1, 0xED });

        m_size.x = std::max(m_label.getSize().x, m_LEDTexture.getSize().x / 2 / LED_SCALE);
        m_size.y = m_label.getSize().y + m_LEDTexture.getSize().y / LED_SCALE + 2;
    }

    void setPosition(EmuCommon::Vec2u position) {
        m_position = position;
        unsigned int xPos = position.x + ((m_size.x == m_label.getSize().x) ? 0 : (m_size.x - m_label.getSize().x) / 2);
        m_label.setPosition({ (float)xPos, (float)position.y });
    }

    EmuCommon::Vec2u getSize() const { return m_size; }

    void render(SDL_Renderer* renderer) {
        m_label.render(renderer);
        const unsigned int width = m_LEDTexture.getSize().x / 2;
        const unsigned int scaledWidth = width / LED_SCALE;
        const unsigned int height = m_LEDTexture.getSize().y;
        const SDL_Rect srcRect{ int(m_status ? width : 0), 0, int(width), int(height) };
        int xPos = m_position.x + ((m_size.x == scaledWidth) ? 0 : (m_size.x - scaledWidth) / 2);
        const SDL_Rect dstRect{ xPos, int(m_position.y + m_label.getSize().y + 2),
            int(scaledWidth), int(height / LED_SCALE) };
        SDL_RenderCopy(renderer, m_LEDTexture.getHandle(), &srcRect, &dstRect);
    }
private:
    static constexpr unsigned int LED_SCALE = 4;

    bool m_status = false;
    EmuCommon::Vec2u m_position;
    EmuCommon::Vec2u m_size;
    EmuCommon::SDLText m_label;
    EmuCommon::SDLTexture& m_LEDTexture;
};

class TwoWayButton {
public:
    TwoWayButton(const char* upText, const char* downText, EmuCommon::SDLFont& font, EmuCommon::SDLTexture& texture, Vec2i position = {}) :
        m_texture{ texture },
        m_upLabel{ font, upText },
        m_downLabel{ font, downText },
        m_position{ position }
    {
        m_width = std::max({
            unsigned int((texture.getSize().x / 3) / SWITCH_SCALE),
            m_upLabel.getSize().x,
            m_downLabel.getSize().x,
        }) + 4;
        if (*upText) {
            m_upButton.setPosition({ 0, 0 });
            m_upButton.setSize({ float(m_width), ((texture.getSize().y / 2) / SWITCH_SCALE / 2) + m_upLabel.getSize().y });
        }
        if (*downText) {
            m_downButton.setPosition({ 0, m_upButton.getSize().y + (int((texture.getSize().y / 2) / SWITCH_SCALE) / 2) - 3 });
            m_downButton.setSize({  float(m_width), ((texture.getSize().y / 2) / SWITCH_SCALE / 2) + m_downLabel.getSize().y });
        }

        m_upLabel.setColor({ 0xF2, 0xF1, 0xED });
        m_upLabel.setPosition({ float((m_width - m_upLabel.getSize().x) / 2), 0 });
        m_upLabel.setAlign(EmuCommon::SDLText::Align::Center);
        m_downLabel.setColor({ 0xF2, 0xF1, 0xED });
        m_downLabel.setPosition({
            float((m_width - m_downLabel.getSize().x) / 2),
            m_downButton.getPosition().y + m_downButton.getSize().y - m_downLabel.getSize().y - 2
        });
        m_downLabel.setAlign(EmuCommon::SDLText::Align::Center);
        
        m_upButton.setPressedCallback([this]() { m_switchPos = 1; });
        m_upButton.setReleasedCallback([this]() { m_switchPos = 0; });

        m_downButton.setPressedCallback([this]() { m_switchPos = 2; });
        m_downButton.setReleasedCallback([this]() { m_switchPos = 0; });
    }

    void onEvent(SDL_Event& e) {
        m_upButton.onEvent(e);
        m_downButton.onEvent(e);
    }

    void handleMousePos(EmuCommon::Vec2i position) {
        m_upButton.handleMousePos({ position.x - m_position.x, position.y - m_position.y });
        m_downButton.handleMousePos({ position.x - m_position.x, position.y - m_position.y });
    }

    void render(SDL_Renderer* renderer) {
        EmuCommon::Transform transform;
        transform.translate(EmuCommon::Vec2f{ m_position });

        m_upLabel.render(renderer, transform);
        m_upButton.render(renderer, transform);
        m_downLabel.render(renderer, transform);
        m_downButton.render(renderer, transform);

        const int width = m_texture.getSize().x / 3;
        const float scaledWidth = width / SWITCH_SCALE;
        const int height = m_texture.getSize().y;
        const SDL_Rect srcRect{ m_switchPos * width, 0, width, height };
        const SDL_FRect dstRect{ (float)m_position.x + ((float)m_width - scaledWidth) / 2.f, (float)m_position.y + m_upLabel.getSize().y / 2, scaledWidth, height / SWITCH_SCALE};
        SDL_RenderCopyF(renderer, m_texture.getHandle(), &srcRect, &dstRect);
    }

    void setPosition(Vec2i position) { m_position = position; }
    int getWidth() const { return m_width; }
private:
    static constexpr float SWITCH_SCALE = 3.f;

    EmuCommon::SDLTexture& m_texture;
    EmuCommon::SDLText m_upLabel;
    EmuCommon::SDLText m_downLabel;
    int m_width;
    EmuCommon::GUI::InvisibleButton m_upButton;
    EmuCommon::GUI::InvisibleButton m_downButton;
    int m_switchPos = 0;
    Vec2i m_position;
    bool m_is2line;
};

class LabeledSwitch
{
public:
    void onEvent(SDL_Event& /*e*/) {

    }

    void handleMousePos(EmuCommon::Vec2i /*position*/) {

    }

    void render(SDL_Renderer* /*renderer*/) {

    }
private:

};
