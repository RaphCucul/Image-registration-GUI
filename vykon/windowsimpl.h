#ifndef WINDOWSIMPL_H
#define WINDOWSIMPL_H
#include "util/systemmonitor.h"
#include <QtGlobal>
#include <QVector>

typedef struct _FILETIME FILETIME;
/* forward declaration for  FILENAME  syntax. As we only use a reference, we can avoid
including the  <windows.h>  tag in our file  SysInfoWindowsImpl.h  and keep it
in the CPP file.*/

class WindowsImpl : public SystemMonitor
{
public:
    explicit WindowsImpl();

    void init() override;
    double cpuLoadAverage() override;
    double memoryUsed() override;
    double hddUsed() override;
    // You can now override virtual functions defined in the base class.
    /* The  override  keyword comes from C++11. It ensures that the function is declared as
    virtual in the base class. If the function signature marked as  override  does not match any
    parent class'  virtual  function, a compile-time error will be displayed.*/

private:
    QVector<qulonglong> cpuRawData();
    /* will perform the Windows API call to
    retrieve system timing information and return values in a generic format. We will
    retrieve and return three values: the amount of time that the system has spent in
   idle, in Kernel, and in User mode.*/

    qulonglong convertFileTime(const FILETIME& filetime) const;
    /*will convert a Windows  FILETIME struct  syntax to a  qulonglong  type. The  qulonglong
    type is a Qt  unsigned long long int . This type is guaranteed by Qt to be 64-
    bit on all platforms. You can also use the typedef  quint64 .*/

    QVector<qulonglong> mCpuLoadLastValues;
    // will store system timing (idle, Kernel, and User) at a given moment.

    QString _name,_parameter;
};

#endif // WINDOWSIMPL_H
