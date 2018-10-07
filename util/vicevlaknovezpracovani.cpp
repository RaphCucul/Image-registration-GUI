#include "vicevlaknovezpracovani.h"
#include "dialogy/multiplevideoet.h"

VicevlaknoveZpracovani::VicevlaknoveZpracovani()
{

}

void VicevlaknoveZpracovani::run()
{

}

int VicevlaknoveZpracovani::percentageComplete(int procento)
{
    MultipleVideoET* MVET = new MultipleVideoET();
    QMetaObject::invokeMethod(MVET, // obj
    "aktualizujProgBar", // member: don't put parameters
    Qt::QueuedConnection, // connection type
    Q_ARG(int, procento)); // val1
    return 0;
}
