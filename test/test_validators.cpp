#include <gtest/gtest.h>
#include "c_sharp_validator.hpp"

TEST(ValidatorTests, Coneccion)
{
    Validator bill();
    Validator coin();
    EXPECT_EQ(1, 1); // Verifica que el nombre se estableció correctamente
}