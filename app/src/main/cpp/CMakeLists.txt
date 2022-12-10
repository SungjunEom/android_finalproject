# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.

project("finalproject")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
add_library( libgles
        SHARED
        IMPORTED )
set_target_properties( # Specifies the target library.
        libgles

        # Specifies the parameter you want to define.
        PROPERTIES IMPORTED_LOCATION

        # Provides the path to the library you want to import.
        C:/Users/Firework/Downloads/OpenCL_lib_and_include1/OpenCL_lib_and_include/lib/libGLES_mali.so )
add_library(GLES_mali STATIC IMPORTED)
include_directories( C:/Users/Firework/Downloads/OpenCL_lib_and_include1/OpenCL_lib_and_include/include/ )
add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp)

add_library( # Sets the name of the library.
        JNIDriver

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        JNIDriver.c)

add_library( # Sets the name of the library.
        OpenCLDriver

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        OpenCLDriver.c)

add_library( # Sets the name of the library.
        bmp

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        bmp.c)

add_library( # Sets the name of the library.
        GPIODriver

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        GPIODriver.c)

add_library( # Sets the name of the library.
        SegmentDriver

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        SegmentDriver.c)



# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

find_library( # Sets the name of the path variable.
        jnigraphics-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        jnigraphics )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries( # Specifies the target library.
        JNIDriver

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries( # Specifies the target library.
        bmp

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries( # Specifies the target library.
        GPIODriver

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries( # Specifies the target library.
        SegmentDriver

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})

target_link_libraries( # Specifies the target library.
        OpenCLDriver
        libgles

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        ${libgles}
        ${jnigraphics-lib})