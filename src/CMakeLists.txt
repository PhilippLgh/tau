# Copyright (c) 2014 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

#
# Source files.
#

# tau sources.
set(TAU_SRCS
  cef_client_app.cc
  cef_client_app.h
  node_main.cc
  node_main.h  
  native/bindings/browser_window.cc
  native/bindings/browser_window.h
  native/bindings/ipc.cc
  native/bindings/ipc.h
  simple_handler.cc
  simple_handler.h
  )
set(TAU_SRCS_LINUX
  tau_linux.cc
  simple_handler_linux.cc
  )
set(TAU_SRCS_MACOSX
  tau_mac.mm
  simple_handler_mac.mm
  )
set(TAU_SRCS_WINDOWS
  tau.exe.manifest
  tau.rc
  tau_win.cc
  resource.h
  simple_handler_win.cc
  )
APPEND_PLATFORM_SOURCES(TAU_SRCS)
source_group(src FILES ${TAU_SRCS})

set(TAU_RES_SRCS_WINDOWS
  res/tau.ico
  res/small.ico
  )
APPEND_PLATFORM_SOURCES(TAU_RES_SRCS)
source_group(src\\\\res FILES ${TAU_RES_SRCS})

set(TAU_SRCS
  ${TAU_SRCS}
  ${TAU_RES_SRCS}
  )

# tau helper sources.
set(TAU_HELPER_SRCS_MACOSX
  process_helper_mac.cc
  )
APPEND_PLATFORM_SOURCES(TAU_HELPER_SRCS)
source_group(src FILES ${TAU_HELPER_SRCS})

# tau resources.
set(TAU_RESOURCES_MAC_SRCS_MACOSX
  mac/Info.plist
  mac/tau.icns
  )
APPEND_PLATFORM_SOURCES(TAU_RESOURCES_MAC_SRCS)
source_group(src\\\\mac FILES ${TAU_RESOURCES_MAC_SRCS})

set(TAU_RESOURCES_MAC_ENGLISH_LPROJ_SRCS_MACOSX
  mac/English.lproj/InfoPlist.strings
  mac/English.lproj/MainMenu.xib
  )
APPEND_PLATFORM_SOURCES(TAU_RESOURCES_MAC_ENGLISH_LPROJ_SRCS)
source_group(src\\\\mac\\\\English.lproj FILES ${TAU_RESOURCES_MAC_ENGLISH_LPROJ_SRCS})

set(TAU_RESOURCES_SRCS
  ${TAU_RESOURCES_MAC_SRCS}
  ${TAU_RESOURCES_MAC_ENGLISH_LPROJ_SRCS}
  )


#
# Shared configuration.
#

# Target executable names.
set(CEF_TARGET "tau")
if(OS_MACOSX)
  set(CEF_HELPER_TARGET "tau_Helper")
  set(CEF_HELPER_OUTPUT_NAME "tau Helper")
endif()

# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")

# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

include_directories ("${PROJECT_SOURCE_DIR}/third_party/nodejs/include")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/v8/include")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/uv/include")

#
# Linux configuration.
#

