
add_executable(run_tests test_main.cpp test_validators.cpp)

# include_directories(../lib/Sqlit3-cpp-lib/include
#                     ../include/controller )
# add_executable( ValidatorTests ../src/controller/c_sharp_validator_validator.cpp )
# Vincular el ejecutable con Google Test, la biblioteca principal y otras dependencias necesarias
target_link_libraries(run_tests PRIVATE GTest::GTest GTest::Main Sqlite-cpp)

# Registrar las pruebas con CTest
add_test(NAME ValidatorTests COMMAND run_tests)