#include <SDL2/SDL_timer.h>
#include <SDL2/SDL.h>

#include "XSCommon/XSCommon.h"
#include "XSCommon/XSCommand.h"
#include "XSCommon/XSConsole.h"
#include "XSCommon/XSCvar.h"
#include "XSCommon/XSEvent.h"
#include "XSCommon/XSError.h"
#include "XSCommon/XSFile.h"
#include "XSCommon/XSTimer.h"
#include "XSCommon/XSString.h"
#include "XSCommon/XSGlobals.h"
#include "XSClient/XSClient.h"
#include "XSInput/XSInput.h"
#include "XSInput/XSKeys.h"
#include "XSRenderer/XSRenderer.h"

namespace XS {

	namespace Common {

		#define DEFAULT_CONFIG			"cfg/pathfinding.cfg"

		Cvar *com_developer;
		static Cvar *com_framerate;
		static Cvar *r_framerate;

		static void RegisterCvars( void ) {
			Cvar::Create( "com_date", __DATE__, "Compilation date", CVAR_READONLY );
		#ifdef _DEBUG
			com_developer = Cvar::Create( "com_developer", "1", "Developer mode", CVAR_NONE );
		#else
			com_developer = Cvar::Create( "com_developer", "0", "Developer mode", CVAR_NONE );
		#endif // _DEBUG
			com_framerate = Cvar::Create( "com_framerate", "50", "Game tick rate", CVAR_NONE );
			r_framerate = Cvar::Create( "r_framerate", "120", "Render framerate", CVAR_NONE );
		}

		static void ParseCommandLine( int32_t argc, char **argv ) {
			std::string commandLine;

			// concatenate argv[] to commandLine
			for ( int32_t i = 1; i < argc; i++ ) {
				const bool containsSpaces = strchr( argv[i], ' ' ) != nullptr;

				if ( containsSpaces ) {
					commandLine += "\"";
				}

				commandLine += argv[i];

				if ( containsSpaces ) {
					commandLine += "\"";
				}

				commandLine += " ";
			}

			// split up commandLine by +
			//	+set x y +set herp derp
			// becomes
			//	set x y
			//	set herp derp
			// then append it to the command buffer
			const char delimiter = '+';
			const size_t start = commandLine.find( delimiter );
			if ( start == std::string::npos ) {
				return;
			}
			Command::Append( &commandLine[start + 1], delimiter );

			console.DebugPrint( "Startup parameters:\n" );
			Indent indent( 1 );
			for ( const auto &arg : String::Split( &commandLine[start + 1], delimiter ) ) {
				console.DebugPrint( "%s\n", arg.c_str() );
			}
		}

		static void LoadConfig( void ) {
			const char *cfg = DEFAULT_CONFIG;
			const File f( cfg, FileMode::READ );

			if ( f.open ) {
				char *buffer = new char[f.length];
					f.Read( reinterpret_cast<uint8_t *>(buffer) );

					Command::ExecuteBuffer(); // flush buffer before we issue commands
					char *current = strtok( buffer, "\n" );
					while ( current ) {
						Command::Append( current );
						Command::ExecuteBuffer();
						current = strtok( nullptr, "\n" );
					}
				delete[] buffer;
			}
		}

		static void WriteConfig( const char *cfg = nullptr ) {
			std::string str = "";
			Cvar::WriteCvars( str );
			Client::WriteBinds( str );

			// default config if none specified
			if ( !cfg ) {
				cfg = DEFAULT_CONFIG;
			}

			const File f( cfg, FileMode::WRITE );
			if ( !f.open ) {
				console.Print( "Failed to write config! (%s)\n", cfg );
				return;
			}

			f.AppendString( str.c_str() );
		}

		static void Cmd_WriteConfig( const commandContext_t * const context ) {
			const char *cfg = nullptr;
			if ( context->size() ) {
				 cfg = (*context)[0].c_str();
			 }

			WriteConfig( cfg );
		}

		Timer *gameTimer = nullptr;

	} // namespace Common

} // namespace XS

