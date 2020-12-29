#include "windowsimpl.h"
#include "shared_staff/globalsettings.h"
#include <windows.h>
#include <pdh.h>
#include <conio.h>
#include <pdhmsg.h>

#include <QString>
CONST ULONG SAMPLE_INTERVAL_MS    = 10;

WindowsImpl::WindowsImpl() : SystemMonitor(), mCpuLoadLastValues()
{
    _name = GlobalSettings::getSettings()->getHDDCounterName();
    _parameter = GlobalSettings::getSettings()->getHDDCounterParameter();

    TCHAR szBuffer[MAX_COMPUTERNAME_LENGTH +1];
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(szBuffer, &dwSize); //Get the computer name

    std::wstring s(L"\\\\");
    s+= std::wstring(szBuffer);
    s+= std::wstring(L"\\");
    s+= std::wstring(_name.toStdWString().c_str());
    s+= std::wstring(L"(_total)\\");
    s+= std::wstring(_parameter.toStdWString().c_str());
    fullString = s; /**< complete string containing PC host name, name and parameter of the HDD counter */
}
double WindowsImpl::memoryUsed(){
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memoryStatus);
    qulonglong memoryPhysicalUsed = memoryStatus.ullTotalPhys - memoryStatus.ullAvailPhys;
    return (double(memoryPhysicalUsed) / double(memoryStatus.ullTotalPhys) * 100.0);
}

double WindowsImpl::hddUsed(){

    PDH_STATUS Status;
    HQUERY Query = nullptr;
    HCOUNTER Counter;
    PDH_FMT_COUNTERVALUE DisplayValue;
    DWORD CounterType;
    SYSTEMTIME SampleTime;

    WCHAR CounterPathBuffer[PDH_MAX_COUNTER_PATH];
    Status = PdhOpenQuery(NULL, 0, &Query);

    wcscpy_s(CounterPathBuffer,sizeof(CounterPathBuffer),fullString.c_str());
    Status = PdhAddCounter(Query, CounterPathBuffer, 0, &Counter);
    if (Status != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    Status = PdhCollectQueryData(Query);
    if (Status != ERROR_SUCCESS)
    {
        goto Cleanup;
    }
    while (!_kbhit())
    {
        Sleep(SAMPLE_INTERVAL_MS);
        GetLocalTime(&SampleTime);
        Status = PdhCollectQueryData(Query);
        Status = PdhGetFormattedCounterValue(Counter,
                                             PDH_FMT_DOUBLE,
                                             &CounterType,
                                             &DisplayValue);
        if (Status != ERROR_SUCCESS)
        {
            goto Cleanup;
        }
        double counterValue = DisplayValue.doubleValue;
        PdhRemoveCounter(Counter);
        return counterValue;
    }
Cleanup:
    if (Query)
    {
        PdhCloseQuery(Query);
        return -999.0;
    }
return 0;
}
void WindowsImpl::init()
{
    mCpuLoadLastValues = cpuRawData();
}

QVector<qulonglong> WindowsImpl::cpuRawData()
{
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    GetSystemTimes(&idleTime,&kernelTime,&userTime);
    QVector<qulonglong> rawData;
    rawData.append(convertFileTime(idleTime));
    rawData.append(convertFileTime(kernelTime));
    rawData.append(convertFileTime(userTime));
    return rawData;
}

qulonglong WindowsImpl::convertFileTime(const FILETIME& filetime) const
{
    ULARGE_INTEGER largeInteger;
    largeInteger.LowPart = filetime.dwLowDateTime;
    largeInteger.HighPart = filetime.dwHighDateTime;
    return largeInteger.QuadPart;
}

double WindowsImpl::cpuLoadAverage()
{
    QVector<qulonglong> firstSample = mCpuLoadLastValues;
    QVector<qulonglong> secondSample = cpuRawData();
    mCpuLoadLastValues = secondSample;
    qulonglong currentIdle = secondSample[0] - firstSample[0];
    qulonglong currentKernel = secondSample[1] - firstSample[1];
    qulonglong currentUser = secondSample[2] - firstSample[2];
    qulonglong currentSystem = currentKernel + currentUser;
    double percent = (currentSystem - currentIdle) * 100.0 /currentSystem ;
    return qBound(0.0, percent, 100.0);
}
