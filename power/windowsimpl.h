#ifndef WINDOWSIMPL_H
#define WINDOWSIMPL_H
#include "util/systemmonitor.h"
#include <QtGlobal>
#include <QVector>
#include <cstdio>

typedef struct _FILETIME FILETIME;
/* forward declaration for  FILENAME  syntax. As we only use a reference, we can avoid
including the  <windows.h>  tag in our file  SysInfoWindowsImpl.h  and keep it
in the CPP file.*/

/**
 * @class WindowsImpl
 * @brief The WindowsImpl class is derived from the SystemMonitor class. It contains functions calculating the usage
 * of CPU, RAM and HDD. All these functions are using Windows API or Windows functions in some way.
 */
class WindowsImpl : public SystemMonitor
{
public:
    explicit WindowsImpl();

    /**
     * @brief Overridden init function calling cpuRawData function.
     * @sa cpuRawData()
     */
    void init() override;
    /**
     * @brief Overridden function calculates true CPU usage from the FILETIME struct variables.
     * @return CPU usage in percentage
     * @sa cpuRawData()
     */
    double cpuLoadAverage() override;
    /**
     * @brief Overridden function calculates real RAM memory usage. Not so difficult as the CPU calculation.
     * @return RAM memory usage in percentage
     */
    double memoryUsed() override;
    /**
     * @brief Overridden function calculates total HDD usage. Requires translated names of the counter and specific
     * parameter (the translation is done automatically when ps1 script runs). These two parameters are combined
     * with the name of PC.
     * @return
     */
    double hddUsed() override;

private:
    /**
     * @brief It performs the Windows API call to retrieve system timing information and returns values in a generic format.
     * @return Three values - time in idle, Kernel and User mode
     */
    QVector<qulonglong> cpuRawData();

    /**
     * @brief It converts a Windows  FILETIME struct syntax to a qulonglong type. The qulonglong
     * type is a Qt unsigned long long int. This type is guaranteed by Qt to be 64-
     * bit on all platforms. You can also use the typedef  quint64 .
     * @param filetime
     * @return Converted FILETIME variable
     * @sa cpuRawData()
     */
    qulonglong convertFileTime(const FILETIME& filetime) const;

    QVector<qulonglong> mCpuLoadLastValues; /**< stores system timing (idle, Kernel, and User) at a given moment */

    QString _name,_parameter;
    std::wstring fullString; /**< Complete path with PC name and HDD counter details */
};

#endif // WINDOWSIMPL_H
