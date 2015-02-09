#include "XSCommon/XSCommon.h"
#include "XSCommon/XSCvar.h"
#include "XSCommon/XSCommand.h"
#include "XSRenderer/XSRenderer.h"
#include "XSRenderer/XSBackend.h"
#include "XSRenderer/XSScreenshot.h"

namespace XS {

	namespace Renderer {

		namespace Backend {

			GLuint defaultVao = 0u;
			GLuint defaultPbo = 0u;

			void Init( void ) {
				Command::AddCommand( "screenshot", Cmd_Screenshot );

				glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
				glClear( GL_COLOR_BUFFER_BIT );
				glClearDepth( 1.0f );

				// state changes
				glEnable( GL_DEPTH_TEST );
				glDepthFunc( GL_LEQUAL );

				// back-face culling
				glEnable( GL_CULL_FACE );
				glCullFace( GL_BACK );
				glFrontFace( GL_CCW );

				// alpha blending
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

				// activate the default vertex array object
				glGenVertexArrays( 1, &defaultVao );
				glBindVertexArray( defaultVao );

				glGenBuffers( 1, &defaultPbo );
				glBindBuffer( GL_PIXEL_PACK_BUFFER, defaultPbo );
				const int32_t w = vid_width->GetInt(), h = vid_height->GetInt();
				glBufferData( GL_PIXEL_PACK_BUFFER, 4 * w * h, NULL, GL_STREAM_COPY );
			}

			void Shutdown( void ) {
				glDeleteVertexArrays( 1, &defaultVao );
				glDeleteBuffers( 1, &defaultPbo );
			}

		} // namespace Backend

	} // namespace Renderer

} // namespace XS
