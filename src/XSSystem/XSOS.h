#pragma once

namespace XS {

	namespace OS {

		// retrieve the current working directory
		void GetCurrentWorkingDirectory(
			char *cwd,
			size_t bufferLen
		);

		// returns true if file exists and last modified time could be retrieved
		// outTime will contain the last modified time as unix timestamp
		bool GetFileTime(
			const char *path,
			int32_t *outTime
		);

		// returns true if file exists
		bool MkDir(
			const char *path
		);

		// resolve a relative game path into a full OS path
		bool ResolvePath(
			char *outPath,
			const char *inPath,
			size_t pathLen
		);

		// returns true is directory was removed
		bool RmDir(
			const char *path
		);

		// returns true if file was removed
		bool RmFile(
			const char *path
		);

		// returns true if file exists
		bool Stat(
			const char *path
		);

	} // namespace OS

} // namespace XS
