# 1. Prepare macros.

macro (do_set_graphics_source_dir)
  set(DO_Sara_Graphics_SOURCE_DIR ${DO_Sara_SOURCE_DIR}/Graphics)
endmacro (do_set_graphics_source_dir)

macro (do_list_graphics_source_files)
  # Master header file
  set(DO_Sara_Graphics_MASTER_HEADER ${DO_Sara_SOURCE_DIR}/Graphics.hpp)
  source_group("Master Header File" FILES ${DO_Graphics_MASTER_HEADER})
  # API header files
  file(GLOB DO_Sara_Graphics_API_HEADER_FILES ${DO_Sara_Graphics_SOURCE_DIR}/*.hpp)
  source_group("API Header Files" FILES ${DO_Sara_Graphics_API_HEADER_FILES})
  # API source files
  file(GLOB DO_Sara_Graphics_API_SOURCE_FILES ${DO_Sara_Graphics_SOURCE_DIR}/*.cpp)
  source_group("API Source Files" FILES ${DO_Sara_Graphics_API_SOURCE_FILES})
  # Derived QObjects header files
  file(GLOB DO_Sara_Graphics_DerivedQObjects_HEADER_FILES ${DO_Sara_Graphics_SOURCE_DIR}/DerivedQObjects/*.hpp)
  source_group("Derived QObjects Header Files" FILES ${DO_Sara_Graphics_DerivedQObjects_HEADER_FILES})
  # Derived QObjects source files
  file(GLOB DO_Sara_Graphics_DerivedQObjects_SOURCE_FILES ${DO_Sara_Graphics_SOURCE_DIR}/DerivedQObjects/*.cpp)
  source_group("Derived QObjects Source Files" FILES ${DO_Sara_Graphics_DerivedQObjects_SOURCE_FILES})
  # All files here
  set(DO_Sara_Graphics_FILES
    ${DO_Sara_Graphics_MASTER_HEADER}
    ${DO_Sara_Graphics_API_HEADER_FILES}
    ${DO_Sara_Graphics_API_SOURCE_FILES}
    ${DO_Sara_Graphics_DerivedQObjects_HEADER_FILES}
    ${DO_Sara_Graphics_DerivedQObjects_SOURCE_FILES})
endmacro (do_list_graphics_source_files)

macro (do_load_packages_for_graphics_library)
  if (WIN32)
    # Temporary workaround for windows 8
    list(APPEND CMAKE_PREFIX_PATH
         "C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64")
  endif ()
  if (DEFINED ENV{QTDIR})
    message(STATUS
            "Found environment variable QTDIR = $ENV{QTDIR} and appending "
            "it to CMAKE_MODULE_PATH")
    list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})
  endif ()
  find_package(Qt5Widgets REQUIRED)
  find_package(Qt5OpenGL REQUIRED)
  find_package(OpenGL REQUIRED)
  include_directories(${Qt5Widgets_INCLUDE_DIRS}
                      ${Qt5OpenGL_INCLUDE_DIRS})
  include(${DO_Sara_Core_USE_FILE})
  add_definitions(${Qt5Widgets_DEFINITIONS})
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
endmacro (do_load_packages_for_graphics_library)

macro (do_create_variables_for_graphics_library)
  # Use this variable if you want to link statically DO_Sara_Graphics.
  set(DO_Sara_Graphics_LIBRARIES DO_Sara_Graphics)
  # External libraries which DO_Sara_Graphics depends on.
  list(APPEND DO_Sara_Graphics_LINK_LIBRARIES ${OPENGL_LIBRARIES})
  # Note that DO_Sara_Graphics must also be linked against Qt but qt5_use_modules()
  # takes care of this.
endmacro (do_create_variables_for_graphics_library)





# 2. Setup the project by calling the macros
do_load_packages_for_graphics_library()

if (DO_USE_FROM_SOURCE)
  get_property(DO_Sara_Graphics_ADDED GLOBAL PROPERTY _DO_Sara_Graphics_INCLUDED)
  if (NOT DO_Sara_Graphics_ADDED)
    do_set_graphics_source_dir()
    do_list_graphics_source_files()
    do_create_variables_for_graphics_library()

    # Static library
    set_property(GLOBAL PROPERTY _DO_Sara_Graphics_INCLUDED 1)
    # Moc files
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
    source_group("Moc Files" FILES ${DO_Sara_Graphics_MOC_SOURCES})
    # Static library
    add_library(
      DO_Sara_Graphics STATIC
      ${DO_Sara_Graphics_FILES} ${DO_Sara_Graphics_MOC_SOURCES})
    target_link_libraries(
      DO_Sara_Graphics
      Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES})
    do_set_specific_target_properties(DO_Sara_Graphics DO_STATIC)
    install(TARGETS DO_Sara_Graphics
            ARCHIVE DESTINATION lib/DO/Sara
            LIBRARY DESTINATION lib/DO/Sara)

    # See DOMacros.cmake for details on do_set_specific_target_properties.
    set_property(TARGET DO_Sara_Graphics PROPERTY FOLDER "DO Sara Libraries")

    # Shared library
    if (DO_BUILD_SHARED_LIBS)
      add_library(
        DO_Sara_Graphics_SHARED SHARED
        ${DO_Sara_Graphics_FILES} ${DOGraphics_MOC_SOURCES})
      target_link_libraries(
        DO_Sara_Graphics_SHARED
        Qt5::Widgets Qt5::OpenGL ${OPENGL_LIBRARIES})
      do_set_specific_target_properties(DO_Sara_Graphics_SHARED DO_EXPORTS)
      set_property(TARGET DO_Sara_Graphics_SHARED PROPERTY FOLDER "DO Sara Libraries")
    endif ()

    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_INCLUDE_CURRENT_DIR OFF)
  endif()
endif ()
