#include "XSCommon/XSCommon.h"
#include "XSCommon/XSTimer.h"

// Author: Lourens Elzinga, Kieren McDevitt
//TODO: move to platform specific code.

namespace XS {

	Timer::Timer() {
		Start();
	}

	void Timer::Start( void ) {
	#if defined(XS_OS_WINDOWS)
		QueryPerformanceCounter( &start );
	#elif defined(XS_OS_LINUX) || defined(XS_OS_MAC)
		gettimeofday( &start, nullptr );
	#endif
	}

	void Timer::Stop( void ) {
	#if defined(XS_OS_WINDOWS)
		QueryPerformanceCounter( &stop );
	#elif defined(XS_OS_LINUX) || defined(XS_OS_MAC)
		gettimeofday( &stop, nullptr );
	#endif
	}

	double Timer::GetTiming( bool restart, Resolution resolution ) {
		Stop();

	#if defined(XS_OS_WINDOWS)
		timeVal_t frequency;
		QueryPerformanceFrequency( &frequency );

		double startTime = start.QuadPart * (1000000.0 / frequency.QuadPart);
		double stopTime = stop.QuadPart * (1000000.0 / frequency.QuadPart);
	#elif defined(XS_OS_LINUX) || defined(XS_OS_MAC)
		double startTime = (start.tv_sec * 1000000.0) + start.tv_usec;
		double stopTime = (stop.tv_sec * 1000000.0) + stop.tv_usec;
	#endif

		if ( restart ) {
			Start();
		}

		switch ( resolution ) {
		case Resolution::SECONDS: {
			return (stopTime - startTime) * 0.000001;
		} break;
		case Resolution::MILLISECONDS: {
			return (stopTime - startTime) * 0.001;
		} break;
		case Resolution::MICROSECONDS: {
			return (stopTime - startTime);
		} break;
		default: {
			SDL_assert( !"Invalid timer resolution" );
			return 0.0;
		} break;
		}
	}

} // namespace XS
