cmake_minimum_required(VERSION 3.16)

project(MapsQt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Widgets)

qt_add_executable(appMapsQt
    main.cpp
    benchmark.h
    bounds.cpp bounds.h
    connection.cpp connection.h
    core.h
    graphRepresentation.cpp graphRepresentation.h
    guiRepresentation.cpp guiRepresentation.h
    highwayClassification.cpp highwayClassification.h
    iniReader.cpp iniReader.h
    map.cpp map.h
    mapData.cpp mapData.h
    mapFiltering.cpp mapFiltering.h
    myXmlParser.cpp myXmlParser.h
    myXmlWriter.cpp myXmlWriter.h
    node.cpp node.h
    random.cpp random.h
    relation.cpp relation.h
    trafficCar.cpp trafficCar.h
    trafficDummy.cpp trafficDummy.h
    trafficObject.cpp trafficObject.h
    trafficSignal.cpp trafficSignal.h
    trafficSignalSynchro.cpp trafficSignalSynchro.h
    trafficSimulation.cpp trafficSimulation.h
    way.cpp way.h
)

qt_add_qml_module(appMapsQt
    URI MapsQt
    VERSION 1.0
    QML_FILES
        Main.qml
        SOURCES mainWindow.h
        SOURCES mainWindow.cpp
        SOURCES mapGraphicsItem.h
        SOURCES mapGraphicsItem.cpp
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
