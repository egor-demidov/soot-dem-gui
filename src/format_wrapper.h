#ifndef SOOT_DEM_GUI_FORMAT_WRAPPER_H
#define SOOT_DEM_GUI_FORMAT_WRAPPER_H
#ifndef USE_FMT
#include <format>
#define format_string std::format
#else //USE_FMT
#include <fmt/format.h>
#define format_string fmt::format
#endif //USE_FMT
#endif //SOOT_DEM_GUI_FORMAT_WRAPPER_H