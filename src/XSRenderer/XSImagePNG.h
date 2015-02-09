#pragma once

namespace XS {

	namespace Renderer {

		// load a PNG file
		// caller must free
		uint8_t *LoadPNG(
			const char *filename,
			uint32_t *outWidth = nullptr,
			uint32_t *outHeight = nullptr
		);

		// write a PNG file to filename
		bool WritePNG(
			const char *filename,
			uint8_t *pixels,
			int w,
			int h,
			int numChannels = 4
		);

	} // namespace Renderer

} // namespace XS
