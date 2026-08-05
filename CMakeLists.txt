cmake_minimum_required(VERSION 3.16)

project(MapsQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Widgets)

set(GUI_SOURCES
    src/gui/mapGraphicsItem.cpp
    src/gui/mapGraphicsItem.h
    src/gui/mainWindow.cpp
    src/gui/mainWindow.h
    src/gui/dynamicMapGraphicsItem.cpp
    src/gui/dynamicMapGraphicsItem.h
)

set(IDM_SOURCES
    src/idm/trafficSimulation.cpp
    src/idm/trafficSimulation.h
    src/idm/trafficCar.cpp
    src/idm/trafficCar.h
    src/idm/trafficObject.cpp
    src/idm/trafficObject.h
    src/idm/trafficSignal.cpp
    src/idm/trafficSignal.h
    src/idm/trafficDummy.cpp
    src/idm/trafficDummy.h
)

set(MAP_SOURCES
    src/map/bounds.cpp
    src/map/bounds.h
    src/map/connection.cpp
    src/map/connection.h
    src/map/graphRepresentation.cpp
    src/map/graphRepresentation.h
    src/map/guiRepresentation.cpp
    src/map/guiRepresentation.h
    src/map/highwayClassification.cpp
    src/map/highwayClassification.h
    src/map/map.cpp
    src/map/map.h
    src/map/mapData.cpp
    src/map/mapData.h
    src/map/node.cpp
    src/map/node.h
    src/map/relation.cpp
    src/map/relation.h
    src/map/trafficSignalSynchro.cpp
    src/map/trafficSignalSynchro.h
    src/map/way.cpp
    src/map/way.h
)

set(MAPIO_SOURCES
    src/mapIO/mapFiltering.cpp
    src/mapIO/mapFiltering.h
    src/mapIO/myXmlParser.cpp
    src/mapIO/myXmlParser.h
    src/mapIO/myXmlWriter.cpp
    src/mapIO/myXmlWriter.h
)

set(UTILS_SOURCES
    src/utils/benchmark.h
    src/utils/hash.h
    src/utils/random.cpp
    src/utils/random.h
    src/utils/settings.cpp
    src/utils/settings.h
    src/utils/span.h
)

source_group("gui" FILES ${GUI_SOURCES})
source_group("idm" FILES ${IDM_SOURCES})
source_group("map" FILES ${MAP_SOURCES})
source_group("mapIO" FILES ${MAPIO_SOURCES})
source_group("utils" FILES ${UTILS_SOURCES})
source_group("main" FILES src/main.cpp src/core.h Main.qml)

qt_add_executable(appMapsQt
    src/main.cpp
    src/core.h
    ${IDM_SOURCES}
    ${MAP_SOURCES}
    ${MAPIO_SOURCES}
    ${UTILS_SOURCES}
)

qt_add_qml_module(appMapsQt
    URI MapsQt
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES ${GUI_SOURCES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appMapsQt PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appMapsQt
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appMapsQt
    PRIVATE Qt6::Quick Qt6::Core Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS appMapsQt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
