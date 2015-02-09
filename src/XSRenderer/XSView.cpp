#include "XSCommon/XSCommon.h"
#include "XSCommon/XSCvar.h"
#include "XSCommon/XSError.h"
#include "XSRenderer/XSRenderer.h"
#include "XSRenderer/XSBuffer.h"
#include "XSRenderer/XSView.h"
#include "XSRenderer/XSBackend.h"

namespace XS {

	namespace Renderer {

		View::View( uint32_t width, uint32_t height, renderCallback_t preRender, renderCallback_t postRender )
		: width( width ), height( height ), callbackPreRender( preRender ), callbackPostRender( postRender )
		{
			if ( !width || !height ) {
				throw( XSError( "Registered View with 0 width or 0 height" ) );
			}

			perFrameData = new Backend::Buffer( Backend::Buffer::Type::UNIFORM, nullptr, sizeof(float) * 16 * 2 );

			RegisterView( this );
		}

		void View::PreRender( void ) {
			// set up 2d/3d perspective
			projectionMatrix = ortho( 0.0f, static_cast<float>( width ), 0.0f, static_cast<float>( height ),
				0.0f, 1.0f );

			matrix4 *m = static_cast<matrix4 *>( perFrameData->Map() );

			*m = projectionMatrix;

			perFrameData->Unmap();
			perFrameData->BindRange( 6 );

			if ( callbackPreRender ) {
				callbackPreRender();
			}

			// sort surfaces etc?
		}

		void View::PostRender( void ) const {
			if ( callbackPostRender ) {
				callbackPostRender();
			}
		}

		void View::Bind( void ) {
			BindView( this );
		}

	} // namespace Renderer

} // namespace XS
