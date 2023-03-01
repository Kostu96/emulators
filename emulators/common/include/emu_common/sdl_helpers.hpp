#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <string>

namespace EmuCommon {

	struct Vec2i { int x = 0, y = 0; };
	struct Vec2f { float x = 0.f, y = 0.f; };
	struct Color { uint8_t r = 0, g = 0, b = 0, a = 255; };

	class SDLTexture
	{
	public:
		SDLTexture() = default;
		explicit SDLTexture(const char* filename) { loadFromFile(filename); }
		~SDLTexture();

		bool loadFromFile(const char* filename);

		Vec2i getSize() const { return m_size; }

		operator SDL_Texture*() { return m_handle; }
	private:
		SDL_Texture* m_handle = nullptr;
		Vec2i m_size;
	};

	class SDLFont
	{
	public:
		SDLFont() = default;
		explicit SDLFont(const char* filename) { loadFromFile(filename); }
		~SDLFont();

		bool loadFromFile(const char* filename);
	private:
		static constexpr int DEFAULT_SIZE = 14;

		void setSize(int size);
		operator TTF_Font*() { return m_handle; }

		TTF_Font* m_handle = nullptr;

		friend class SDLText;
	};

	class SDLText
	{
	public:
		explicit SDLText(SDLFont& font, const char* text = "") : m_font{font} { setText(text); };
		~SDLText();

		void setText(const char* text);
		void setTextSize(unsigned int size);
		void setColor(Color color) { m_color = color; }
		void setPosition(Vec2i position) { m_position = position; }
		Vec2i getPosition() const { return m_position; }
		Vec2i getSize();

		void render(SDL_Renderer* renderer, Vec2i offset = { 0, 0 });
	private:
		SDLFont& m_font;
		std::string m_text{ "" };
		unsigned int m_textSize{ SDLFont::DEFAULT_SIZE };
		Color m_color{ 255, 255, 255, 255 };
		Vec2i m_position;
		Vec2i m_size;

		SDL_Texture* m_texture = nullptr;
		bool m_isTextureDirty = true;
		bool m_isSizeDirty = true;
		bool m_need2Texture = false;
		SDL_Texture* m_texture2 = nullptr;
		std::string m_str2;
	};

} // namespace EmuCommon
