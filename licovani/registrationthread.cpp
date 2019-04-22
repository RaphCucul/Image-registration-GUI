#include "licovani/registrationthread.h"
#include "licovani/multiPOC_Ai1.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "analyza_obrazu/korelacni_koeficient.h"
#include "analyza_obrazu/pouzij_frangiho.h"
#include "licovani/fazova_korelace_funkce.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include <QTableWidgetItem>
#include "analyza_obrazu/pouzij_frangiho.h"
#include <exception>
using cv::Mat;
using cv::VideoCapture;
using cv::Point3d;
using cv::Rect;

RegistrationThread::RegistrationThread(int& indexOfThread,
                                       QString &fullVideoPath,
                                       QString& nameOfVideo,
                                       QVector<double>& frangiParam,
                                       QVector<int>& frameEvaluation,
                                       cv::Mat& referencni_snimek,
                                       int& startFrame,
                                       int& stopFrame,
                                       int& iterace,
                                       double& oblastMaxima,
                                       double& uhel,
                                       int &timeStamp,
                                       int &lightAnomaly,
                                       bool nutnost_zmenit_velikost_snimku,
                                       QObject *parent) : QThread(parent),
    referencialImage(referencni_snimek),
    frangiParameters(frangiParam),
    ohodnoceniSnimku(frameEvaluation),
    iteration(iterace),
    startingFrame(startFrame),
    stoppingFrame(stopFrame),
    maximalArea(oblastMaxima),
    angle(uhel),
    casovaZnacka(timeStamp),
    svetelAnomalie(lightAnomaly),
    scaling(nutnost_zmenit_velikost_snimku),
    threadIndex(indexOfThread),
    videoName(nameOfVideo),
    videoPath(fullVideoPath)
{
    emit setTerminationEnabled(true);
    capture = cv::VideoCapture(videoPath.toLocal8Bit().constData());
    QVector<double> pomD(static_cast<int>(capture.get(CV_CAP_PROP_FRAME_COUNT)),0.0);
    frangiX = pomD;frangiY = pomD;frangiEuklidean = pomD;finalPOCx = pomD;finalPOCy = pomD;maximalAngles = pomD;
    setTerminationEnabled(true);
    qDebug()<<"registration thread initialized";
}

QMap<QString,QVector<double>> RegistrationThread::provideResults()
{
    vectors["FrangiX"] = frangiX;
    vectors["FrangiY"] = frangiY;
    vectors["FrangiEuklid"] = frangiEuklidean;
    vectors["POCX"] = finalPOCx;
    vectors["POCY"] = finalPOCy;
    vectors["Uhel"] = maximalAngles;
    return vectors;
}

QVector<int> RegistrationThread::threadFrameRange(){
    QVector<int> output;
    output.push_back(startingFrame);
    output.push_back(stoppingFrame);
    return output;
}

bool registrateTheBest(cv::VideoCapture& i_cap,
                       cv::Mat& i_referencialFrame,
                       cv::Point3d i_coordsFrangiStandardReferencialReverse,
                       int i_index_translated,
                       int i_iteration,
                       double i_areaMaximum,
                       double i_angle,
                       cv::Rect& i_cutoutExtra,
                       cv::Rect& i_cutoutStandard,
                       bool i_scaleChanged,
                       QVector<double> &parametry_frangi,
                       QVector<double> &_pocX,
                       QVector<double> &_pocY,
                       QVector<double> &_frangiX,
                       QVector<double> &_frangiY,
                       QVector<double> &_frangiEucl,
                       QVector<double> &_maxAngles);

bool fullRegistration(cv::VideoCapture& cap,
                      cv::Mat& referencni_snimek,
                      int cislo_posunuty,
                      int iterace,
                      double oblastMaxima,
                      double uhel,
                      cv::Rect& korelacni_vyrez_navic,
                      cv::Rect& korelacni_vyrez_standardni,
                      bool nutnost_zmenit_velikost_snimku,
                      cv::Mat& slicovany_kompletne,
                      QVector<double> &_pocX,
                      QVector<double> &_pocY,
                      QVector<double> &_maxAngles);

bool imagePreprocessing(cv::Mat &reference,
                        cv::Mat &obraz,
                        QVector<double> &parFrang,
                        cv::Point2f &hraniceAnomalie,
                        cv::Point2f &hraniceCasu,
                        cv::Rect &oblastAnomalie,
                        cv::Rect &vyrezKoreEx,
                        cv::Rect &vyrezKoreStand,
                        cv::VideoCapture &cap,
                        bool &zmeMer);

bool registrationCorrection(cv::Mat& slicovany_snimek,
                            cv::Mat& obraz,
                            cv::Mat& snimek_korigovany,
                            cv::Rect& vyrez_korelace_standard,
                            cv::Point3d& korekce_bod);

