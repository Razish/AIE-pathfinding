#pragma once

#include <vector>

#include "XSCommon/XSMatrix.h"
#include "XSRenderer/XSRenderCommand.h"
#include "XSRenderer/XSBuffer.h"

namespace XS {

	namespace Renderer {

		// forward declarations

		// used for pre/post render callbacks
		typedef void (*renderCallback_t)( void );

		struct View {
		private:
			uint32_t			width, height;
			renderCallback_t	callbackPreRender, callbackPostRender;

		public:
			matrix4						 projectionMatrix;
			matrix4						 viewMatrix;
			Backend::Buffer				*perFrameData;
			std::vector<RenderCommand>	 renderCommands;

			// construct a view, specifying additional callbacks if necessary
			View(
				uint32_t width,
				uint32_t height,
				renderCallback_t preRender = nullptr,
				renderCallback_t postRender = nullptr
			);

			// do not call publicly
			~View() {
				delete perFrameData;
			}

			// bind a view so that subsequent render calls are associated with it
			void Bind(
				void
			);

			// prepare the view for rendering, i.e. adding objects to the scene
			// do not call externally, set callbacks via constructor
			void PreRender(
				void
			);

			// post-process step if necessary
			// do not call externally, set callbacks via constructor
			void PostRender(
				void
			) const;
		};

	} // namespace Renderer

} // namespace XS
