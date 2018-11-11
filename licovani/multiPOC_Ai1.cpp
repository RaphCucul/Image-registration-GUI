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
int kompletni_slicovani(cv::VideoCapture& cap,
                        cv::Mat& referencni_snimek,
                        int cislo_posunuty,
                        int &iterace,
                        double &oblastMaxima,
                        double &uhel,
                        cv::Rect& korelacni_vyrez_navic,
                        cv::Rect& korelacni_vyrez_standardni,
                        bool nutnost_zmenit_velikost_snimku,
                        cv::Mat& slicovany_kompletne,
                        cv::Point3d& mira_translace,
                        double& celkovy_uhel)
{
    Mat posunuty_temp;//,,posunuty_32f;
    int uspech_licovani;
    cap.set(CV_CAP_PROP_POS_FRAMES,cislo_posunuty);
    if(!cap.read(posunuty_temp))
    {
        return uspech_licovani = 0;
    }
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

    /*******************************************************************************************/
    Mat slicovany1;
    Point3d pt1(0,0,0);
    if (nutnost_zmenit_velikost_snimku == true)
    {
        pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        //qDebug() << "PT1 pri zmene velikosti Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
        {
            pt1 = fk_translace(referencni_snimek_32f,posunuty_32f);
        }
        if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
        {
            pt1 = fk_translace(referencni_snimek_vyrez,posunuty_vyrez);
        }
    }
    if (nutnost_zmenit_velikost_snimku == false)
    {
        pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        //qDebug() << "PT1 bez zmeny velikosti Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
    }

    if (pt1.x>=55 || pt1.y>=55)
    {
        mira_translace = pt1;
        //qDebug() << "PT1 nad 55 - Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        return uspech_licovani = 0;
    }
    else
    {
        //qDebug() << "PT1 v normálu Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        slicovany1 = translace_snimku(posunuty,pt1,rows,cols);
        cv::Mat slicovany1_32f_rotace,slicovany1_32f,slicovany1_vyrez;
        slicovany1.copyTo(slicovany1_32f);
        kontrola_typu_snimku_32C1(slicovany1_32f);
        Point3d vysledek_rotace = fk_rotace(referencni_snimek_32f,slicovany1_32f,uhel,pt1.z,pt1);
        if (std::abs(vysledek_rotace.y) > 0.1){vysledek_rotace.y=0;}
        slicovany1_32f_rotace = rotace_snimku(slicovany1_32f,vysledek_rotace.y);
        slicovany1_32f_rotace(korelacni_vyrez_standardni).copyTo(slicovany1_vyrez);
        /**************************************************************************************/
        Point3d pt2 = fk_translace(referencni_snimek_vyrez,slicovany1_vyrez);
        if (pt2.x >= 55 || pt2.y >= 55)
        {
            mira_translace = pt1;
            slicovany1.copyTo(slicovany_kompletne);
            slicovany1.release();
            return uspech_licovani = 0;
        }
        else
        {
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            //qDebug()<<"FWHM: "<<FWHM;
            slicovany1.release();
            slicovany1_32f.release();
            slicovany1_vyrez.release();
            Point3d pt3;
            pt3.x = pt1.x+pt2.x;
            pt3.y = pt1.y+pt2.y;
            pt3.z = pt2.z;
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
            /*************************************************************/
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
            /*************************************************************/
            for (int i = 0; i < max_pocet_iteraci; i++)
            {
                Point3d rotace_ForLoop = fk_rotace(referencni_snimek,mezivysledek,uhel,pt3.z,pt3);
                if (std::abs(rotace_ForLoop.y) > 0.1){rotace_ForLoop.y = 0;}
                else if (std::abs(celkovy_uhel+rotace_ForLoop.y)>0.1){rotace_ForLoop.y=0;}
                else {celkovy_uhel+=rotace_ForLoop.y;}
                //qDebug() << "rotace " << celkovy_uhel <<" se zmenou " << rotace_ForLoop.y << endl;
                Mat rotovany = rotace_snimku(mezivysledek,rotace_ForLoop.y);
                rotace_ForLoop.y = 0;
                Mat rotovany_vyrez;
                rotovany(korelacni_vyrez_standardni).copyTo(rotovany_vyrez);
                rotovany.release();
                Point3d pt4 = fk_translace(referencni_snimek_vyrez,rotovany_vyrez);
                rotovany_vyrez.release();
                if (pt4.x >= 55 || pt4.y >= 55)
                {
                    mira_translace = pt3;
                    slicovany2.copyTo(slicovany_kompletne);
                    return uspech_licovani = 1;
                }
                else
                {
                    //qDebug() << "Y: "<< (pt4.y) <<" a X: "<<(pt4.x)<<endl;
                    pt3.x += pt4.x;
                    pt3.y += pt4.y;
                    pt3.z = pt4.z;
                    //qDebug() << "Y: "<< (pt3.y) <<" a X: "<<(pt3.x)<<endl<<endl;
                    Mat posunuty_temp = translace_snimku(posunuty,pt3,rows,cols);
                    Mat rotovany_temp = rotace_snimku(posunuty_temp,celkovy_uhel);
                    posunuty_temp.release();
                    rotovany_temp.copyTo(mezivysledek);
                    Mat rotovany_temp32f_vyrez;
                    kontrola_typu_snimku_32C1(rotovany_temp);
                    rotovany_temp(korelacni_vyrez_standardni).copyTo(rotovany_temp32f_vyrez);
                    rotovany_temp32f_vyrez.copyTo(mezivysledek_vyrez);
                    rotovany_temp.release();
                    rotovany_temp32f_vyrez.release();
                }
            }
            mira_translace = pt3;
            mezivysledek.copyTo(slicovany_kompletne);
            return uspech_licovani = 1;
        }
    }
}

