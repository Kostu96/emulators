#pragma once
#include "emu_common/graphics/transformable.hpp"
#include "emu_common/graphics/renderable.hpp"
#include "emu_common/graphics/rect.hpp"

namespace EmuCommon {

	class SDLTexture;

	class SDLSprite :
		public Transformable,
		public Renderable
	{
	public:
		SDLSprite() = default;

		void setTexture(const SDLTexture& texture, bool resetRect = false);
		void setTextureRect(const IRect& rect) { m_textureRect = rect; }
		const SDLTexture* getTexture() const { return m_texture; }
		const IRect& getTextureRect() const { return m_textureRect; }

		void render(SDL_Renderer* renderer, const RenderStates& states) override;
	private:
		const SDLTexture* m_texture = nullptr;
		IRect m_textureRect{};
	};

} // EmuCommon
