cmake_minimum_required(VERSION 2.8.12)
set (CMAKE_CXX_STANDARD 14)
project(style_rank)

SET(SOURCE_DIR "src/style_rank")
# Tell cmake that headers are in alse in source_dir
include_directories(${SOURCE_DIR})

SET(TEST_DIR "tests")
SET(TESTS ${SOURCES}
    "${TEST_DIR}/test_main.cpp"
    "${TEST_DIR}/test_features.cpp"
)

# Generate a test executable
include_directories(lib/catch/include)
add_executable("${PROJECT_NAME}_test" ${TESTS})

# Generate python module
add_subdirectory(lib/pybind11)
pybind11_add_module(
    style_rank ${SOURCES} 
    "${SOURCE_DIR}/bindings.cpp" 
    "${SOURCE_DIR}/deps/MidiFile.cpp" 
    "${SOURCE_DIR}/deps/MidiEvent.cpp" 
    "${SOURCE_DIR}/deps/MidiEventList.cpp" 
    "${SOURCE_DIR}/deps/MidiMessage.cpp" 
    "${SOURCE_DIR}/deps/Binasc.cpp"
)