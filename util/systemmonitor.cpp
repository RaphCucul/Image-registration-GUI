#include "util/systemmonitor.h"
#include "vykon/windowsimpl.h"

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