void RegistrationThread::run()
{
    /// define helpers for frame preprocessing
    bool lightAnomalyPresent = false;
    bool timeStampPresent = false;
    bool scaling=false;
    /// frame and video properties
    double sirka = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

    if (svetelAnomalie >0.0f && svetelAnomalie < float(sirka))
    {
        ziskane_hranice_anomalie.x = svetelAnomalie;
        ziskane_hranice_anomalie.y = 0;
        lightAnomalyPresent=true;
        scaling = true;
    }
    else
    {
        ziskane_hranice_anomalie.x = 0.0f;
        ziskane_hranice_anomalie.y = 0.0f;
    }
    if (casovaZnacka > 0.0f && casovaZnacka < float(vyska))
    {
        ziskane_hranice_CasZnac.y = casovaZnacka;
        ziskane_hranice_CasZnac.x = 0;
        timeStampPresent=true;
        scaling=true;
    }
    else
    {
        ziskane_hranice_CasZnac.y = 0.0f;
        ziskane_hranice_CasZnac.x = 0.0f;
    }
    Rect pom(0,0,0,0);
    correl_standard = pom;
    correl_extra = pom;
    anomalyCutoff = pom;
    Point3d pt_temp(0.0,0.0,0.0);
    Mat obraz,obraz_vyrez;
    if(!imagePreprocessing(referencialImage,
                           obraz,
                           frangiParameters,
                           ziskane_hranice_anomalie,
                           ziskane_hranice_CasZnac,
                           anomalyCutoff,
                           correl_extra,
                           correl_standard,
                           capture,
                           scaling)){
        emit errorDetected(threadIndex,QString(tr("Frame preprocessing failed for %1.")).arg(videoName));
        return;
    }
    obraz(correl_standard).copyTo(obraz_vyrez);
    Point3d frangiMaxReversal = frangi_analysis(obraz,2,2,0,"",1,pt_temp,frangiParameters);
    if (frangiMaxReversal.z == 0.0){
        emit errorDetected(threadIndex,QString(tr("Frangi filter for referencial image failed for %1.")).arg(videoName));
        return;
    }
    //qDebug()<<"Frangi reversal: "<<frangiMaxReversal.x<<" "<<frangiMaxReversal.y;
    for (int indexFrame = startingFrame; indexFrame <= stoppingFrame; indexFrame++)
    {
        bool errorOccured = false;
        if (ohodnoceniSnimku[indexFrame] == 0){
            if (!registrateTheBest(capture,referencialImage,frangiMaxReversal,indexFrame,iteration,maximalArea,
                                   totalAngle,correl_extra,correl_standard,scaling,frangiParameters,
                                   finalPOCx,finalPOCy,frangiX,frangiY,frangiEuklidean,maximalAngles)){
                errorOccured = true;
                continue;
            }
        }
        else if (ohodnoceniSnimku[indexFrame] == 2)
        {
            Mat posunuty_temp;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(posunuty_temp)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            else
            {
                if (scaling == true)
                {
                    Mat posunuty;
                    posunuty_temp(correl_extra).copyTo(posunuty);
                    Point3d frangi_bod_obraz_reverse = frangi_analysis(posunuty,2,2,0,"",1,pt_temp,frangiParameters);
                    if (frangi_bod_obraz_reverse.z == 0.0){
                        errorOccured = true;
                        continue;
                    }
                    frangiX[indexFrame] = frangi_bod_obraz_reverse.x;
                    frangiY[indexFrame] = frangi_bod_obraz_reverse.y;
                    frangiEuklidean[indexFrame] = 0.0;
                    //qDebug() << "Referencial frame "<<indexFrame<<" analysed.";
                    posunuty_temp.release();
                    posunuty.release();
                }
                else
                {
                    Point3d frangi_bod_obraz_reverse = frangi_analysis(posunuty_temp,2,2,0,"",1,pt_temp,frangiParameters);
                    if (frangi_bod_obraz_reverse.z == 0.0){
                        errorOccured = true;
                        continue;
                    }
                    //qDebug() << "Referencial frame "<<indexFrame<<" analysed.";
                    posunuty_temp.release();
                    frangiX[indexFrame] = frangi_bod_obraz_reverse.x;
                    frangiY[indexFrame] = frangi_bod_obraz_reverse.y;
                    frangiEuklidean[indexFrame] = 0.0;
                }
                maximalAngles[indexFrame] = 0.0;
                finalPOCx[indexFrame] = 0.0;
                finalPOCy[indexFrame] = 0.0;
            }
        }
        else if (ohodnoceniSnimku[indexFrame] == 5)
        {
            Mat posunuty;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(posunuty)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                errorOccured = true;
                continue;
            }
            else
            {
                //qDebug() << "Frame "<<indexFrame<<" written without any changes.";
                posunuty.release();
                frangiX[indexFrame] = 999.0;
                frangiY[indexFrame] = 999.0;
                frangiEuklidean[indexFrame] = 999.0;
                maximalAngles[indexFrame] = 999;
                finalPOCx[indexFrame] = 999.0;
                finalPOCy[indexFrame] = 999.0;
            }
        }
        else if (ohodnoceniSnimku[indexFrame] == 1 || ohodnoceniSnimku[indexFrame] == 4)
        {
            Mat slicovany_snimek;
            if (!fullRegistration(capture,
                                  referencialImage,
                                  indexFrame,
                                  iteration,
                                  maximalArea,
                                  angle,
                                  correl_extra,
                                  correl_standard,
                                  scaling,
                                  slicovany_snimek,
                                  finalPOCx,
                                  finalPOCy,
                                  maximalAngles)){
                errorOccured = true;
                continue;
            }
            Mat posunuty_temp;
            capture.set(CV_CAP_PROP_POS_FRAMES,indexFrame);
            if (capture.read(posunuty_temp)!=1)
            {
                qWarning()<<"Frame "<<indexFrame<<" could not be read!";
                continue;
            }
            else
            {
                Point3d korekce_translace(0.0,0.0,0.0);
                cv::Mat plne_slicovany_snimek;
                if (!registrationCorrection(slicovany_snimek,obraz,plne_slicovany_snimek,
                                            correl_standard,
                                            korekce_translace)){
                    errorOccured = true;
                    continue;
                }

                if (korekce_translace.x != 0.0 || korekce_translace.y != 0.0)
                {
                    finalPOCx[indexFrame] += korekce_translace.x;
                    finalPOCy[indexFrame] += korekce_translace.y;
                    Point3d pt6 = fk_translace_hann(obraz,plne_slicovany_snimek);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(obraz,plne_slicovany_snimek);
                    }
                    //qDebug() << "After correction of translation Y: " << pt6.y <<" X: "<<pt6.x;
                }
                Point3d _tempTranslation = Point3d(finalPOCx[indexFrame],finalPOCy[indexFrame],0.0);
                //qDebug()<<" Translation: "<<finalPOCx[indexFrame]<<" "<<finalPOCy[indexFrame]<<" "<<maximalAngles[indexFrame];
                if (scaling == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,_tempTranslation,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,maximalAngles[indexFrame]);
                    plne_slicovany_snimek.release();
                    finalni_licovani2.release();
                    posunuty_original.release();
                    posunuty_temp.release();
                }
                else
                {
                    plne_slicovany_snimek.release();
                    posunuty_temp.release();
                }
            }
        }
        else {
            emit x_coordInfo(indexFrame,0,QString::number(999.0));
            emit y_coordInfo(indexFrame,1,QString::number(999.0));
            emit angleInfo(indexFrame,2,QString::number(999.0));
            emit statusInfo(indexFrame,3,QString("error"));
        }
        if (errorOccured){
            emit x_coordInfo(indexFrame,0,QString::number(999.0));
            emit y_coordInfo(indexFrame,1,QString::number(999.0));
            emit angleInfo(indexFrame,2,QString::number(999.0));
            emit statusInfo(indexFrame,3,QString("error"));
        }
        else if (!errorOccured){
            /*qDebug()<<"Sending: frame: "<<indexFrame<<" "<<QString::number(finalPOCx[indexFrame])<<" "<<
                      QString::number(finalPOCy[indexFrame])<<" "<<QString::number(maximalAngles[indexFrame]);*/

            //QTableWidgetItem* newItem = new QTableWidgetItem(QString::number(finalPOCx[indexFrame]));
            //emit x_coordInfo(indexFrame,0,newItem);
            emit x_coordInfo(indexFrame,0,QString::number(finalPOCx[indexFrame]));
            //newItem = new QTableWidgetItem(QString::number(finalPOCy[indexFrame]));
            //emit y_coordInfo(indexFrame,1,newItem);
            emit y_coordInfo(indexFrame,1,QString::number(finalPOCy[indexFrame]));
            //newItem = new QTableWidgetItem(QString::number(maximalAngles[indexFrame]));
            //emit angleInfo(indexFrame,2,newItem);
            emit angleInfo(indexFrame,2,QString::number(maximalAngles[indexFrame]));
            emit statusInfo(indexFrame,3,QString("done"));
        }
        //qDebug()<<"####################################################";
    }
    if (ohodnoceniSnimku[startingFrame] == 0 || ohodnoceniSnimku[startingFrame] == 1 ||
            ohodnoceniSnimku[startingFrame] == 4){
        //qDebug()<<"Recalculating starting frame "<<startingFrame;
        if (registrateTheBest(capture,referencialImage,frangiMaxReversal,startingFrame,iteration,maximalArea,
                               totalAngle,correl_extra,correl_standard,scaling,frangiParameters,
                               finalPOCx,finalPOCy,frangiX,frangiY,frangiEuklidean,maximalAngles)){
            emit x_coordInfo(startingFrame,0,QString::number(finalPOCx[startingFrame]));
            emit y_coordInfo(startingFrame,1,QString::number(finalPOCy[startingFrame]));
            emit angleInfo(startingFrame,2,QString::number(maximalAngles[startingFrame]));
            emit statusInfo(startingFrame,3,QString("done"));
        }
    }
    emit allWorkDone(threadIndex);
}

