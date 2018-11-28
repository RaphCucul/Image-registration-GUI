#include "dialogy/singlevideolicovani.h"
#include "licovani/registrationthread.h"
#include "ui_singlevideolicovani.h"
#include "hlavni_program/t_b_ho.h"
#include "util/licovaniparent.h"
#include "util/souborove_operace.h"
#include "util/prace_s_vektory.h"
#include "analyza_obrazu/pouzij_frangiho.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iterator>

#include <QVector>
#include <QString>
#include <QStringList>
#include <QFileDialog>
#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QObject>
#include <QThread>

SingleVideoLicovani::SingleVideoLicovani(QWidget *parent) : LicovaniParent(parent),
    ui(new Ui::SingleVideoLicovani)
{
    ui->setupUi(this);
    ui->slicovatPB->setEnabled(false);
    ui->ulozitPB->setEnabled(false);
    ui->zobrazVysledekLicovaniPB->setEnabled(false);

    videoParametersDouble["FrangiX"]=snimkyFrangiX;
    videoParametersDouble["FrangiX"]=snimkyFrangiY;
    videoParametersDouble["FrangiEuklid"]=snimkyFrangiEuklid;
    videoParametersDouble["POCX"]=snimkyPOCX;
    videoParametersDouble["POCY"]=snimkyPOCY;
    videoParametersDouble["Uhel"]=snimkyUhel;
    videoParametersInt["Ohodnoceni"]=snimkyOhodnoceniKomplet;    
    videoAnomalies["VerticalAnomaly"]=PritomnostCasoveZnacky;
    videoAnomalies["HorizontalAnomaly"]=PritomnostSvetelneAnomalie;

    velikost_frangi_opt(6,parametryFrangi);
    QFile soubor;
    soubor.setFileName(paramFrangi+"/frangiParameters.json");
    parametryFrangiJS = readJson(soubor);
    QStringList parametry = {"sigma_start","sigma_end","sigma_step","beta_one","beta_two","zpracovani"};
    for (int a = 0; a < 6; a++)
    {
        inicializace_frangi_opt(parametryFrangiJS,parametry.at(a),parametryFrangi,a);
    }

    if (videaKanalyzeAktual == "")
        ui->vybraneVideoLE->setPlaceholderText("Vybrane video");
    else
    {
        QString slozka,jmeno,koncovka;
        QStringList nalezeneSoubory;
        int pocetNalezenych;
        analyzuj_jmena_souboru_avi(videaKanalyzeAktual,nalezeneSoubory,pocetNalezenych,"avi");
        if (pocetNalezenych != 0)
        {
            QString celeJmeno = videaKanalyzeAktual+"/"+nalezeneSoubory.at(0);
            zpracujJmeno(celeJmeno,slozka,jmeno,koncovka);
            if (vybraneVideoLicovaniSingle.length() == 0)
            {
                vybraneVideoLicovaniSingle.push_back(slozka);
                vybraneVideoLicovaniSingle.push_back(jmeno);
                vybraneVideoLicovaniSingle.push_back(koncovka);
            }
            else
            {
                vybraneVideoLicovaniSingle.clear();
                vybraneVideoLicovaniSingle.push_back(slozka);
                vybraneVideoLicovaniSingle.push_back(jmeno);
                vybraneVideoLicovaniSingle.push_back(koncovka);
            }
            ui->vybraneVideoLE->setText(jmeno);
        }
        QFile videoParametersFile(TXTulozeniAktual+vybraneVideoLicovaniSingle[1]+".dat");
        videoParametersJson = readJson(videoParametersFile);
        processVideoParameters(videoParametersJson);
    }
}

SingleVideoLicovani::~SingleVideoLicovani()
{
    delete ui;
}

void SingleVideoLicovani::on_vybraneVideoLE_textChanged(const QString &arg1)
{
    QString kompletni_cesta = vybraneVideoLicovaniSingle[0]+"/"+arg1+"."+vybraneVideoLicovaniSingle[2];
    cv::VideoCapture cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        ui->vybraneVideoLE->setStyleSheet("color: #339900");
        vybraneVideoLicovaniSingle[1] = arg1;
    }
}

void SingleVideoLicovani::on_vyberVideaPB_clicked()
{
    fullVideoPath = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    zpracujJmeno(fullVideoPath,vybrana_slozka,vybrany_soubor,koncovka);
    cv::VideoCapture cap = cv::VideoCapture(fullVideoPath.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        ui->vybraneVideoLE->setText(vybrany_soubor);
        ui->vybraneVideoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
    }
    else
    {
        if (vybraneVideoLicovaniSingle.length() == 0)
        {
            vybraneVideoLicovaniSingle.push_back(vybrana_slozka);
            vybraneVideoLicovaniSingle.push_back(vybrany_soubor);
            vybraneVideoLicovaniSingle.push_back(koncovka);
        }
        else
        {
            vybraneVideoLicovaniSingle.clear();
            vybraneVideoLicovaniSingle.push_back(vybrana_slozka);
            vybraneVideoLicovaniSingle.push_back(vybrany_soubor);
            vybraneVideoLicovaniSingle.push_back(koncovka);
        }
        ui->vybraneVideoLE->setText(vybraneVideoLicovaniSingle[1]);
        ui->vybraneVideoLE->setStyleSheet("color: #339900");
        QString dir = TXTnacteniAktual;
        QDir chosenDirectory(dir);
        QStringList JsonInDirectory = chosenDirectory.entryList(QStringList() << "*.dat" << "*.DAT",QDir::Files);
        for (int a = 0; a < JsonInDirectory.count(); a++)
        {
            if (JsonInDirectory.at(a) == (vybraneVideoLicovaniSingle[1]+".dat"))
            {
                if (vybranyJSONLicovaniSingle.length() == 0)
                {
                    vybranyJSONLicovaniSingle.push_back(TXTnacteniAktual);
                    vybranyJSONLicovaniSingle.push_back(vybraneVideoLicovaniSingle[1]);
                    vybranyJSONLicovaniSingle.push_back("dat");
                }
                else
                {
                    vybranyJSONLicovaniSingle.clear();
                    vybranyJSONLicovaniSingle.push_back(TXTnacteniAktual);
                    vybranyJSONLicovaniSingle.push_back(vybraneVideoLicovaniSingle[1]);
                    vybranyJSONLicovaniSingle.push_back("dat");
                }
            }
        }

    }
}

