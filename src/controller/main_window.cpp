#include "controller/main_window.hpp"

MainWindow::MainWindow(/* args */) 
{
    Global::Rest::future = Global::Rest::app.port(44333).run_async(); 


    
}

MainWindow::~MainWindow()
{
    Global::Rest::app.stop();
    Global::Rest::future.get();
}