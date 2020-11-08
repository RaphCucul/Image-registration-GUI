#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

namespace Ui {
class SystemMonitor;
}

/**
 * @class SystemMonitor
 * @brief Contains virtual functions for derived classes to perform data acquision
 * for CPU, RAM and HDD graphs. The program contains implementation for Windows style data acquisition. Therefore
 * only one WindowsImpl class exists as a derived class from SystemMonitor class. However, if the program
 * would be ported on another system, another derived class can be created.
 */
class SystemMonitor
{

public:
    /**
     * @brief Creates and returns a WindowsImp class object.
     */
    static SystemMonitor& instance();
    ~SystemMonitor();

    /**
     * @brief Allows to perform any initialization
     * process depending on the OS platform.
     */
    virtual void init() = 0;

    /**
     * @brief Calls some OS-specific code to retrieve the
     * average CPU load.
     * @return Actual value of CPU usage
     */
    virtual double cpuLoadAverage() = 0;

    /**
     * @brief It calls some OS-specific code to retrieve the memory
     * used.
     * @return Actual value of RAM usage
     */
    virtual double memoryUsed() = 0;

    /**
     * @brief It calls some OS-specific code to retrieve HDD usage.
     *
     * Keep in mind that the HDD usage
     * requires specific information about the current PC on which the program is running. This
     * information can be obtained by calling Powershell script (available in the repository).
     * @return Actual value of HDD usage
     */
    virtual double hddUsed() = 0;
protected:
    explicit SystemMonitor();
private:
    Ui::SystemMonitor *ui;
private:
    SystemMonitor(const SystemMonitor& rhs);
    SystemMonitor& operator=(const SystemMonitor& rhs);
};

#endif // SYSTEMMONITOR_H
