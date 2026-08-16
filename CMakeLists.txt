cmake_minimum_required(VERSION 3.16)

project(LANTransfer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

qt_standard_project_setup()

qt_add_executable(LANTransfer
    client/main.cpp
    client/MainWindow.h
    client/MainWindow.cpp
    client/NetworkManager.h
    client/NetworkManager.cpp
    client/TransferManager.h
    client/TransferManager.cpp

    common/Protocol.h
    common/Protocol.cpp
)

qt_add_executable(FileServer
    server/main.cpp
    server/FileServer.h
    server/FileServer.cpp

    common/Protocol.h
    common/Protocol.cpp
)

target_link_libraries(LANTransfer
    PRIVATE
    Qt6::Widgets
    Qt6::Network
)

target_link_libraries(FileServer
    PRIVATE
    Qt6::Core
    Qt6::Network
)