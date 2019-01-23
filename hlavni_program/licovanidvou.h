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
#include <QJsonObject>

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>

namespace Ui {
class LicovaniDvou;
}

class LicovaniDvou : public QWidget
{
    Q_OBJECT

public:
    explicit LicovaniDvou(QWidget *parent = nullptr);
    ~LicovaniDvou();    
    void clearLayout(QGridLayout *layout);
    //void VideoLE_textChanged(QLineEdit *LE, QString& s);
    void checkPaths();

public slots:
    //void GetClickCoordinates(QPointF hranice_anomalie);
    void chosenVideoPBWrapper();
    void chosenVideoPB_clicked(QWidget *W);
    void chosenReferenceImgPB_clicked(QWidget *W);
    void chosenTranslatedImgPB_clicked(QWidget *W);
    void ReferenceImgLE_textChanged(const QString &arg1);
    void TranslatedImgLE_textChanged(const QString &arg1);

signals:
    void checkRegistrationPass();

private slots:
    void Slot_VideoLE_textChanged(const QString &s);
    void ReferenceLE_textChanged(const QString &arg1);
    void TranslatedLE_textChanged(const QString &arg1);
    void on_areaMaximum_textChanged(const QString &arg1);
    void on_rotationAngle_textChanged(const QString &arg1);
    void on_iterationCount_textChanged(const QString &arg1);
    void on_registrateTwo_clicked();
    void showDialog();
    void evaluateCorrectValues();
    //void on_comboBox_currentIndexChanged(int index);
    void on_comboBox_activated(int index);



private:
    Ui::LicovaniDvou *ui;

    void initChoiceOneInnerWidgets();
    void initChoiceTwoInnerWidgets();
    void placeChoiceOneWidgets();
    void placeChoiceTwoWidgets();
    void analyseAndSaveFirst(QString analysedFolder, QVector<QString>& whereToSave);
    void analyseAndSave(QString analysedFolder, QVector<QString>& whereToSave);
    void evaluateVideoImageInput(QString path,QString method);

    QVector<QString> chosenVideoAnalysis;
    QVector<QString> chosenReferencialImgAnalysis;
    QVector<QString> chosenTranslatedImgAnalysis;
    cv::VideoCapture cap;
    int referenceNumber = -1;
    int translatedNumber = -1;
    int iteration = -1;
    double areaMaximum = -1;
    double angle = 0.1;
    QString chosenVideo="";

    bool videoCorrect = false;
    bool referenceCorrect = false;
    bool referencialImgCorrect = false;
    bool translatedImgCorrect = false;
    bool translatedCorrect = false;
    bool areaMaximumCorrect = false;
    bool angleCorrect = false;
    bool iterationCorrect = false;
    bool horizontalAnomalyPresent = false; // time stamp
    bool verticalAnomalyPresent = false; // light anomaly
    bool scaleChanged = false;

    QVector<double> FrangiParametersVector;
    cv::Point2f horizontalAnomalyCoords;
    cv::Point2f verticalAnomalyCoords;
    cv::Point3d frangiMaximumCoords;
    int formerIndex = 0;
    QJsonObject FrangiParametersFile;
    QLineEdit* chosenVideoLE;
    QPushButton* chosenVideoPB;
    QLineEdit* referenceNoLE;
    QLineEdit* translatedNoLE;

    QSpacerItem* horizontalSpacer1;
    QSpacerItem* horizontalSpacer2;

    QPushButton* chooseReferencialImagePB;
    QPushButton* chooseTranslatedImagePB;
    QLineEdit* referenceImgLE;
    QLineEdit* translatedImgLE;
};

#endif // LICOVANIDVOU_H
