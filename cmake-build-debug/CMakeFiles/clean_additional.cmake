# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/gui_design_soot_dem_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/gui_design_soot_dem_autogen.dir/ParseCache.txt"
  "gui_design_soot_dem_autogen"
  )
endif()
