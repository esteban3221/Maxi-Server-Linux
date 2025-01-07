#include "rest_api.hpp"


Rest_api::Rest_api(/* args */)
{
    
    // CROW_ROUTE(Global::Rest::app, "/hello/json").methods(crow::HTTPMethod::Get)
    //   ([]() 
    //   {
    //     crow::json::wvalue response;
    //     response["data"] = "hello";
    //     return response;
    //   });

    

}

Rest_api::~Rest_api()
{
}