void SingleVideoLicovani::slicovatSnimkyVidea()
{
    QGridLayout* gridForScroll = qobject_cast<QGridLayout * >(widgetForScroll->layout());
    Q_ASSERT(gridForScroll);
    int numberOfThreads = QThread::idealThreadCount();
    seznamVidei.append(fullVideoPath);
    cv::VideoCapture cap = cv::VideoCapture(seznamVidei.at(0).toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        qWarning()<<"Unable to open "+seznamVidei.at(0);
    }
    double frameCount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    QVector<QVector<int>> threadRange = divideIntoPeaces(int(frameCount),numberOfThreads);
    for (int indexThreshold = 0; indexThreshold < threadRange[0].length(); indexThreshold++){
        createAndRunThreads(0,cap,threadRange[0][indexThreshold],threadRange[1][indexThreshold]);
    }
}

void SingleVideoLicovani::createAndRunThreads(int indexProcVid, cv::VideoCapture &cap,
                                              int lowerLimit, int upperLimit)
{

    int cisloReference = findReferenceFrame(snimkyOhodnoceniKomplet[indexProcVid]);
    cv::Mat referencniSnimek;
    cap.set(CV_CAP_PROP_POS_FRAMES,cisloReference);
    if (!cap.read(referencniSnimek))
        qWarning()<<"Frame "+QString::number(cisloReference)+" cannot be opened.";
    QProgressBar *progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    QGridLayout *layout = qobject_cast<QGridLayout*>(ui->scrollArea->layout());
    Q_ASSERT(layout);
    RegistrationThread *regThread = new RegistrationThread(cap,referencniSnimek,lowerLimit,upperLimit,
                                                           -1,10.0,0.1,
                                                           videoAnomalies["VerticalAnomaly"][0],
                                                           videoAnomalies["HorizontalAnomaly"][0],
                                                           false);
    regThread->start();
}

void SingleVideoLicovani::processVideoParameters(QJsonObject &videoData)
{
    for (int parameter = 0; parameter < videoParameters.count(); parameter++){
        if (parameter < 6){
            QVariant variantDouble = videoData[videoParameters.at(parameter)].toVariant();
            QVector<double> pomDouble = variantDouble.value<QVector<double>>();
            videoParametersDouble[videoParameters.at(parameter)].append(pomDouble);
        }
        if (parameter == 6){
            QVariant variantInt = videoData[videoParameters.at(parameter)].toVariant();
            QVector<int> pomInt = variantInt.value<QVector<int>>();
            videoParametersInt[videoParameters.at(parameter)].append(pomInt);
        }
        if (parameter > 6){
            int anomaly = videoData[videoParameters.at(parameter)].toInt();
            videoAnomalies[videoParameters.at(parameter)].push_back(anomaly);
        }
    }
}
/*void SingleVideoLicovani::on_slozkaParametruLE_textChanged(const QString &arg1)
{

}

void SingleVideoLicovani::on_slozkaParametruPB_clicked()
{
    QString kompletnicesta = QFileDialog::getOpenFileName(this,
                                                          "Vyberte soubor s parametry", videaKanalyzeAktual,"*.avi;;Všechny soubory (*)");
    QString vybrana_slozka,vybrany_soubor,koncovka;
    zpracujJmeno(kompletnicesta,vybrana_slozka,vybrany_soubor,koncovka);
    QString dir = TXTnacteniAktual;
    QDir chosenDirectory(dir);
    QStringList videosInDirectory = chosenDirectory.entryList(QStringList() << "*.json" << "*.JSON",QDir::Files);

    ui->slozkaParametruLE->setStyleSheet("color: #339900");
    if (vybranyJSONLicovaniSingle.length() == 0)
    {
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    else
    {
        vybranyJSONLicovaniSingle.clear();
        vybranyJSONLicovaniSingle.push_back(vybrana_slozka);
        vybranyJSONLicovaniSingle.push_back(vybrany_soubor);
        vybranyJSONLicovaniSingle.push_back(koncovka);
    }
    ui->vybraneVideoLE->setText(vybranyJSONLicovaniSingle[1]);


}

void SingleVideoLicovani::on_slicovatPB_clicked()
{

}

void SingleVideoLicovani::on_zobrazVysledekLicovaniPB_clicked()
{

}

void SingleVideoLicovani::on_grafickeHodnoceniPB_clicked()
{

}

void SingleVideoLicovani::on_ulozitPB_clicked()
{

}

void SingleVideoLicovani::on_FF_PB_clicked()
{

}

void SingleVideoLicovani::on_parametryFF_LE_textChanged(const QString &arg1)
{

}*/
