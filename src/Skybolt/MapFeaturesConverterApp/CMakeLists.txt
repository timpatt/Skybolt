set(APP_NAME MapFeaturesConverterApp)

file(GLOB SOURCE_FILES *.cpp *.h)

include_directories("../")

add_executable(${APP_NAME} ${SOURCE_FILES})

target_link_libraries(${APP_NAME} PRIVATE MapFeaturesConverter)