void predzpracovaniKompletnihoLicovani(cv::Mat &reference,
                                       cv::Mat &obraz,
                                       QVector<double> &parFrang,
                                       cv::Point2f &hraniceAnomalie,
                                       cv::Point2f &hraniceCasu,
                                       cv::Point3d &fraMax,
                                       cv::Rect &oblastAnomalie,
                                       cv::Rect &vyrezKoreEx,
                                       cv::Rect &vyrezKoreStand,
                                       cv::VideoCapture &cap,
                                       bool pritomnostAnomalie,
                                       bool casZnacka,
                                       bool &zmeMer)
{
    cv::Point3d pt_temp(0.0,0.0,0.0);
    cv::Point3d frangi_bod(0.0,0.0,0.0);
    double sirka_framu = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska_framu = cap.get(CV_CAP_PROP_FRAME_HEIGHT);    

    if (hraniceAnomalie.x != 0.0f && hraniceAnomalie.y != 0.0f) // světelná anomálie
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
    }
    if (hraniceCasu.x != 0.0f && hraniceCasu.y != 0.0f) // časová anomálie
    {
        if (hraniceCasu.x < float(vyska_framu/2))
        {
            oblastAnomalie.x = int(hraniceCasu.y);
            oblastAnomalie.y = 0;
            oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
            oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
        }
        if (hraniceCasu.x > float(sirka_framu/2))
        {
            oblastAnomalie.x = 0;
            oblastAnomalie.y = 0;
            oblastAnomalie.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
            oblastAnomalie.height = int(vyska_framu-int(hraniceCasu.y)-1);
        }
    }



    if (pritomnostAnomalie == true)
        frangi_bod = frangi_analyza(reference(oblastAnomalie),1,1,0,"",1,casZnacka,pt_temp,parFrang);
    else
        frangi_bod = frangi_analyza(reference,1,1,0,"",1,casZnacka,pt_temp,parFrang);

    if (frangi_bod.z == 0.0)
    {
        qDebug()<<"Nalezeni maxima frangiho funkce se nezdarilo, proverte snimek!"<<endl;
    }
    else
    {
        bool nutnost_zmenit_velikost = false;
        int rows = reference.rows;
        int cols = reference.cols;
        int radek_od = int(round(frangi_bod.y-0.9*frangi_bod.y));
        int radek_do = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
        int sloupec_od = 0;
        int sloupec_do = 0;

        if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f && int(hraniceAnomalie.y)<(cols/2))
        {
            sloupec_od = int(hraniceAnomalie.y);
            nutnost_zmenit_velikost = true;
        }
        else
            sloupec_od = int(round(frangi_bod.x-0.9*(frangi_bod.x)));

        if (pritomnostAnomalie == true && hraniceAnomalie.y != 0.0f &&  int(hraniceAnomalie.y)>(cols/2))
        {
            sloupec_do = int(hraniceAnomalie.y);
            nutnost_zmenit_velikost = true;
        }
        else
            sloupec_do = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));

        int vyrez_sirka = sloupec_do-sloupec_od;
        int vyrez_vyska = radek_do - radek_od;

        if ((vyrez_vyska>480 || vyrez_sirka>640)|| nutnost_zmenit_velikost == true)
        {
            vyrezKoreEx.x = sloupec_od;
            vyrezKoreEx.y = radek_od;
            vyrezKoreEx.width = vyrez_sirka;
            vyrezKoreEx.height = vyrez_vyska;
            reference(vyrezKoreEx).copyTo(obraz);
            frangi_bod = frangi_analyza(obraz,1,1,0,"",1,false,pt_temp,parFrang);
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
            fraMax = frangi_bod;
        }
        else
        {
            fraMax=frangi_bod;
            vyrezKoreStand.x = int(round(fraMax.x-0.9*(fraMax.x)));
            vyrezKoreStand.y = int(round(fraMax.y-0.9*fraMax.y));
            radek_do = int(round(fraMax.y+0.9*(rows - fraMax.y)));
            sloupec_do = int(round(fraMax.x+0.9*(cols - fraMax.x)));
            vyrezKoreStand.width = sloupec_do-vyrezKoreStand.x;
            vyrezKoreStand.height = radek_do - vyrezKoreStand.y;
            reference.copyTo(obraz);
            //obraz_temp.release();
        }
    }
}
