#include "licovani/korekce_zapis.h"
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

using cv::Mat;
using cv::VideoCapture;
using cv::VideoWriter;
using cv::Rect;
using cv::Point3d;
using cv::Size;
using std::cerr;
using std::cout;
using std::endl;
bool licovani_nejvhodnejsich_snimku(cv::VideoCapture& i_cap,
                                    cv::Mat& i_referencialFrame,
                                    cv::Point3d i_coordsFrangiStandardReferencialReverse,
                                    int i_index_translated,
                                    int i_iteration,
                                    double i_areaMaximum,
                                    double i_angle,
                                    cv::Rect& i_cutoutExtra,
                                    cv::Rect& i_cutoutStandard,
                                    bool i_scaleChanged,
                                    QVector<double> parametry_frangi,
                                    QVector<double> &_pocX,
                                    QVector<double> &_pocY,
                                    QVector<double> &_frangiX,
                                    QVector<double> &_frangiY,
                                    QVector<double> &_frangiEucl,
                                    QVector<double> &_maxAngles)
{
    Mat plne_slicovany_snimek = cv::Mat::zeros(cv::Size(i_referencialFrame.cols,i_referencialFrame.rows), CV_32FC3);
    QVector<double> _tempPOCX = _pocX;
    QVector<double> _tempPOCY = _pocY;
    QVector<double> _tempAngles = _maxAngles;
    if (!completeRegistration(i_cap,
                              i_referencialFrame,
                              i_index_translated,
                              i_iteration,
                              i_areaMaximum,
                              i_angle,
                              i_cutoutExtra,
                              i_cutoutStandard,
                              i_scaleChanged,
                              plne_slicovany_snimek,
                              _tempPOCX,_tempPOCY,_tempAngles)){
        qWarning()<< "Registration error. Frame "<<i_index_translated<<" was not registrated.";
        _frangiX[i_index_translated]=999.0;
        _frangiY[i_index_translated]=999.0;
        _frangiEucl[i_index_translated]=999.0;
        plne_slicovany_snimek.release();
        return false;
    }
    else
    {
        qDebug()<<"Full registration correct.";
        Mat refSnimek_vyrez;
        i_referencialFrame(i_cutoutStandard).copyTo(refSnimek_vyrez);
        int rows = i_referencialFrame.rows;
        int cols = i_referencialFrame.cols;
        if (std::abs(_tempPOCX[i_index_translated]) == 999.0)
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
            if (!eventualni_korekce_translace(plne_slicovany_snimek,i_referencialFrame,plneSlicovanyKorekce,
                                              i_cutoutStandard,translace_korekce)){
                qWarning()<<"Frame "<<i_index_translated<<" - registration correction failed";
                return false;
            }
            else{
                if (translace_korekce.x > 0.0 || translace_korekce.y > 0.0)
                {
                    qDebug()<<"Correction: "<<translace_korekce.x<<" "<<translace_korekce.y;
                    _tempPOCX[i_index_translated] += translace_korekce.x;
                    _tempPOCY[i_index_translated] += translace_korekce.y;
                    Point3d pt6 = fk_translace_hann(i_referencialFrame,plneSlicovanyKorekce);
                    if (std::abs(pt6.x)>=290 || std::abs(pt6.y)>=290)
                    {
                        pt6 = fk_translace(i_referencialFrame,plneSlicovanyKorekce);
                    }
                    qDebug()<<"Checking translation after correction: "<<pt6.x<<" "<<pt6.y;
                    _tempPOCX[i_index_translated] += pt6.x;
                    _tempPOCY[i_index_translated] += pt6.y;
                }

                plneSlicovanyKorekce.copyTo(mezivysledek32f);
                kontrola_typu_snimku_32C1(mezivysledek32f);
                mezivysledek32f(i_cutoutStandard).copyTo(mezivysledek32f_vyrez);
                double R_prvni = vypocet_KK(i_referencialFrame,plneSlicovanyKorekce,i_cutoutStandard);
                Point3d _tempTranslation = Point3d(_tempPOCX[i_index_translated],_tempPOCY[i_index_translated],0.0);

                Point3d frangi_bod_slicovany_reverse = frangi_analysis(plneSlicovanyKorekce,2,2,0,"",2,_tempTranslation,parametry_frangi);

                _frangiX[i_index_translated] = frangi_bod_slicovany_reverse.x;
                _frangiY[i_index_translated] = frangi_bod_slicovany_reverse.y;
                double yydef = i_coordsFrangiStandardReferencialReverse.x - frangi_bod_slicovany_reverse.x;
                double xxdef = i_coordsFrangiStandardReferencialReverse.y - frangi_bod_slicovany_reverse.y;
                double suma_rozdilu = std::pow(xxdef,2.0) + std::pow(yydef,2.0);
                double euklid = std::sqrt(suma_rozdilu);
                _frangiEucl[i_index_translated] = euklid;

                Point3d vysledne_posunuti(0.0,0.0,0.0);
                vysledne_posunuti.y = _tempPOCY[i_index_translated] - yydef;
                vysledne_posunuti.x = _tempPOCX[i_index_translated] - xxdef;
                vysledne_posunuti.z = 0;
                xxdef = 0.0;
                yydef = 0.0;
                suma_rozdilu = 0.0;
                euklid = 0.0;

                Mat posunuty2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
                Mat finalni_licovani = rotace_snimku(posunuty2,_tempAngles[i_index_translated]);
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
                    qDebug()<< "Frame "<<i_index_translated<<" written after standard registration.";
                    qDebug()<<"R1: "<<R_prvni<<" R2: "<<R_druhy;
                    qDebug()<<"Translation: "<<_tempPOCX[i_index_translated]<<" "<<_tempPOCY[i_index_translated]<<" "<<_maxAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,_tempTranslation,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,_tempAngles[i_index_translated]);
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
                    _pocX = _tempPOCX;
                    _pocY = _tempPOCY;
                    _maxAngles = _tempAngles;
                }
                else
                {
                    qDebug()<< "Frame "<<i_index_translated<<" written after vein analysis";
                    qDebug()<<" Translation: "<<vysledne_posunuti.x<<" "<<vysledne_posunuti.y<<" "<<_tempAngles[i_index_translated];
                    if (i_scaleChanged == true)
                    {
                        int radky = posunuty_temp.rows;
                        int sloupce = posunuty_temp.cols;
                        Mat posunuty_original = translace_snimku(posunuty_temp,vysledne_posunuti,radky,sloupce);
                        Mat finalni_licovani2 = rotace_snimku(posunuty_original,_tempAngles[i_index_translated]);
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
                    _tempPOCX[i_index_translated] = vysledne_posunuti.x;
                    _tempPOCY[i_index_translated] = vysledne_posunuti.y;
                    _pocX = _tempPOCX;
                    _pocY = _tempPOCY;
                    _maxAngles = _tempAngles;
                }
                mezivysledek32f.release();
                mezivysledek32f_vyrez.release();
                return true;
            }
        }
    }
}

bool eventualni_korekce_translace(cv::Mat& slicovany_snimek, cv::Mat& obraz, cv::Mat &snimek_korigovany,
                                  cv::Rect &vyrez_korelace_standard,
                                     cv::Point3d &korekce_bod)
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