void RegistrationThread::dataObtained(){
    emit readyForFinish(threadIndex);
}

bool registrateTheBest(cv::VideoCapture& i_cap,
                       cv::Mat& i_referencialFrame,
                       cv::Point3d i_coordsFrangiStandardReferencialReverse,
                       int i_index_translated,
                       int i_iteration,
                       double i_areaMaximum,
                       double i_angle,
                       cv::Rect& i_cutoutExtra,
                       cv::Rect& i_cutoutStandard,
                       bool i_scaleChanged,
                       QVector<double> &parametry_frangi,
                       QVector<double> &_pocX,
                       QVector<double> &_pocY,
                       QVector<double> &_frangiX,
                       QVector<double> &_frangiY,
                       QVector<double> &_frangiEucl,
                       QVector<double> &_maxAngles)
{
    Mat plne_slicovany_snimek = cv::Mat::zeros(cv::Size(i_referencialFrame.cols,i_referencialFrame.rows), CV_32FC3);

    if (!fullRegistration(i_cap,
                          i_referencialFrame,
                          i_index_translated,
                          i_iteration,
                          i_areaMaximum,
                          i_angle,
                          i_cutoutExtra,
                          i_cutoutStandard,
                          i_scaleChanged,
                          plne_slicovany_snimek,
                          _pocX,_pocY,_maxAngles)){
        qWarning()<< "Registration error. Frame "<<i_index_translated<<" was not registrated.";
        _frangiX[i_index_translated]=999.0;
        _frangiY[i_index_translated]=999.0;
        _frangiEucl[i_index_translated]=999.0;
        plne_slicovany_snimek.release();
        return false;
    }
    else
    {
        //qDebug()<<"Full registration correct.";
        Mat refSnimek_vyrez;
        i_referencialFrame(i_cutoutStandard).copyTo(refSnimek_vyrez);
        int rows = i_referencialFrame.rows;
        int cols = i_referencialFrame.cols;
        if (std::abs(_pocX[i_index_translated]) == 999.0)
        {
            qWarning()<< "Frame "<<i_index_translated<<" written without changes.";
            plne_slicovany_snimek.release();
            _frangiX[i_index_translated]=999.0;
            _frangiY[i_index_translated]=999.0;
            _frangiEucl[i_index_translated]=999.0;
            return false;
        }
        else
        {
            Mat posunuty_temp;
            i_cap.set(CV_CAP_PROP_POS_FRAMES,i_index_translated);
            if (i_cap.read(posunuty_temp)!=1)
            {
                qWarning()<<"Frame "<<i_index_translated<<" could not be read!";
            }
            Mat mezivysledek32f,mezivysledek32f_vyrez,posunuty;
            if (i_scaleChanged == true)
            {
                posunuty_temp(i_cutoutExtra).copyTo(posunuty);
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
            }

            Point3d translace_korekce(0.0,0.0,0.0);
            Mat plneSlicovanyKorekce = cv::Mat::zeros(cv::Size(i_referencialFrame.cols,i_referencialFrame.rows), CV_32FC3);
            if (!registrationCorrection(plne_slicovany_snimek,i_referencialFrame,plneSlicovanyKorekce,
                                        i_cutoutStandard,translace_korekce)){
                qWarning()<<"Frame "<<i_index_translated<<" - registration correction failed";
                return false;
            }
            else{
                if (translace_korekce.x > 0.0 || translace_korekce.y > 0.0)
                {
                    //qDebug()<<"Correction: "<<translace_korekce.x<<" "<<translace_korekce.y;
                    _pocX[i_index_translated] += translace_korekce.x;
                    _pocY[i_index_translated] += translace_korekce.y;
                    Point3d pt6 = fk_translace_hann(i_referencialFrame,plneSlicovanyKorekce);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(i_referencialFrame,plneSlicovanyKorekce);
                    }
                    //qDebug()<<"Checking translation after correction: "<<pt6.x<<" "<<pt6.y;
                    _pocX[i_index_translated] += pt6.x;
                    _pocY[i_index_translated] += pt6.y;
                }

                plneSlicovanyKorekce.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(i_cutoutStandard).copyTo(mezivysledek32f_vyrez);
                double R_prvni = vypocet_KK(i_referencialFrame,plneSlicovanyKorekce,i_cutoutStandard);
                Point3d _tempTranslation = Point3d(_pocX[i_index_translated],_pocY[i_index_translated],0.0);

                Point3d frangi_bod_slicovany_reverse = frangi_analysis(plneSlicovanyKorekce,2,2,0,"",2,_tempTranslation,parametry_frangi);

                _frangiX[i_index_translated] = frangi_bod_slicovany_reverse.x;
                _frangiY[i_index_translated] = frangi_bod_slicovany_reverse.y;
                double yydef = i_coordsFrangiStandardReferencialReverse.x - frangi_bod_slicovany_reverse.x;
                double xxdef = i_coordsFrangiStandardReferencialReverse.y - frangi_bod_slicovany_reverse.y;
                double suma_rozdilu = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                _frangiEucl[i_index_translated] = euklid;

                Point3d vysledne_posunuti(0.0,0.0,0.0);
                vysledne_posunuti.y = _pocY[i_index_translated] - yydef;
                vysledne_posunuti.x = _pocX[i_index_translated] - xxdef;
                vysledne_posunuti.z = 0;
                xxdef = 0.0;
                yydef = 0.0;
                suma_rozdilu = 0.0;
                euklid = 0.0;

                Mat posunuty2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
                Mat finalni_licovani = rotace_snimku(posunuty2,_maxAngles[i_index_translated]);
                //Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
                /*finalni_licovani.copyTo(finalni_licovani_32f);
                kontrola_typu_snimku_32C1(finalni_licovani_32f);
                finalni_licovani_32f(i_cutoutStandard).copyTo(finalni_licovani_32f_vyrez);
                finalni_licovani_32f.release();*/
                posunuty2.release();
                double R_druhy = vypocet_KK(i_referencialFrame,finalni_licovani,i_cutoutStandard);
                // finalni_licovani_32f_vyrez.release();
                if (R_prvni >= R_druhy)
                {
                    //qDebug()<< "Frame "<<i_index_translated<<" written after standard registration.";
                    //qDebug()<<"R1: "<<R_prvni<<" R2: "<<R_druhy;
                    //qDebug()<<"Translation: "<<_pocX[i_index_translated]<<" "<<_pocY[i_index_translated]<<" "<<_maxAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,_tempTranslation,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,_maxAngles[i_index_translated]);
                        plne_slicovany_snimek.release();
                        plneSlicovanyKorekce.release();
                        finalni_licovani2.release();
                        finalni_licovani.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        plne_slicovany_snimek.release();
                        finalni_licovani.release();
                    }
                }
                else
                {
                    qDebug()<< "Frame "<<i_index_translated<<" written after vein analysis";
                    qDebug()<<" Translation: "<<vysledne_posunuti.x<<" "<<vysledne_posunuti.y<<" "<<_maxAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,vysledne_posunuti,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,_maxAngles[i_index_translated]);
                        plne_slicovany_snimek.release();
                        plneSlicovanyKorekce.release();
                        finalni_licovani2.release();
                        finalni_licovani.release();
                        posunuty_original.release();
                        posunuty_temp.release();
                    }
                    else
                    {
                        plne_slicovany_snimek.release();
                        finalni_licovani.release();
                    }
                    _pocX[i_index_translated] = vysledne_posunuti.x;
                    _pocY[i_index_translated] = vysledne_posunuti.y;
                }
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                return true;
            }
        }
    }
};

