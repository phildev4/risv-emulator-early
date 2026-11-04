# The emulator itself, built as a library so the program and the tests share it.
# Add new emulator source files (cpu.cpp, decoder.cpp, ...) to this list.
add_library(risv_core STATIC
    memory.cpp
    memory.h
)
# Code includes the headers as "risv/memory.h". The folder prefix avoids
# clashing with system headers such as <memory.h>.
target_include_directories(risv_core PUBLIC ${PROJECT_SOURCE_DIR})
risv_setup_target(risv_core)

# The program you run.
add_executable(risv
    risv.cpp
    risv.h
)
target_link_libraries(risv PRIVATE risv_core)
risv_setup_target(risv)
