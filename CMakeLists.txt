cmake_minimum_required(VERSION 3.14)

project(RtpAVServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置ffpmeg root路径
 set(ffmpeg501path ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg-5.0.1-full_build-shared)

 # 包含ffpmeg头文件和lib库目录
 include_directories(${ffmpeg501path}/include/)
 link_directories(${ffmpeg501path}/lib/)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick  Multimedia LinguistTools)

set(TS_FILES RtpAVServer_zh_CN.ts)

set(PROJECT_SOURCES
        audiocapture.cpp
        audiocapture.h
        udpclient.cpp
        udpclient.h
        cameracapture.cpp
        cameracapture.h
        WriteDataDevice.h
        WriteDataDevice.cpp
        videoaudiomux.cpp
        videoaudiomux.h
        main.cpp
        qml.qrc
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(RtpAVServer_server
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET RtpAVServer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(RtpAVServer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(RtpAVServer
          ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(RtpAVServer
  PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::Multimedia)

set_target_properties(RtpAVServer PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS RtpAVServer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(RtpAVServer)
    qt_finalize_executable(RtpAVServer)
endif()



#设置 ffmpeg dll路径，并复制到打包目录
 set(dll_src ${ffmpeg501path}/bin/*.dll)
 FILE(GLOB Plugin_Runtime_Debug_DLL_FILES CONFIGURE_DEPENDS ${dll_src})
FILE(GLOB Plugin_Runtime_Release_DLL_FILES CONFIGURE_DEPENDS ${dll_src})
FILE(GLOB Plugin_Runtime_Debug_Resources_FILES CONFIGURE_DEPENDS ${dll_src})
FILE(GLOB Plugin_Runtime_Release_Resources_FILES CONFIGURE_DEPENDS ${dll_src})
message( "build path = " ${PROJECT_BINARY_DIR})

#复制ffmpeg的dll文件到到打包目录，打包目录支持3个目录，。
#VS2019下编译是复制到当前目录下的，debug和release分别分开了所以只需要复制到PROJECT_BINARY_DIR的当前目录
#cmake打包时，分release和debug目录，是在apps/edge-service/Debug 和apps/edge-service/Release
#所以是三个目录，apps/edge-service/Debug，apps/edge-service/Release ，PROJECT_BINARY_DIR
add_custom_target(CopyRuntimeFiles  ALL
  VERBATIM
  COMMAND_EXPAND_LISTS
  COMMAND ${CMAKE_COMMAND} -E
  make_directory "${PROJECT_BINARY_DIR}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:.//>/"
  COMMAND ${CMAKE_COMMAND} -E
  copy_if_different
  "$<$<CONFIG:Release>:${Plugin_Runtime_Release_DLL_FILES}>"
  "$<$<CONFIG:Debug>:${Plugin_Runtime_Debug_DLL_FILES}>"
  "${PROJECT_BINARY_DIR}/"
  COMMAND ${CMAKE_COMMAND} -E
  make_directory "${PROJECT_BINARY_DIR}/"
  COMMAND ${CMAKE_COMMAND} -E
  copy_if_different
  "$<$<CONFIG:Release>:${Plugin_Runtime_Release_Resources_FILES}>"
  "$<$<CONFIG:Debug>:${Plugin_Runtime_Debug_Resources_FILES}>"
  "${PROJECT_BINARY_DIR}/"

   COMMAND ${CMAKE_COMMAND} -E
  make_directory "${PROJECT_BINARY_DIR}/"
  COMMAND ${CMAKE_COMMAND} -E
  copy_if_different
  "$<$<CONFIG:Release>:${Plugin_Runtime_Release_Resources_FILES}>"
  "$<$<CONFIG:Debug>:${Plugin_Runtime_Debug_Resources_FILES}>"
  "${PROJECT_BINARY_DIR}/")
