#ifndef KRUHOVYPROGRESSBAR_H
#define KRUHOVYPROGRESSBAR_H

#include <QWidget>
class QPaintEvent;
class KruhovyProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit KruhovyProgressBar(QWidget *parent = nullptr);
    void setLoadingAngle(int loadingAngle);
    int loadingAngle() const;

    void setDiscWidth(int width);
    int discWidth() const;
protected:
     void paintEvent(QPaintEvent *);
private:
     int m_loadingAngle;
     int m_width;
signals:

public slots:
};

#endif // KRUHOVYPROGRESSBAR_H
