#ifndef VICEVLAKNOVEZPRACOVANI_H
#define VICEVLAKNOVEZPRACOVANI_H
#include <QThread>

class VicevlaknoveZpracovani : public QThread
{
public:
    VicevlaknoveZpracovani();
    void run();
    int percentageComplete(int procento);
signals:

};

#endif // VICEVLAKNOVEZPRACOVANI_H
