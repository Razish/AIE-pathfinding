#pragma once

namespace XS {

	namespace Renderer {

		enum class InternalFormat {
			NONE = 0,
			R8,
			RGBA8,
			RGBA16F,
			DEPTH_COMPONENT16,
			DEPTH_COMPONENT24,
			STENCIL_INDEX4,
			STENCIL_INDEX8,
			DEPTH24_STENCIL8
		};

		// ???
		unsigned int GetGLInternalFormat(
			InternalFormat internalFormat
		);

		// ???
		unsigned int GetGLFormat(
			InternalFormat internalFormat
		);

		// retrieve the data type used to store texture colours
		unsigned int GetDataTypeForFormat(
			InternalFormat internalFormat
		);

	} // namespace Renderer

} // namespace XS
