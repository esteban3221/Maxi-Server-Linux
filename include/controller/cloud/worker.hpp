#pragma once
#include <thread>
#include <chrono>
#include <atomic>
#include <iostream>

#include "model/log.hpp"
#include "controller/cloud/cloud.h"
#include "controller/cloud/updaper.hpp"

class CloudWorker
{
private:
    std::atomic<bool> m_running{false};
    std::thread m_worker_thread;

    void process_queue();
    void sincronizar_logs_pendientes();

public:
    static CloudWorker &getInstance()
    {
        static CloudWorker instance;
        return instance;
    }

    void start();
    void stop();
};