#ifndef SYSTEM_INFO_HPP
#define SYSTEM_INFO_HPP

#include <string>
#include <thread>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

namespace json {

struct CPUInfo {
    std::string name;
    unsigned int physicalCores;
    unsigned int logicalCores;
    unsigned int recommendedThreads;
};

class SystemInfo {
public:
    static CPUInfo getCPUInfo() {
        CPUInfo info;

        // Логические ядра (потоки)
        info.logicalCores = std::thread::hardware_concurrency();
        if (info.logicalCores == 0) {
            info.logicalCores = 1;
        }

        // Физические ядра
        info.physicalCores = getPhysicalCores();
        if (info.physicalCores == 0) {
            info.physicalCores = info.logicalCores;
        }

        // Рекомендуемое количество потоков (оставляем 1 ядро для системы)
        info.recommendedThreads = info.logicalCores > 1 ? info.logicalCores - 1 : 1;

        // Имя процессора
        info.name = getCPUName();

        return info;
    }

    static unsigned int getPhysicalCores() {
#ifdef __APPLE__
        int cores;
        size_t size = sizeof(cores);
        if (sysctlbyname("hw.physicalcpu", &cores, &size, nullptr, 0) == 0) {
            return static_cast<unsigned int>(cores);
        }
#endif

#ifdef _WIN32
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return sysInfo.dwNumberOfProcessors;
#endif

#ifdef __linux__
        // На Linux можно читать /proc/cpuinfo
        // Упрощённая версия
        return std::thread::hardware_concurrency() / 2;
#endif

        return std::thread::hardware_concurrency();
    }

    static std::string getCPUName() {
#ifdef __APPLE__
        char buffer[256];
        size_t size = sizeof(buffer);
        if (sysctlbyname("machdep.cpu.brand_string", buffer, &size, nullptr, 0) == 0) {
            return std::string(buffer);
        }
#endif

#ifdef _WIN32
        // Windows: читаем из реестра
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char buffer[256];
            DWORD size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr,
                (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                RegCloseKey(hKey);
                return std::string(buffer);
            }
            RegCloseKey(hKey);
        }
#endif

#ifdef __linux__
        // Linux: читаем /proc/cpuinfo
        FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
        if (cpuinfo) {
            char line[256];
            while (fgets(line, sizeof(line), cpuinfo)) {
                if (strncmp(line, "model name", 10) == 0) {
                    char* colon = strchr(line, ':');
                    if (colon) {
                        fclose(cpuinfo);
                        // Убираем пробелы в начале
                        while (*++colon == ' ');
                        std::string name(colon);
                        // Убираем перенос строки
                        if (!name.empty() && name.back() == '\n') {
                            name.pop_back();
                        }
                        return name;
                    }
                }
            }
            fclose(cpuinfo);
        }
#endif

        return "Unknown CPU";
    }

    static size_t getAvailableMemoryMB() {
#ifdef __APPLE__
        int64_t memsize;
        size_t size = sizeof(memsize);
        if (sysctlbyname("hw.memsize", &memsize, &size, nullptr, 0) == 0) {
            return static_cast<size_t>(memsize / (1024 * 1024));
        }
#endif

#ifdef _WIN32
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        if (GlobalMemoryStatusEx(&statex)) {
            return static_cast<size_t>(statex.ullAvailPhys / (1024 * 1024));
        }
#endif

#ifdef __linux__
        FILE* meminfo = fopen("/proc/meminfo", "r");
        if (meminfo) {
            char line[256];
            while (fgets(line, sizeof(line), meminfo)) {
                if (strncmp(line, "MemAvailable:", 13) == 0) {
                    size_t mem;
                    if (sscanf(line, "MemAvailable: %zu kB", &mem) == 1) {
                        fclose(meminfo);
                        return mem / 1024;
                    }
                }
            }
            fclose(meminfo);
        }
#endif

        return 0;
    }
};

} // namespace json

#endif // SYSTEM_INFO_HPP
