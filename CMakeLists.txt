cmake_minimum_required(VERSION 3.25)

enable_language(ASM)
project(Calorimeter VERSION 1.0 DESCRIPTION "Timing a function.")
include_directories("include/")

# Configure the main executable
add_executable(Calorimeter src/main.cpp)
target_link_libraries(Calorimeter Application Patch Symbols)

# Configure the building of the source code for the main executable. We add
# special compiler flags so that we can be patchable.
add_library(Application src/calorimeter.cpp)
target_compile_options(Application BEFORE PUBLIC -pg -mfentry -mnop-mcount -mrecord-mcount)

# Configure building the support libraries.
add_library(Patch asm/patch.S src/patch.cpp)
add_library(Symbols "${CMAKE_SOURCE_DIR}/asm/symbols.S")
# Everytime that the application source code is rebuilt, rebuild the symbols.S file that will
# define the functions that are patchable.
add_custom_command(OUTPUT "${CMAKE_SOURCE_DIR}/asm/symbols.S" 
				COMMAND "${CMAKE_SOURCE_DIR}/scripts/generate_symbols.sh" "$<TARGET_FILE:Application>"  "${CMAKE_SOURCE_DIR}/asm/symbols.S" 
				DEPENDS Application)

# Now let's just make some dumps of the object files just for fun!
add_custom_command(TARGET Calorimeter POST_BUILD COMMAND "${CMAKE_SOURCE_DIR}/scripts/objdump_wrapper.sh" "$<TARGET_FILE:Calorimeter>" "${CMAKE_SOURCE_DIR}/dumps/Calorimeter.obj")
add_custom_command(TARGET Calorimeter POST_BUILD COMMAND "${CMAKE_SOURCE_DIR}/scripts/hexdump_wrapper.sh" "$<TARGET_FILE:Calorimeter>" "${CMAKE_SOURCE_DIR}/dumps/Calorimeter.hex")