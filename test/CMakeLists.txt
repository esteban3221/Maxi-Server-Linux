
add_executable(run_tests test_main.cpp test_validators.cpp)

#add_subdirectory(../lib/Sqlit3-cpp-lib/)
# Vincular el ejecutable con Google Test, la biblioteca principal y otras dependencias necesarias
target_link_libraries(run_tests PRIVATE GTest::GTest GTest::Main Sqlite-cpp)

# Registrar las pruebas con CTest
add_test(NAME ValidatorTests COMMAND run_tests)