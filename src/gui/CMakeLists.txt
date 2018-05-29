cmake_minimum_required (VERSION 3.0.0)
project (THERMALTRACKER) 

# EDIT THIS LINE TO POINT TO THE ROOT QT DIRECTORY
set(QT_VERSION_DIR /usr/local/opt/qt)

if(QT_VERSION_DIR)
  file(TO_CMAKE_PATH ${QT_VERSION_DIR}/lib/cmake/Qt5Widgets Qt5Widgets_DIR)
else()
  message("QT_VERSION_DIR not provided, using Qt5Widgets_DIR")
endif()

find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})

set(GUI_SOURCES main.cpp
                mainwindow.cpp
                helpwindow.cpp
                qt_logger.cpp
                processthread.cpp
                thermal-resources.qrc
                helpicon.cpp)


add_executable(thtracker-gui ${OS_BUNDLE} ${GUI_SOURCES})
set_target_properties(thtracker-gui PROPERTIES OUTPUT_NAME ThermalTracker)
target_link_libraries(thtracker-gui thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
qt5_use_modules(thtracker-gui Widgets Gui)

if(APPLE)
  file(TO_CMAKE_PATH ${QT_VERSION_DIR}/bin/macdeployqt MACDEPLOYQT)
  add_custom_command(TARGET thtracker-gui
                     POST_BUILD
                     COMMAND ${MACDEPLOYQT} ${CMAKE_CURRENT_BINARY_DIR}/ThermalTracker.app -dmg)
  set(GUI_EXE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ThermalTracker.app/Contents/MacOS)
  
elseif(WIN32)
  if(${CMAKE_BUILD_TYPE} MATCHES Release)
    set(DEPLOY_CONFIG --release)
  else()
    set(DEPLOY_CONFIG --debug)
  endif()

  file(TO_CMAKE_PATH ${QT_VERSION_DIR}/bin/windeployqt WINDEPLOYQT)
  
  if(MSVC)
    set(GUI_EXE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
  else()
    set(GUI_EXE_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  
  add_custom_command(TARGET thtracker-gui
                     POST_BUILD
                     COMMAND ${WINDEPLOYQT} ${GUI_EXE_DIR}/ThermalTracker.exe ${DEPLOY_CONFIG})  
else()
  set(GUI_EXE_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()

#copy the default configuration file to the executable directory
file(COPY defaults.ini DESTINATION ${GUI_EXE_DIR})

if(${CMAKE_BUILD_TYPE} MATCHES Release)
  if (WIN32)
    if(NOT MSVC)
      #After the build, move all of the files to a common release directory, with the exe and dlls at the root.
      add_custom_command(TARGET thtracker-gui
                     POST_BUILD
                     COMMAND copy ${OpenCV_DIR}/x86/mingw/bin/libopencv_core*.dll ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND copy ${OpenCV_DIR}/x86/mingw/bin/libopencv_highgui*.dll ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND copy ${OpenCV_DIR}/x86/mingw/bin/libopencv_imgproc*.dll ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND copy ${OpenCV_DIR}/x86/mingw/bin/opencv_ffmpeg*.dll ${CMAKE_CURRENT_BINARY_DIR}
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/*.exe ${CMAKE_CURRENT_BINARY_DIR}/../Release/
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/*.dll ${CMAKE_CURRENT_BINARY_DIR}/../Release/
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/*.ini ${CMAKE_CURRENT_BINARY_DIR}/../Release/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/../tracker/*.exe ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/../tracker/*.lib ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/doc
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/../../../doc/* ${CMAKE_CURRENT_BINARY_DIR}/../Release/doc/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/sample_video
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}/../../../sample_video/*.mp4 ${CMAKE_CURRENT_BINARY_DIR}/../Release/sample_video/)
    else()
      #STRING(REGEX REPLACE "/" "\\\\" OpenCV_CONFIG_PATH ${OpenCV_CONFIG_PATH})
      STRING(REGEX REPLACE "/" "\\\\" OpenCV_LIB_DIR_OPT ${OpenCV_LIB_DIR_OPT})
      STRING(REGEX REPLACE "/" "\\\\" GUI_EXE_DIR ${GUI_EXE_DIR})
      STRING(REGEX REPLACE "/" "\\\\" CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
      add_custom_command(TARGET thtracker-gui
                     POST_BUILD
                     #COMMAND copy ${OpenCV_CONFIG_PATH}\\x64\\vc14\\bin\\opencv_core*.dll ${GUI_EXE_DIR})
                     COMMAND copy ${OpenCV_LIB_DIR_OPT}\\..\\bin\\opencv_core*.dll ${GUI_EXE_DIR}
                     COMMAND copy ${OpenCV_LIB_DIR_OPT}\\..\\bin\\opencv_highgui*.dll ${GUI_EXE_DIR}
                     COMMAND copy ${OpenCV_LIB_DIR_OPT}\\..\\bin\\opencv_imgproc*.dll ${GUI_EXE_DIR}
                     COMMAND copy ${OpenCV_LIB_DIR_OPT}\\..\\bin\\opencv_ffmpeg*.dll ${GUI_EXE_DIR}
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release
                     COMMAND xcopy ${GUI_EXE_DIR}\\* ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\ /e
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\tracker
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}\\..\\tracker\\Release\\* ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\tracker\\
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\doc
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}\\..\\..\\..\\doc\\* ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\doc\\
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\sample_video
                     COMMAND copy ${CMAKE_CURRENT_BINARY_DIR}\\..\\..\\..\\sample_video\\*.mp4 ${CMAKE_CURRENT_BINARY_DIR}\\..\\Release\\sample_video\\)
  
    endif()

  else ()
    #if not windows:
      add_custom_command(TARGET thtracker-gui
                     POST_BUILD
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/ThermalTracker ${CMAKE_CURRENT_BINARY_DIR}/../Release/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/*.ini ${CMAKE_CURRENT_BINARY_DIR}/../Release/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/extracttracks ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/processvideo ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/testutil ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/trackblobs ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/trackfeatures ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../tracker/trackpositions ${CMAKE_CURRENT_BINARY_DIR}/../Release/tracker/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/doc
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../../../doc/* ${CMAKE_CURRENT_BINARY_DIR}/../Release/doc/
                     COMMAND mkdir ${CMAKE_CURRENT_BINARY_DIR}/../Release/sample_video
                     COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/../../../sample_video/*.mp4 ${CMAKE_CURRENT_BINARY_DIR}/../Release/sample_video/)
  endif()

endif()



