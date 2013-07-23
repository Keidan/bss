
# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "bss_config.h.in"
  "bss_config.h"
)


set(bin_src 
  ${BSS_SRC_DIR}/bss.c 
)
add_executable(bss ${bin_src})
target_link_libraries(bss tk)
