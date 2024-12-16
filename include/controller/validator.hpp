#include <utility>
#include <iostream>
#include <cpr/cpr.h>
#include "global.hpp"

class Validator
{
private:
    cpr::Response r;
    const std::string VALIDATOR;
public:
    std::pair<int, std::string> Command_Post(const std::string &command);
    std::pair<int, std::string> Command_Post(const std::string &command, const std::string &json);

    Validator(const std::string &validator);
    ~Validator();
};
