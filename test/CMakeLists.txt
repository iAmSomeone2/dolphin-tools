# Copy test resources into the binary directory.
file(COPY ${CMAKE_SOURCE_DIR}/test/resources DESTINATION ${CMAKE_BINARY_DIR}/test/)

# Add the Yaz0 test
add_executable(yaz0test Yaz0/Yaz0Test.cpp)
target_link_libraries(yaz0test GTest::GTest)
target_link_libraries(yaz0test yaz0)
target_include_directories(yaz0test PUBLIC SYSTEM ${GTEST_INCLUDE_DIRS})