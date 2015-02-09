#pragma once

#include <GL/glew.h>

namespace XS {

	// front-end renderer access

	class Cvar;
	struct vector4;

	namespace Renderer {

		struct Material;
		struct View;

		extern Cvar		*r_clear;
		extern Cvar		*r_debug;
		extern Cvar		*r_multisample;
		extern Cvar		*r_skipRender;
		extern Cvar		*r_swapInterval;
		extern Cvar		*vid_height;
		extern Cvar		*vid_noBorder;
		extern Cvar		*vid_width;

		// used internally
		// register cvars owned by the renderer
		void RegisterCvars(
			void
		);

		// used internally
		// create a window with valid render context
		void CreateDisplay(
			void
		);

		// used internally
		// clean up window and render context
		void DestroyDisplay(
			void
		);

		// start up the renderer
		// also creates a window and registers cvars
		void Init(
			void
		);

		// shutdown the renderer
		void Shutdown(
			void
		);

		// render all views and flip the backbuffer
		void Update(
			void
		);

		//
		// views
		//

		// register a view to be rendered
		void RegisterView(
			View *view
		);

		// set the current view for subsequent rendering
		void BindView(
			View *view
		);

		//
		// render commands
		//

		// when called with a null material, will use default quad material
		// when called with a null colour, will use white
		void DrawQuad(
			float x,
			float y,
			float w,
			float h,
			float s1,
			float t1,
			float s2,
			float t2,
			const vector4 *colour,
			const Material *material
		);

	} // namespace Renderer

} // namespace XS
