// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package
// Created on: 06/01/2020

#include "core/crash_handler.h"

#include <csignal>
#include <cstdio>
#include <ios>
#include <sstream>

#include "core/filesystem.h"
#include "core/logger.h"

// Logs message to stderr and file
#define CRASH_LOG_MESSAGE(format_, ... )\
	{\
		fprintf(file, format_, __VA_ARGS__);\
		fprintf(stderr, format_, __VA_ARGS__);\
	}

// Close ofstream and open as FILE*
#define CRASH_OPEN_LOG_FILE()\
	jactorio::core::CloseLogFile();\
	auto* file = fopen(jactorio::core::ResolvePath(jactorio::core::kLogFileName).c_str(), "a");\
	CRASH_LOG_MESSAGE("\n\nJactorio crashed%c", '\n');\
	CRASH_LOG_MESSAGE("Below contains debug information related to the crash\n%c", '\n')


constexpr char kCrashContactInfo[] = "https://github.com/jaihysc/Jactorio";


void PrintStackTrace(FILE* file);

// ======================================================================
// Windows (MSVC) crash handling

#ifdef _MSC_VER

#include <StackWalker/StackWalker.h>

class JactorioStackWalker final : public StackWalker
{
public:
	explicit JactorioStackWalker(FILE* file)
		: file(file) {
	}

protected:
	void OnOutput(const LPCSTR szText) override {
		CRASH_LOG_MESSAGE("%s", szText);
	}

private:
	FILE* file;  // Cannot be renamed for macro to work
};

LONG WINAPI TopLevelExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo) {
	CRASH_OPEN_LOG_FILE();

	std::ostringstream sstr;
	sstr << std::hex << pExceptionInfo->ExceptionRecord->ExceptionCode;

	CRASH_LOG_MESSAGE("%s\n", sstr.str().c_str());

	PrintStackTrace(file);

	return EXCEPTION_CONTINUE_SEARCH;
}

#endif


// ======================================================================
// Linux

#ifdef __linux__

#define BACKWARD_HAS_DW 1
#include <backward.hpp>

#endif

///
/// \param file Will be closed on function return
void PrintStackTrace(FILE* file) {
	// Log to both file and stderr

	CRASH_LOG_MESSAGE("\nStacktrace: %c", '\n');

	// Print stacktrace
	if constexpr (std::string_view(JACTORIO_BUILD_TARGET_PLATFORM) == "Windows") {
#ifdef _MSC_VER

		JactorioStackWalker sw{file};
		sw.ShowCallstack();

#endif
	}
	else if constexpr (std::string_view(JACTORIO_BUILD_TARGET_PLATFORM) == "Darwin") {
		// TODO stacktrace on OSX?
	}
	else if constexpr (std::string_view(JACTORIO_BUILD_TARGET_PLATFORM) == "Linux") {
#ifdef __linux__

		using namespace backward;
		StackTrace st;

		st.load_here(99);  // Limit the number of trace depth
		// st.skip_n_firsts(3);  // Skip some backward internal function from the trace

		// To console
		Printer p;
		p.snippet = true;
		p.object = true;
		p.address = true;
		p.color_mode = ColorMode::automatic;
		p.print(st, stderr);

		// To log file
		p.color_mode = ColorMode::never;
		p.print(st, file);

#endif
	}

	CRASH_LOG_MESSAGE("\n\nPlease report this with the current log at %s\n", kCrashContactInfo);

	fclose(file);
}

[[noreturn]] void SignalCrashHandler(const int signum) {
	const char* name = nullptr;

	switch (signum) {
	case SIGABRT:
		name = "SIGABRT";
		break;
	case SIGSEGV:
		name = "SIGSEGV";
		break;
	case SIGILL:
		name = "SIGILL";
		break;
	case SIGFPE:
		name = "SIGFPE";
		break;

	default:
		break;
	}

	CRASH_OPEN_LOG_FILE();

	if (name) {
		CRASH_LOG_MESSAGE("Signal %d (%s)\n", signum, name);
	}
	else {
		CRASH_LOG_MESSAGE("Signal %d\n", signum);
	}

	PrintStackTrace(file);
	exit(signum);
}

void jactorio::core::RegisterCrashHandler() {
	// Crash handling functions
	signal(SIGABRT, &SignalCrashHandler);
	signal(SIGSEGV, &SignalCrashHandler);
	signal(SIGILL, &SignalCrashHandler);
	signal(SIGFPE, &SignalCrashHandler);

	// Windows SEH
#ifdef _MSC_VER
	SetUnhandledExceptionFilter(TopLevelExceptionHandler);
#endif
}
