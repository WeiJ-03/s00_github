#ifndef N900_MODEL_LOGGER_H
#define N900_MODEL_LOGGER_H

#include <memory>
#include <mutex>

//#include "spdlog/spdlog.h"

#ifdef PROFILE_NODES
#include "profc.h"
#define SMART_PROFILE(node_name) PROFC_NODE(node_name)
#else
#define SMART_PROFILE(node_name) {}
#endif


//namespace log {
//    class Logger {
//    private:
//        static std::shared_ptr<spdlog::logger> logger_instance_;
//        static std::mutex mutex_;
//        InferenceLogger();
//
//    public:
//        static std::shared_ptr<spdlog::logger> GetLogger();
//    };
//}

#endif //N900_MODEL_LOGGER_H
