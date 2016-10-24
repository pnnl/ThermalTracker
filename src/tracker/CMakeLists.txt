include_directories(${Boost_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

SET (TRACK_SRCS trackfile.cpp utilities.cpp thermaltracker.cpp)

add_library(thermaltracker ${TRACK_SRCS} extracttracks.cpp processvideo.cpp
            trackblobs.cpp trackfeatures.cpp trackfile.cpp trackpositions.cpp
            utilities.cpp visualization.cpp vpsfile.cpp process_params.hpp)

add_executable(processvideo processvideo_cli.cpp)
add_executable(extracttracks extracttracks_cli.cpp)
add_executable(trackfeatures trackfeatures_cli.cpp)
add_executable(testutil testutil.cpp utilities.cpp)
add_executable(trackblobs trackblobs_cli.cpp)
add_executable(trackpositions trackpositions_cli.cpp)

target_link_libraries(thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(processvideo thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(extracttracks thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(trackfeatures thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(testutil ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(trackblobs thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
target_link_libraries(trackpositions thermaltracker ${OpenCV_LIBS} ${Boost_LIBRARIES})
