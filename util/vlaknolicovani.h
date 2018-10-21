#ifndef VLAKNOLICOVANI_H
#define VLAKNOLICOVANI_H

#include <QObject>
#include <QThread>

class VlaknoLicovani : public QThread
{
    Q_OBJECT
public:
    explicit VlaknoLicovani(QObject* parent = nullptr);
private:
    void run();
signals:
    void progress(int);
    void finished();

};

#endif // VLAKNOLICOVANI_H