static XS::Timer globalTimer;

int main( int argc, char **argv ) {
	try {
		srand( time( nullptr ) );

		// critical initialisation
		XS::File::Init();
		XS::Common::RegisterCvars();
		XS::Command::Init(); // register commands like exec, vstr
		XS::Command::AddCommand( "writeconfig", XS::Common::Cmd_WriteConfig );
		XS::Common::ParseCommandLine( argc, argv );

		// execute the command line args, so config can be loaded from an overridden base path
		XS::Command::ExecuteBuffer();
		XS::File::SetBasePath();
		XS::Common::LoadConfig();

		//
		// DO NOT LOAD MEDIA BEFORE THIS POINT
		//

		XS::console.Print( WINDOW_TITLE " (" XSTR( ARCH_WIDTH ) " bits) built on " __DATE__ " [git " REVISION "]\n" );

		XS::Renderer::Init();
		XS::Event::Init();

		if ( XS::Common::com_developer->GetBool() ) {
			double t = globalTimer.GetTiming( true, XS::Timer::Resolution::MILLISECONDS );
			XS::console.DebugPrint( "Init time: %.0f milliseconds\n", t );
		}

		XS::Client::Init();

		// post-init stuff
		XS::Cvar::initialised = true;

		// frame
		XS::Common::gameTimer = new XS::Timer();
		while ( 1 ) {
			static double currentTime = XS::Common::gameTimer->GetTiming( false, XS::Timer::Resolution::MILLISECONDS );
			static double accumulator = 0.0;

			// calculate delta time for integrating this frame
			const double newTime = XS::Common::gameTimer->GetTiming( false, XS::Timer::Resolution::MILLISECONDS );
			const double dt = 1000.0 / XS::Common::com_framerate->GetDouble();
			const double frameTime = newTime - currentTime;
			currentTime = newTime;

			double sliceMsec = frameTime;
			if ( sliceMsec > 250.0 ) {
				sliceMsec = 250.0; // avoid spiral of death, maximum 250mspf
			}
			accumulator += sliceMsec;

			// input
			XS::Client::input.Poll();

			// event pump
			XS::Event::Pump();
			XS::Command::ExecuteBuffer();

			while ( accumulator >= dt ) {
				XS::Client::RunFrame( dt );
				accumulator -= dt;
			}

			// alpha = accumulator / dt;
			// lerp( previousState, alpha, currentState )
			XS::Client::DrawFrame( frameTime );
			XS::Renderer::Update( /*state*/ );

			const double frameRate = XS::Common::r_framerate->GetDouble();
			const double renderMsec = 1000.0 / frameRate;
			if ( frameTime < renderMsec ) {
				XS::console.DebugPrint( "frameTime %.5f < %.5f, delaying for %0i\n", frameTime, renderMsec,
					(uint32_t)(renderMsec - frameTime) );
				SDL_Delay( (uint32_t)(renderMsec - frameTime) );
			}
		}
	}
	catch( const XS::XSError &e ) {
		const bool developer = XS::Common::com_developer->GetBool();

		XS::console.Print( "\n*** xsngine is shutting down\nReason: %s\n\n", e.what() );

		if ( developer ) {
			const double runtime = globalTimer.GetTiming( true, XS::Timer::Resolution::SECONDS );
			XS::console.DebugPrint( "Run time: %.3f seconds\n", runtime );
		}

		// indent the console for this scope
		{
			XS::Indent indent( 1 );
			XS::Client::Shutdown();
			XS::Renderer::Shutdown();

			if ( XS::Cvar::initialised ) {
				XS::Common::WriteConfig();
				XS::Cvar::Clean();
			}
		}

		if ( developer ) {
			const double shutdownTIme = globalTimer.GetTiming( false, XS::Timer::Resolution::SECONDS );
			XS::console.DebugPrint( "Shutdown time: %.3f seconds\n\n\n", shutdownTIme );
		}

		return EXIT_SUCCESS;
	}

	// never reached
	return EXIT_FAILURE;
}
