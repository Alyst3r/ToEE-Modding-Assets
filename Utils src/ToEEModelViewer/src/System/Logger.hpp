#pragma once

#include <filesystem>
#include <fstream>
#include <string>

std::string relProjectPath(std::string const& pathIn);

class Logger
{
public:
    Logger();

    template <typename T>
    Logger& operator<<(T const& obj)
    {
#if LOG_LEVEL > 0
        m_file << obj;
        m_file.flush();
#endif
        return *this;
    }
private:
    std::ofstream m_file;
};

Logger& log(const char* chr, const char* file, size_t line);

#ifndef NDEBUG
#define LOG_DEBUG log("DEBUG", __FILE__, __LINE__)
#define LOG_ERROR log("ERROR", __FILE__, __LINE__)
#define LOG_INFO log("INFO", __FILE__, __LINE__)
#define LOG_WARN log("WARN", __FILE__, __LINE__)
#else
#define LOG_DEBUG if(false) log("DEBUG", 0,0)
#define LOG_ERROR if(false) log("ERROR", 0,0)
#define LOG_INFO if(false) log("INFO", 0,0)
#define LOG_WARN if(false) log("WARN", 0,0)
#endif

