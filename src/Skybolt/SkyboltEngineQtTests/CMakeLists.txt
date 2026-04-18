set(APP_NAME SkyboltEngineQtTests)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")

find_package(Catch2)

set(CMAKE_AUTOMOC ON)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries(${APP_NAME}
	SkyboltEngineQt
	Catch2::Catch2
)

catch_discover_tests(${APP_NAME})
