#include "util/systemmonitor.h"
#include "power/windowsimpl.h"

SystemMonitor::SystemMonitor()
{
}

SystemMonitor::~SystemMonitor()
{
}

SystemMonitor &SystemMonitor::instance()
{
    // Q_OS_WIN
    static WindowsImpl singleton;
    return singleton;
}