bool fullRegistration(cv::VideoCapture& cap,
                      cv::Mat& referencni_snimek,
                      int cislo_posunuty,
                      int iterace,
                      double oblastMaxima,
                      double uhel,
                      cv::Rect& korelacni_vyrez_navic,
                      cv::Rect& korelacni_vyrez_standardni,
                      bool nutnost_zmenit_velikost_snimku,
                      cv::Mat& slicovany_kompletne,
                      QVector<double> &_pocX,
                      QVector<double> &_pocY,
                      QVector<double> &_maxAngles)
{
    try {
        Mat posunuty_temp;
        cap.set(CV_CAP_PROP_POS_FRAMES,cislo_posunuty);
        double celkovy_uhel = 0.0;
        if(!cap.read(posunuty_temp))
            return false;

        kontrola_typu_snimku_8C3(referencni_snimek);
        kontrola_typu_snimku_8C3(posunuty_temp);
        int rows = referencni_snimek.rows;
        int cols = referencni_snimek.cols;
        Mat hann;
        createHanningWindow(hann, referencni_snimek.size(), CV_32FC1);
        Mat referencni_snimek_32f,referencni_snimek_vyrez;
        referencni_snimek.copyTo(referencni_snimek_32f);
        kontrola_typu_snimku_32C1(referencni_snimek_32f);
        referencni_snimek_32f(korelacni_vyrez_standardni).copyTo(referencni_snimek_vyrez);
        Mat posunuty, posunuty_vyrez;
        if (nutnost_zmenit_velikost_snimku == true)
        {
            posunuty_temp(korelacni_vyrez_navic).copyTo(posunuty);
            posunuty(korelacni_vyrez_standardni).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        else
        {
            posunuty_temp.copyTo(posunuty);
            posunuty(korelacni_vyrez_standardni).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        Mat posunuty_32f;
        posunuty.copyTo(posunuty_32f);
        kontrola_typu_snimku_32C1(posunuty_32f);

        Mat slicovany1;
        Point3d pt1(0.0,0.0,0.0);
        if (nutnost_zmenit_velikost_snimku == true)
        {
            pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = fk_translace(referencni_snimek_32f,posunuty_32f);

            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = fk_translace(referencni_snimek_vyrez,posunuty_vyrez);
        }
        if (nutnost_zmenit_velikost_snimku == false)
        {
            pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        }

        if (pt1.x>=55 || pt1.y>=55)
        {
            _pocX[cislo_posunuty] = 999.0;
            _pocY[cislo_posunuty] = 999.0;
            _maxAngles[cislo_posunuty] = 999.0;
            return false;
        }
        else
        {
            //qDebug()<<"Filling pt1";
            _pocX[cislo_posunuty] = pt1.x;
            _pocY[cislo_posunuty] = pt1.y;
            //qDebug()<<"pt1 filled.";
            //if (cislo_posunuty == 0)
                //qDebug()<<"PT1: "<<pt1.x<<" "<<pt1.y;
            slicovany1 = translace_snimku(posunuty,pt1,rows,cols);
            cv::Mat slicovany1_32f_rotace,slicovany1_32f,slicovany1_vyrez;
            slicovany1.copyTo(slicovany1_32f);
            kontrola_typu_snimku_32C1(slicovany1_32f);
            Point3d vysledek_rotace = fk_rotace(referencni_snimek_32f,slicovany1_32f,uhel,pt1.z,pt1);
            if (std::abs(vysledek_rotace.y) > uhel)
                vysledek_rotace.y=0;

            _maxAngles[cislo_posunuty] = vysledek_rotace.y;
            slicovany1_32f_rotace = rotace_snimku(slicovany1_32f,vysledek_rotace.y);
            slicovany1_32f_rotace(korelacni_vyrez_standardni).copyTo(slicovany1_vyrez);

            Point3d pt2(0.0,0.0,0.0);
            pt2 = fk_translace(referencni_snimek_vyrez,slicovany1_vyrez);
            if (pt2.x >= 55 || pt2.y >= 55)
            {
                _pocX[cislo_posunuty] = 999.0;
                _pocY[cislo_posunuty] = 999.0;
                _maxAngles[cislo_posunuty] = 999.0;
                slicovany1.copyTo(slicovany_kompletne);
                slicovany1.release();
                return false;
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                //qDebug()<<"FWHM for "<<cislo_posunuty<<" = "<<FWHM;
                slicovany1.release();
                slicovany1_32f.release();
                slicovany1_vyrez.release();
                //if (cislo_posunuty == 0)
                    //qDebug()<<"PT2: "<<pt2.x<<" "<<pt2.y;

                Point3d pt3(0.0,0.0,0.0);
                pt3.x = pt1.x+pt2.x;
                pt3.y = pt1.y+pt2.y;
                pt3.z = pt2.z;
                _pocX[cislo_posunuty] = pt3.x;
                _pocY[cislo_posunuty] = pt3.y;
                Mat slicovany2 = translace_snimku(posunuty,pt3,rows,cols);
                Mat slicovany2_32f,slicovany2_vyrez;
                slicovany2.copyTo(slicovany2_32f);
                kontrola_typu_snimku_32C1(slicovany2_32f);
                Mat slicovany2_rotace = rotace_snimku(slicovany2_32f,vysledek_rotace.y);
                slicovany2_rotace(korelacni_vyrez_standardni).copyTo(slicovany2_vyrez);
                Mat mezivysledek_vyrez,mezivysledek;
                slicovany2_rotace.copyTo(mezivysledek);
                slicovany2_vyrez.copyTo(mezivysledek_vyrez);
                //slicovany2.release();
                slicovany2_vyrez.release();
                slicovany2_32f.release();
                slicovany2_rotace.release();
                celkovy_uhel+=vysledek_rotace.y;
                vysledek_rotace.y = 0;
                int max_pocet_iteraci = 0;
                if (iterace == -1)
                {
                    if (FWHM <= 20){max_pocet_iteraci = 2;}
                    else if (FWHM > 20 && FWHM <= 30){max_pocet_iteraci = 4;}
                    else if (FWHM > 30 && FWHM <= 35){max_pocet_iteraci = 6;}
                    else if (FWHM > 35 && FWHM <= 40){max_pocet_iteraci = 8;}
                    else if (FWHM > 40 && FWHM <= 45){max_pocet_iteraci = 10;}
                    else if (FWHM > 45){max_pocet_iteraci = 5;};
                }
                if (iterace >= 1)
                {
                    max_pocet_iteraci = iterace;
                }
                for (int i = 0; i < max_pocet_iteraci; i++)
                {
                    Point3d rotace_ForLoop(0.0,0.0,0.0);
                    rotace_ForLoop = fk_rotace(referencni_snimek,mezivysledek,uhel,pt3.z,pt3);
                    if (std::abs(rotace_ForLoop.y) > uhel)
                        rotace_ForLoop.y = 0.0;
                    else if (std::abs(celkovy_uhel+rotace_ForLoop.y)>uhel)
                        rotace_ForLoop.y=0.0;
                    else
                        celkovy_uhel+=rotace_ForLoop.y;

                    Mat rotovany;
                    if (rotace_ForLoop.y != 0.0)
                        rotovany = rotace_snimku(mezivysledek,rotace_ForLoop.y);
                    else
                        rotovany = mezivysledek;

                    rotace_ForLoop.y = 0.0;
                    Mat rotovany_vyrez;
                    rotovany(korelacni_vyrez_standardni).copyTo(rotovany_vyrez);
                    rotovany.release();
                    Point3d pt4(0.0,0.0,0.0);
                    pt4 = fk_translace(referencni_snimek_vyrez,rotovany_vyrez);
                    rotovany_vyrez.release();
                    if (pt4.x >= 55 || pt4.y >= 55)
                    {
                        slicovany2.copyTo(slicovany_kompletne);
                        //qDebug()<<"Frame "<<cislo_posunuty<<" terminated because \"condition 55\" reached.";
                        break;
                    }
                    else
                    {
                        pt3.x += pt4.x;
                        pt3.y += pt4.y;
                        pt3.z = pt4.z;
                        //if (cislo_posunuty == 0)
                            //qDebug()<<"PT3 loop: "<<pt3.x<<" "<<pt3.y;
                        _pocX[cislo_posunuty] = pt3.x;
                        _pocY[cislo_posunuty] = pt3.y;
                        _maxAngles[cislo_posunuty] = celkovy_uhel;
                        Mat posunuty_temp = translace_snimku(posunuty,pt3,rows,cols);
                        Mat rotovany_temp = rotace_snimku(posunuty_temp,celkovy_uhel);
                        posunuty_temp.release();
                        rotovany_temp.copyTo(mezivysledek);
                        kontrola_typu_snimku_32C1(rotovany_temp);
                        rotovany_temp.release();
                    }
                }
                mezivysledek.copyTo(slicovany_kompletne);
                return true;
            }
        }
    } catch (std::exception &e) {
        qWarning()<<"Full registration error: "<<e.what();
        _pocX[cislo_posunuty] = 999.0;
        _pocY[cislo_posunuty] = 999.0;
        _maxAngles[cislo_posunuty] = 999.0;
        return false;
    }
};

bool imagePreprocessing(cv::Mat &reference,
                        cv::Mat &obraz,
                        QVector<double> &parFrang,
                        cv::Point2f &hraniceAnomalie,
                        cv::Point2f &hraniceCasu,
                        cv::Rect &oblastAnomalie,
                        cv::Rect &vyrezKoreEx,
                        cv::Rect &vyrezKoreStand,
                        cv::VideoCapture &cap,
                        bool &zmeMer)
{
    try {
        cv::Point3d pt_temp(0.0,0.0,0.0);
        cv::Point3d frangi_bod(0.0,0.0,0.0);
        double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        bool pritomnostAnomalie = false;
        bool pritomnostCasZn = false;
        if (hraniceAnomalie.x != 0.0f) // světelná anomálie
        {
            if (hraniceAnomalie.x < float(sirka_framu/2))
            {
                oblastAnomalie.x = 0;
                oblastAnomalie.y = int(hraniceAnomalie.x);
                oblastAnomalie.width = int(sirka_framu-int(hraniceAnomalie.x)-1);
                oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            if (hraniceAnomalie.x > float(sirka_framu/2))
            {
                oblastAnomalie.x = 0;
                oblastAnomalie.y = 0;
                oblastAnomalie.width = int(hraniceAnomalie.x);
                oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            pritomnostAnomalie = true;
        }
        if (hraniceCasu.y != 0.0f) // časová anomálie
        {
            if (hraniceCasu.x < float(vyska_framu/2))
            {
                oblastAnomalie.x = int(hraniceCasu.y);
                oblastAnomalie.y = 0;
                if (pritomnostAnomalie != true)
                    oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
                oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
            }
            if (hraniceCasu.x > float(sirka_framu/2))
            {
                oblastAnomalie.x = 0;
                oblastAnomalie.y = 0;
                if (pritomnostAnomalie != true)
                    oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
                oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
            }
            pritomnostCasZn = true;
        }
        if (pritomnostAnomalie == true || pritomnostCasZn == true)
            frangi_bod = frangi_analysis(reference(oblastAnomalie),1,1,0,"",1,pt_temp,parFrang);
        else
            frangi_bod = frangi_analysis(reference,1,1,0,"",1,pt_temp,parFrang);

        if (frangi_bod.z == 0.0)
        {
            return false;
        }
        else
        {
            bool nutnost_zmenit_velikost = false;
            int rows = reference.rows;
            int cols = reference.cols;
            int radek_od = int(round(frangi_bod.y-0.8*frangi_bod.y));
            int radek_do = int(round(frangi_bod.y+0.8*(rows - frangi_bod.y)));
            int sloupec_od = 0;
            int sloupec_do = 0;

            if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f && int(hraniceAnomalie.y)<(cols/2))
            {
                sloupec_od = int(hraniceAnomalie.y);
                nutnost_zmenit_velikost = true;
            }
            else
                sloupec_od = int(round(frangi_bod.x-0.8*(frangi_bod.x)));

            if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f &&  int(hraniceAnomalie.y)>(cols/2))
            {
                sloupec_do = int(hraniceAnomalie.y);
                nutnost_zmenit_velikost = true;
            }
            else
                sloupec_do = int(round(frangi_bod.x+0.8*(cols - frangi_bod.x)));

            int vyrez_sirka = sloupec_do-sloupec_od;
            int vyrez_vyska = radek_do - radek_od;

            if ((vyrez_vyska>480 || vyrez_sirka>640)|| nutnost_zmenit_velikost == true)
            {
                vyrezKoreEx.x = sloupec_od;
                vyrezKoreEx.y = radek_od;
                vyrezKoreEx.width = vyrez_sirka;
                vyrezKoreEx.height = vyrez_vyska;

                reference(vyrezKoreEx).copyTo(obraz);

                frangi_bod = frangi_analysis(obraz,1,1,0,"",1,pt_temp,parFrang);
                rows = obraz.rows;
                cols = obraz.cols;
                radek_od = int(round(frangi_bod.y-0.9*frangi_bod.y));
                radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
                sloupec_od = int(round(frangi_bod.x-0.9*(frangi_bod.x)));
                sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));
                vyrez_sirka = sloupec_do-sloupec_od;
                vyrez_vyska = radek_do - radek_od;
                vyrezKoreStand.x = sloupec_od;
                vyrezKoreStand.y = radek_od;
                vyrezKoreStand.width = vyrez_sirka;
                vyrezKoreStand.height = vyrez_vyska;
                zmeMer = true;
            }
            else
            {
                vyrezKoreStand.x = int(round(frangi_bod.x-0.9*(frangi_bod.x)));
                vyrezKoreStand.y = int(round(frangi_bod.y-0.9*frangi_bod.y));
                radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
                sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));
                vyrezKoreStand.width = sloupec_do-vyrezKoreStand.x;
                vyrezKoreStand.height = radek_do - vyrezKoreStand.y;

                reference.copyTo(obraz);
            }
            return true;
        }
    } catch (std::exception &e) {
        qWarning()<<"Image preprocessing error: "<<e.what();
        return false;
    }
};

