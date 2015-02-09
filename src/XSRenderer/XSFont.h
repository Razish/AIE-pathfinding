#pragma once

#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "freetype/ftglyph.h"

#include "XSCommon/XSVector.h"

namespace XS {

	namespace Renderer {

		class Texture;
		struct Material;

		struct FontData {
			vector2		size;
			vector2		s, t;
			vector2		offset;
			float		advance;
		};

		class Font {
		private:
			Texture		*texture;
			Material	*material;

		public:
			std::string	file;
			std::string	name;
			uint16_t	size;
			FontData	data[256];
			float		lineHeight;

			// don't allow default instantiation
			Font() = delete;
			Font( const Font& ) = delete;
			Font& operator=( const Font& ) = delete;

			// register a new font
			Font(
				const char *name,
				uint16_t size
			);

			// generate a glyph atlas for the font at the current size.
			void RenderGlyphs(
				void
			);

			// draw a string at the given position
			void Draw(
				const vector2 &pos,
				const std::string &text
			);

			// return the number of lines it would take to draw (for linefeeds and wrapping)
			uint32_t GetTextLineCount(
				const vector2 &pos,
				const std::string &text
			);

			// return the pixel width of the specified character
			float GetGlyphWidth(
				char c
			) const;

			// initialise the font system (i.e. freetype)
			static void Init(
				void
			);

			// free all font resources
			static void Shutdown(
				void
			);

			// register a new font, or be returned an existing one
			static Font *Register(
				const char *name,
				uint16_t size
			);
		};

	} // namespace Renderer

} // namespace XS
