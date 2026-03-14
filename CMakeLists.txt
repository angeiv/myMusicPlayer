cmake_minimum_required(VERSION 3.16)

project(myMusicPlayer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    Qml
    Quick
    QuickControls2
)

add_executable(myMusicPlayer
    main.cpp
    appcontroller.cpp
    appcontroller.h
    lyricsmodel.cpp
    lyricsmodel.h
    playlistmodel.cpp
    playlistmodel.h
    playerengine.cpp
    playerengine.h
    musicplayer.qrc
)

target_link_libraries(myMusicPlayer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

