cmake_minimum_required(VERSION 3.16)

project(kyall VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    Gui
    WebSockets
)

find_package(KF6 REQUIRED COMPONENTS
    I18n
    CoreAddons
    Config
    ConfigWidgets
    StatusNotifierItem
    Notifications
    KIO
)

# Find secp256k1 for Nostr cryptography
find_library(SECP256K1_LIBRARY secp256k1 REQUIRED)
find_path(SECP256K1_INCLUDE_DIR secp256k1.h REQUIRED)

set(kyall_SRCS
    src/main.cpp
    src/mainwindow.cpp
    src/postwidget.cpp
    src/accountmanager.cpp
    src/serviceinterface.cpp
    src/mastodonservice.cpp
    src/blueskyservice.cpp
    src/microblogservice.cpp
    src/nostrservice.cpp
    src/testservice.cpp
    src/imageuploader.cpp
    src/settings.cpp
    src/accountdialog.cpp
    src/settingsdialog.cpp
    src/securestorage.cpp
)

qt6_add_resources(kyall_SRCS resources/resources.qrc)

add_executable(kyall ${kyall_SRCS})

target_include_directories(kyall PRIVATE ${SECP256K1_INCLUDE_DIR})

target_link_libraries(kyall
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
    Qt6::WebSockets
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::StatusNotifierItem
    KF6::Notifications
    KF6::KIOCore
    ${SECP256K1_LIBRARY}
)

install(TARGETS kyall ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES kyall.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES kyall.svg DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps)
