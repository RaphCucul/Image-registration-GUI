#ifndef VICEVLAKNOVEZPRACOVANI_H
#define VICEVLAKNOVEZPRACOVANI_H
#include <QThread>

class VicevlaknoveZpracovani : public QThread
{
public:
    VicevlaknoveZpracovani();
    void run();
signals:
     void percentageComplete(int);
};

#endif // VICEVLAKNOVEZPRACOVANI_H
