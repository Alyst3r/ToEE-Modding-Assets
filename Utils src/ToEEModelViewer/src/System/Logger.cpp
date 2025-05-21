#include "Logger.hpp"

#include <fstream>
#include <ctime>
#include <iomanip>
#include <windows.h>
#include <filesystem>

static std::filesystem::path dirname = std::filesystem::path(__FILE__).parent_path();

Logger logger;

Logger::Logger()
{
#ifndef NDEBUG
    m_file.open("log.txt", std::ios::out);
#endif
}

static bool isFirst = true;
Logger& log(const char* type, const char* file, size_t line)
{
#ifndef NDEBUG
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    if (isFirst)
    {
        isFirst = false;
    }
    else
    {
        logger << "\n";
    }
    logger << "[" << type << "][" << std::put_time(&tm, "%H:%M:%S") << "][" << relProjectPath(file) << ":" << line << "] ";
#endif
    return logger;
}

std::string relProjectPath(std::string const& pathIn)
{
    return std::filesystem::relative(pathIn, dirname).string();
}
