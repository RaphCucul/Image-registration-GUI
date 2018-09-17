#ifndef ZASOBNIK_STRANEK_H
#define ZASOBNIK_STRANEK_H

#include <QWidget>
#include <QTabWidget>

class Zasobnik_stranek : public QWidget
{
    Q_OBJECT
public:
    explicit Zasobnik_stranek(QWidget *parent = nullptr);
private:
    QTabWidget* zasobnik;
signals:

public slots:
};

#endif // ZASOBNIK_STRANEK_H
