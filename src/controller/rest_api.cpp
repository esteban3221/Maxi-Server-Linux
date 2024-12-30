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

    Global::Rest::future = Global::Rest::app.port(8080).run_async();

}

Rest_api::~Rest_api()
{
}
