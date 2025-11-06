cmake_minimum_required(VERSION 3.19)
project(HS_Demo LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

qt_standard_project_setup()

set(uiPath ${CMAKE_BINARY_DIR}/${PROJECT_NAME}_autogen/include)
include_directories(${uiPath})
include_directories(${CMAKE_BINARY_DIR})
include_directories(src/ThirdParty/QCustomPlot)
include_directories(src/Interface)
include_directories(src/Core)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Charts Widgets OpenGL OpenGLWidgets PrintSupport LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Charts Widgets OpenGL OpenGLWidgets PrintSupport LinguistTools)

qt_add_executable(HS_Demo
    WIN32 MACOSX_BUNDLE
    main.cpp
    src/interface/mainwindow.cpp
    src/interface/mainwindow.h
    src/interface/mainwindow.ui
    src/Core/DemoTimer.cpp
    src/Core/CSV_Saver.cpp
    src/Core/Sampler.cpp
    src/Core/DemoBuffer.cpp
    src/Core/DemoProcessor.cpp
    src/ThirdParty/QCustomPlot/qcustomplot.cpp
)

target_compile_definitions(HS_Demo PRIVATE QCUSTOMPLOT_USE_OPENGL)
target_compile_options(HS_Demo PRIVATE "-Wa,-mbig-obj")

target_link_libraries(HS_Demo
    PRIVATE
        Qt::Core
        Qt::Charts
        Qt::OpenGL
        Qt::OpenGLWidgets
        Qt::Widgets
        Qt::PrintSupport
        glu32.dll
        opengl32.dll
)

include(GNUInstallDirs)

install(TARGETS HS_Demo
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET HS_Demo
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})


# file(COPY ${CMAKE_SOURCE_DIR}/thirdparty/glfw3.dll DESTINATION ${CMAKE_BINARY_DIR}/)
# file(COPY ${CMAKE_SOURCE_DIR}/thirdparty/glu32.dll DESTINATION ${CMAKE_BINARY_DIR}/)
# file(COPY ${CMAKE_SOURCE_DIR}/thirdparty/opengl32.dll DESTINATION ${CMAKE_BINARY_DIR}/)
file(COPY ${CMAKE_SOURCE_DIR}/src/ThirdParty/QCustomPlot/qcustomplot.h DESTINATION ${CMAKE_BINARY_DIR}/)