bool registrationCorrection(cv::Mat& slicovany_snimek,
                            cv::Mat& obraz,
                            cv::Mat &snimek_korigovany,
                            cv::Rect& vyrez_korelace_standard,
                            cv::Point3d& korekce_bod)
{
    try {
        Mat mezivysledek,mezivysledek32f,mezivysledek32f_vyrez,obraz_vyrez;
        slicovany_snimek.copyTo(mezivysledek);
        int rows = slicovany_snimek.rows;
        int cols = slicovany_snimek.cols;
        //mezivysledek.copyTo(mezivysledek32f);
        //kontrola_typu_snimku_32C1(mezivysledek32f);
        //mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
        obraz(vyrez_korelace_standard).copyTo(obraz_vyrez);

        double R1 = vypocet_KK(obraz,mezivysledek,vyrez_korelace_standard);

        //mezivysledek32f.release();
        //mezivysledek32f_vyrez.release();

        Point3d mira_korekcniho_posunuti(0.0,0.0,0.0);
        mira_korekcniho_posunuti = fk_translace_hann(obraz,mezivysledek);
        if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
        {
            mira_korekcniho_posunuti = fk_translace(obraz,mezivysledek);
        }
        if (std::abs(mira_korekcniho_posunuti.x) > 290 || std::abs(mira_korekcniho_posunuti.y) > 290)
        {
            mira_korekcniho_posunuti = fk_translace(obraz_vyrez,mezivysledek32f_vyrez);
        }
        snimek_korigovany = translace_snimku(mezivysledek,mira_korekcniho_posunuti,rows,cols);
        double R2 = vypocet_KK(obraz,snimek_korigovany,vyrez_korelace_standard);
        if ((R2 > R1) && ((std::abs(mira_korekcniho_posunuti.x) > 0.3) || (std::abs(mira_korekcniho_posunuti.y) > 0.3)))
        {
            korekce_bod = mira_korekcniho_posunuti;
            return true;
        }
        else
            return true;

    } catch (std::exception &e) {
        qWarning()<<"Registration correction error: "<<e.what();
        return false;
    }
}
