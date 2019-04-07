#include "multiPOC_Ai1.h"
#include "analyza_obrazu/upravy_obrazu.h"
#include "licovani/fazova_korelace_funkce.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include "analyza_obrazu/pouzij_frangiho.h"

using cv::Mat;
using cv::Rect;
using cv::Point3d;
using std::cout;
using std::endl;
bool completeRegistration(cv::VideoCapture& cap,
                          cv::Mat& i_referencial,
                          int i_translatedNo,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angleLimit,
                          cv::Rect &i_cutoutExtra,
                          cv::Rect &i_cutoutStandard,
                          bool i_scaleChanged,
                          cv::Mat& i_completelyRegistrated,
                          QVector<double> &i_pocX,
                          QVector<double> &i_pocY,
                          QVector<double> &i_maxAngles)
{
    try {
        i_pocX.push_back(0.0);
        i_pocY.push_back(0.0);
        i_maxAngles.push_back(0.0);

        Mat posunuty_temp;
        cap.set(CV_CAP_PROP_POS_FRAMES,i_translatedNo);
        double celkovy_uhel = 0.0;
        if(!cap.read(posunuty_temp))
            return false;

        kontrola_typu_snimku_8C3(i_referencial);
        kontrola_typu_snimku_8C3(posunuty_temp);
        int rows = i_referencial.rows;
        int cols = i_referencial.cols;
        Mat hann;
        createHanningWindow(hann, i_referencial.size(), CV_32FC1);
        Mat referencni_snimek_32f,referencni_snimek_vyrez;
        i_referencial.copyTo(referencni_snimek_32f);
        kontrola_typu_snimku_32C1(referencni_snimek_32f);
        referencni_snimek_32f(i_cutoutStandard).copyTo(referencni_snimek_vyrez);
        Mat posunuty, posunuty_vyrez;
        if (i_scaleChanged == true)
        {
            posunuty_temp(i_cutoutExtra).copyTo(posunuty);
            posunuty(i_cutoutStandard).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        else
        {
            posunuty_temp.copyTo(posunuty);
            posunuty(i_cutoutStandard).copyTo(posunuty_vyrez);
            posunuty_temp.release();
        }
        Mat posunuty_32f;
        posunuty.copyTo(posunuty_32f);
        kontrola_typu_snimku_32C1(posunuty_32f);

        Mat slicovany1;
        Point3d pt1(0.0,0.0,0.0);
        if (i_scaleChanged == true)
        {
            pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = fk_translace(referencni_snimek_32f,posunuty_32f);

            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = fk_translace(referencni_snimek_vyrez,posunuty_vyrez);
        }
        if (i_scaleChanged == false)
        {
            pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        }

        if (pt1.x>=55 || pt1.y>=55)
        {
            i_pocX[0] = 999.0;
            i_pocY[0] = 999.0;
            i_maxAngles[0] = 999.0;
            return false;
        }
        else
        {
            qDebug()<<"Filling pt1";
            i_pocX[0] = pt1.x;
            i_pocY[0] = pt1.y;
            qDebug()<<"pt1 filled.";
            if (i_translatedNo == 0)
                qDebug()<<"PT1: "<<pt1.x<<" "<<pt1.y;
            slicovany1 = translace_snimku(posunuty,pt1,rows,cols);
            cv::Mat slicovany1_32f_rotace,slicovany1_32f,slicovany1_vyrez;
            slicovany1.copyTo(slicovany1_32f);
            kontrola_typu_snimku_32C1(slicovany1_32f);
            Point3d vysledek_rotace = fk_rotace(referencni_snimek_32f,slicovany1_32f,i_angleLimit,pt1.z,pt1);
            if (std::abs(vysledek_rotace.y) > i_angleLimit)
                vysledek_rotace.y=0;

            i_maxAngles[0] = vysledek_rotace.y;
            slicovany1_32f_rotace = rotace_snimku(slicovany1_32f,vysledek_rotace.y);
            slicovany1_32f_rotace(i_cutoutStandard).copyTo(slicovany1_vyrez);

            Point3d pt2(0.0,0.0,0.0);
            pt2 = fk_translace(referencni_snimek_vyrez,slicovany1_vyrez);
            if (pt2.x >= 55 || pt2.y >= 55)
            {
                i_pocX[0] = 999.0;
                i_pocY[0] = 999.0;
                i_maxAngles[0] = 999.0;
                slicovany1.copyTo(i_completelyRegistrated);
                slicovany1.release();
                return false;
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                qDebug()<<"FWHM for "<<i_translatedNo<<" = "<<FWHM;
                slicovany1.release();
                slicovany1_32f.release();
                slicovany1_vyrez.release();
                if (i_translatedNo == 0)
                    qDebug()<<"PT2: "<<pt2.x<<" "<<pt2.y;

                Point3d pt3(0.0,0.0,0.0);
                pt3.x = pt1.x+pt2.x;
                pt3.y = pt1.y+pt2.y;
                pt3.z = pt2.z;
                i_pocX[0] = pt3.x;
                i_pocY[0] = pt3.y;
                Mat slicovany2 = translace_snimku(posunuty,pt3,rows,cols);
                Mat slicovany2_32f,slicovany2_vyrez;
                slicovany2.copyTo(slicovany2_32f);
                kontrola_typu_snimku_32C1(slicovany2_32f);
                Mat slicovany2_rotace = rotace_snimku(slicovany2_32f,vysledek_rotace.y);
                slicovany2_rotace(i_cutoutStandard).copyTo(slicovany2_vyrez);
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
                if (i_iteration == -1.0)
                {
                    if (FWHM <= 20){max_pocet_iteraci = 2;}
                    else if (FWHM > 20 && FWHM <= 30){max_pocet_iteraci = 4;}
                    else if (FWHM > 30 && FWHM <= 35){max_pocet_iteraci = 6;}
                    else if (FWHM > 35 && FWHM <= 40){max_pocet_iteraci = 8;}
                    else if (FWHM > 40 && FWHM <= 45){max_pocet_iteraci = 10;}
                    else if (FWHM > 45){max_pocet_iteraci = 5;};
                }
                if (i_iteration >= 1)
                {
                    max_pocet_iteraci = int(i_iteration);
                }
                for (int i = 0; i < max_pocet_iteraci; i++)
                {
                    Point3d rotace_ForLoop(0.0,0.0,0.0);
                    rotace_ForLoop = fk_rotace(i_referencial,mezivysledek,i_angleLimit,pt3.z,pt3);
                    if (std::abs(rotace_ForLoop.y) > i_angleLimit)
                        rotace_ForLoop.y = 0.0;
                    else if (std::abs(celkovy_uhel+rotace_ForLoop.y)>i_angleLimit)
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
                    rotovany(i_cutoutStandard).copyTo(rotovany_vyrez);
                    rotovany.release();
                    Point3d pt4(0.0,0.0,0.0);
                    pt4 = fk_translace(referencni_snimek_vyrez,rotovany_vyrez);
                    rotovany_vyrez.release();
                    if (pt4.x >= 55 || pt4.y >= 55)
                    {
                        slicovany2.copyTo(i_completelyRegistrated);
                        qDebug()<<"Frame "<<i_translatedNo<<" terminated because \"condition 55\" reached.";
                        break;
                    }
                    else
                    {
                        pt3.x += pt4.x;
                        pt3.y += pt4.y;
                        pt3.z = pt4.z;
                        if (i_translatedNo == 0)
                            qDebug()<<"PT3 loop: "<<pt3.x<<" "<<pt3.y;
                        i_pocX[0] = pt3.x;
                        i_pocY[0] = pt3.y;
                        i_maxAngles[0] = celkovy_uhel;
                        Mat posunuty_temp = translace_snimku(posunuty,pt3,rows,cols);
                        Mat rotovany_temp = rotace_snimku(posunuty_temp,celkovy_uhel);
                        posunuty_temp.release();
                        rotovany_temp.copyTo(mezivysledek);
                        kontrola_typu_snimku_32C1(rotovany_temp);
                        rotovany_temp.release();
                    }
                }
                mezivysledek.copyTo(i_completelyRegistrated);
                return true;
            }
        }
    } catch (std::exception &e) {
        qWarning()<<"Full registration error: "<<e.what();
        i_pocX[0] = 999.0;
        i_pocY[0] = 999.0;
        i_maxAngles[0] = 999.0;
        return false;
    }
}

bool preprocessingCompleteRegistration(cv::Mat &reference,
                                       cv::Mat &obraz,
                                       QVector<double> parFrang,
                                       cv::Point2d &hraniceAnomalie,
                                       cv::Point2d &hraniceCasu,
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
        if (hraniceAnomalie.x != 0.0) // světelná anomálie
        {
            if (hraniceAnomalie.x < (sirka_framu/2))
            {
                oblastAnomalie.x = 0;
                oblastAnomalie.y = int(hraniceAnomalie.x);
                oblastAnomalie.width = int(sirka_framu-int(hraniceAnomalie.x)-1);
                oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            if (hraniceAnomalie.x > (sirka_framu/2))
            {
                oblastAnomalie.x = 0;
                oblastAnomalie.y = 0;
                oblastAnomalie.width = int(hraniceAnomalie.x);
                oblastAnomalie.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            pritomnostAnomalie = true;
        }
        if (hraniceCasu.y != 0.0) // časová anomálie
        {
            if (hraniceCasu.x < (vyska_framu/2))
            {
                oblastAnomalie.x = int(hraniceCasu.y);
                oblastAnomalie.y = 0;
                if (pritomnostAnomalie != true)
                    oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
                oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
            }
            if (hraniceCasu.x > (sirka_framu/2))
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

            if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0 && int(hraniceAnomalie.y)<(cols/2))
            {
                sloupec_od = int(hraniceAnomalie.y);
                nutnost_zmenit_velikost = true;
            }
            else
                sloupec_od = int(round(frangi_bod.x-0.8*(frangi_bod.x)));

            if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0 &&  int(hraniceAnomalie.y)>(cols/2))
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
}

