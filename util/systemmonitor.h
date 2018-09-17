#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

namespace Ui {
class SystemMonitor;
}

class SystemMonitor
{

public:
    static SystemMonitor& instance();
    ~SystemMonitor();
    virtual void init() = 0;
    // This function allows the derived class to perform any initialization
    // process depending on the OS platform

    virtual double cpuLoadAverage() = 0;
   // This function calls some OS-specific code to retrieve the
   // average CPU load and returns it as a percentage value

    virtual double memoryUsed() = 0;
    // This function calls some OS-specific code to retrieve the memory
    // used and returns it as a percentage value

    // The  virtual  keyword indicates that the function can be overridden in a derived class.
    /* The  = 0  syntax means that this function is pure virtual, and must be overridden in any
    concrete derived class. Moreover, this makes  SysInfo  an abstract class that cannot be
    instantiated.*/
    virtual double hddUsed() = 0;
    // Funkce analyzující procentuální vytížení disku
protected:
    explicit SystemMonitor();
private:
    Ui::SystemMonitor *ui; // pozor na toto, inicializuje se ui pro následující odvozené widgety
private:
    SystemMonitor(const SystemMonitor& rhs); // copy constructor
    SystemMonitor& operator=(const SystemMonitor& rhs);
    /* The singleton must guarantee that there will be only one instance of the class and that this
    instance will be easily accessible from a single access point.*/
};

#endif // SYSTEMMONITOR_H