if(OS_LINUX)
  # Executable target.
  add_executable(${CEF_TARGET} ${TAU_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${CEF_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN")
  set_target_properties(${CEF_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${CEF_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CEF_TARGET_OUT_DIR})

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")

  # Set SUID permissions on the chrome-sandbox target.
  SET_LINUX_SUID_PERMISSIONS("${CEF_TARGET}" "${CEF_TARGET_OUT_DIR}/chrome-sandbox")
endif()


#
# Mac OS X configuration.
#

if(OS_MACOSX)
  # Output paths for the app bundles.
  set(CEF_APP "${CEF_TARGET_OUT_DIR}/${CEF_TARGET}.app")
  set(CEF_HELPER_APP "${CEF_TARGET_OUT_DIR}/${CEF_HELPER_OUTPUT_NAME}.app")

  # Variable referenced from Info.plist files.
  set(PRODUCT_NAME "${CEF_TARGET}")

  # Helper executable target.
  add_executable(${CEF_HELPER_TARGET} MACOSX_BUNDLE ${TAU_HELPER_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_HELPER_TARGET})
  add_dependencies(${CEF_HELPER_TARGET} libcef_dll_wrapper)
  target_link_libraries(${CEF_HELPER_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_HELPER_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/helper-Info.plist
    OUTPUT_NAME ${CEF_HELPER_OUTPUT_NAME}
    )

  # Fix the framework rpath in the helper executable.
  FIX_MACOSX_HELPER_FRAMEWORK_RPATH(${CEF_HELPER_TARGET})

  # Main executable target.
  add_executable(${CEF_TARGET} MACOSX_BUNDLE ${TAU_RESOURCES_SRCS} ${TAU_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper "${CEF_HELPER_TARGET}")
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})
  set_target_properties(${CEF_TARGET} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/mac/Info.plist
    )

  # link node
  target_link_libraries (tau ${PROJECT_SOURCE_DIR}/third_party/nodejs/Release/libnode.64.dylib)
  link_directories(${PROJECT_SOURCE_DIR}/third_party/nodejs/Release)  

  # Copy files into the main app bundle.
  add_custom_command(
    TARGET ${CEF_TARGET}
    POST_BUILD
    # Copy the helper app bundle into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_HELPER_APP}"
            "${CEF_APP}/Contents/Frameworks/${CEF_HELPER_OUTPUT_NAME}.app"
    # Copy the CEF framework into the Frameworks directory.
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CEF_BINARY_DIR}/Chromium Embedded Framework.framework"
            "${CEF_APP}/Contents/Frameworks/Chromium Embedded Framework.framework"
    # Copy node shared library TODO replace with copy file? now it copies also dlls
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/third_party/nodejs/Release/"
            "${CEF_TARGET_OUT_DIR}/"
    VERBATIM
    )

  # Fix the framework rpath in the main executable.
  FIX_MACOSX_MAIN_FRAMEWORK_RPATH(${CEF_TARGET})

  # Manually process and copy over resource files.
  # The Xcode generator can support this via the set_target_properties RESOURCE
  # directive but that doesn't properly handle nested resource directories.
  # Remove these prefixes from input file paths.
  set(PREFIXES "mac/")
  COPY_MACOSX_RESOURCES("${TAU_RESOURCES_SRCS}" "${PREFIXES}" "${CEF_TARGET}" "${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_APP}")
endif()


#
# Windows configuration.
#

if(OS_WINDOWS)

  #TODO: warnings as errors -> false
  #CONSOLE APP instead of windows app

  # Executable target.
  # add_executable(${CEF_TARGET} WIN32 ${TAU_SRCS})
  # make console app
  add_executable(${CEF_TARGET} ${TAU_SRCS})
  add_dependencies(${CEF_TARGET} libcef_dll_wrapper)
  SET_EXECUTABLE_TARGET_PROPERTIES(${CEF_TARGET})
  target_link_libraries(${CEF_TARGET} libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  if(USE_SANDBOX)
    # Logical target used to link the cef_sandbox library.
    ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
    target_link_libraries(${CEF_TARGET} cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
  endif()

  # Add the custom manifest files to the executable.
  ADD_WINDOWS_MANIFEST("${CMAKE_CURRENT_SOURCE_DIR}" "${CEF_TARGET}" "exe")

  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  target_link_libraries (${CEF_TARGET} ${PROJECT_SOURCE_DIR}/third_party/nodejs/Release/node.lib)

  # Copy dynamic libraries into the bin/ dir after build has finished
  add_custom_command(TARGET tau POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/third_party/nodejs/Release/"
          $<TARGET_FILE_DIR:tau>)

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${CEF_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
  COPY_FILES("${CEF_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
endif()
