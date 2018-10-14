#ifndef LICOVANIDVOU_H
#define LICOVANIDVOU_H

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QGridLayout>
#include <QSignalMapper>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>

/*cv::Point2d hranice(0,0);
void onMouse(int event, int x, int y, int flags, void* data)
{
    if  ( event == CV_EVENT_LBUTTONDOWN )
        {
            hranice.x = x;
            hranice.y = y;
            cv::destroyAllWindows();
        }
}*/

namespace Ui {
class LicovaniDvou;
}

class LicovaniDvou : public QWidget
{
    Q_OBJECT

public:
    explicit LicovaniDvou(QWidget *parent = nullptr);
    ~LicovaniDvou();
    void velikost_frangi_opt(int velikost);
    void inicializace_frangi_opt(QString &hodnota,int &pozice);
    void clearLayout(QGridLayout *layout);
    void VideoLE_textChanged(QLineEdit *LE, QString& s);


public slots:
    //void GetClickCoordinates(QPointF hranice_anomalie);
    void vyberVideaPBWrapper();
    void vyberVideaPB_clicked(QWidget *W);
    void vyberReferenceObrPB_clicked(QWidget *W);
    void vyberPosunutehoObrPB_clicked(QWidget *W);
    void ReferenceObrLE_textChanged(const QString &arg1);
    void PosunutyObrLE_textChanged(const QString &arg1);

signals:

private slots:
    void Slot_VideoLE_textChanged(const QString &s);
    void ReferenceLE_textChanged(const QString &arg1);
    void PosunutyLE_textChanged(const QString &arg1);
    void on_oblastMaxima_textChanged(const QString &arg1);
    void on_uhelRotace_textChanged(const QString &arg1);
    void on_pocetIteraci_textChanged(const QString &arg1);
    void on_slicujDvaSnimky_clicked();
    void zobrazKliknutelnyDialog();
    //void on_comboBox_currentIndexChanged(int index);
    void on_comboBox_activated(int index);



private:
    Ui::LicovaniDvou *ui;
    QVector<QString> rozborVybranehoSouboru;
    QVector<QString> rozborObrReference;
    QVector<QString> rozborObrPosunuty;
    cv::VideoCapture cap;
    int cisloReference = -1;
    int cisloPosunuteho = -1;
    int iterace = -1;
    double oblastMaxima = -1;
    double uhel = 0.1;
    QString vybrane_video="";
    /*****************************************/
    bool spravnostVidea = false;
    bool spravnostReference = false;
    bool spravnostReferenceObr = false;
    bool spravnostPosunutyObr = false;
    bool spravnostPosunuteho = false;
    bool spravnostOblasti = false;
    bool spravnostUhlu = false;
    bool spravnostIteraci = false;
    QPixmap reprezentace_reference;
    bool pritomnost_casove_znacky = false;
    bool pritomnost_svetelne_anomalie = false;
    bool zmena_meritka = false;
    /*******************************************/
    /******************************************/
    /*QLineEdit* vybraneVideoLE_stable = new QLineEdit();
    QPushButton* vyberVideaPB_stable = new QPushButton();
    QLineEdit* cisloReferenceLE_stable = new QLineEdit();
    QLineEdit* cisloPosunutehoLE_stable = new QLineEdit();
    QSpacerItem* horizontalSpacer1_stable = new QSpacerItem(20,20);
    QSpacerItem* horizontalSpacer2_stable = new QSpacerItem(5,20);
    QLineEdit* volbaReferenceLE_stable = new QLineEdit();
    QLineEdit* volbaPosunutehoLE_stable = new QLineEdit();
    QPushButton* vyberObrazekReferencePB_stable = new QPushButton();
    QPushButton* vyberObrazekPosunutyPB_stable = new QPushButton();*/
    /******************************************/
    QVector<double> parametry_frangi;
    cv::Point2f ziskane_hranice_anomalie;
    cv::Point3d maximum_frangi;
    int predchozi_index = 0;
};

#endif // LICOVANIDVOU_H
