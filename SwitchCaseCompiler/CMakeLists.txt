cmake_minimum_required(VERSION 3.16)
project(SwitchCaseCompiler VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 (or Qt5 if Qt6 not available)
find_package(Qt6 COMPONENTS Core Widgets Gui QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# Source files
set(CORE_SOURCES
    src/core/Lexer.cpp
    src/core/Parser.cpp
    src/core/AST.cpp
    src/core/SemanticAnalyzer.cpp
    src/core/TACGenerator.cpp
    src/core/Optimizer.cpp
    src/core/CodeGenerator.cpp
)

set(CORE_HEADERS
    src/core/Lexer.h
    src/core/Parser.h
    src/core/AST.h
    src/core/SemanticAnalyzer.h
    src/core/TACGenerator.h
    src/core/Optimizer.h
    src/core/CodeGenerator.h
)

set(GUI_SOURCES
    src/gui/MainWindow.cpp
    src/gui/DerivationViewer.cpp
    src/gui/ParseTreeView.cpp
    src/gui/ASTView.cpp
    src/gui/IRFlowDiagram.cpp
    src/gui/PerformanceMetrics.cpp
    src/gui/ThreeColumnView.cpp
    src/gui/OptimizationPanel.cpp
    src/gui/OptimizationDiff.cpp
    src/gui/TACSimulator.cpp
    src/gui/PipelineDiagram.cpp
    src/gui/GrammarCoverage.cpp
    src/gui/TokenChart.cpp
    src/gui/LifetimeDiagram.cpp
    src/gui/CompileReport.cpp
    src/gui/ShortcutOverlay.cpp
    src/gui/SyntaxHighlighter.cpp
    src/gui/CodeEditor.cpp
    src/gui/resources/resources.qrc
)

set(GUI_HEADERS
    src/gui/MainWindow.h
    src/gui/DerivationViewer.h
    src/gui/ParseTreeView.h
    src/gui/ASTView.h
    src/gui/IRFlowDiagram.h
    src/gui/PerformanceMetrics.h
    src/gui/ThreeColumnView.h
    src/gui/OptimizationPanel.h
    src/gui/OptimizationDiff.h
    src/gui/TACSimulator.h
    src/gui/PipelineDiagram.h
    src/gui/GrammarCoverage.h
    src/gui/TokenChart.h
    src/gui/LifetimeDiagram.h
    src/gui/CompileReport.h
    src/gui/ShortcutOverlay.h
    src/gui/SyntaxHighlighter.h
    src/gui/CodeEditor.h
)

# Create executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${GUI_SOURCES}
    ${GUI_HEADERS}
)

add_executable(SwitchCaseCompilerCLI
    src/cli/CompilerCli.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

# Link Qt libraries
if (${QT_VERSION_MAJOR} EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets Qt5::Gui)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
)

target_include_directories(SwitchCaseCompilerCLI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# Platform-specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
