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
#include "analyza_obrazu/pouzij_frangiho.h"
using cv::Mat;
using cv::VideoCapture;
using cv::Point3d;
using cv::Rect;

RegistrationThread::RegistrationThread(cv::VideoCapture& cap,
                                       QVector<double> frangiParam,
                                       QVector<int> frameEvaluation,
                                       cv::Mat& referencni_snimek,
                                       int startFrame,
                                       int stopFrame,
                                       int iterace,
                                       double oblastMaxima,
                                       double uhel,
                                       float timeStamp,
                                       float lightAnomaly,
                                       bool nutnost_zmenit_velikost_snimku)
{
    capture = cap;
    referencialImage = referencni_snimek;
    iteration = iterace;
    maximalArea = oblastMaxima;
    angle = uhel;
    casovaZnacka = timeStamp;
    svetelAnomalie = lightAnomaly;
    scaling = nutnost_zmenit_velikost_snimku;
    startingFrame = startFrame;
    stoppingFrame = stopFrame;
    frangiParameters = frangiParam;
    ohodnoceniSnimku = frameEvaluation;
}

void RegistrationThread::run()
{
    /// define helpers for frame preprocessing
    bool lightAnomalyPresent = false;
    bool timeStampPresent = false;
    bool scaling=false;

    /// frame and video properties
    double sirka = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    double vyska = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
    double frameCount = capture.get(CV_CAP_PROP_FRAME_COUNT);

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
    Point3d frangiMaxDetected(0.0,0.0,0.0);
    Point3d pt_temp(0.0,0.0,0.0);
    Mat obraz,obraz_vyrez;
    imagePreprocessing(referencialImage,
                       obraz,
                       frangiParameters,
                       frangiMaxDetected,
                       ziskane_hranice_anomalie,
                       ziskane_hranice_CasZnac,
                       anomalyCutoff,
                       correl_extra,
                       correl_standard,
                       capture,
                       lightAnomalyPresent,
                       timeStampPresent,
                       scaling);
    obraz(correl_standard).copyTo(obraz_vyrez);
    Point3d frangiMaxReversal = frangi_analyza(obraz,2,2,0,"",1,false,pt_temp,frangiParameters);
    for (int indexFrame = startingFrame; indexFrame <= stoppingFrame; indexFrame++){
        if (ohodnoceniSnimku[indexFrame] == 0){
            registrateTheBest(capture,referencialImage,frangiMaxReversal,indexFrame,iteration,maximalArea,
                              totalAngle,correl_extra,correl_standard,scaling,frangiParameters,);
        }
    }
}

int RegistrationThread::fullRegistration(cv::VideoCapture& cap,
                                         cv::Mat& referencni_snimek,
                                         int cislo_posunuty,
                                         int iterace,
                                         double oblastMaxima,
                                         double uhel,
                                         cv::Rect& korelacni_vyrez_navic,
                                         cv::Rect& korelacni_vyrez_standardni,
                                         bool nutnost_zmenit_velikost_snimku,
                                         cv::Mat& slicovany_kompletne,
                                         cv::Point3d& mira_translace,
                                         double& celkovy_uhel)
{

}

int RegistrationThread::registrateTheBest(cv::VideoCapture& cap,
                                          cv::Mat& referencni_snimek,
                                          cv::Point3d bod_RefS_reverse,
                                          int index_posunuty,
                                          int iterace,
                                          double oblastMaxima,
                                          double uhel,
                                          cv::Rect& vyrez_korelace_extra,
                                          cv::Rect& vyrez_korelace_standard,
                                          bool zmena_meritka,
                                          QVector<double> &parametry_frangi,
                                          QVector<double> &frangi_x,
                                          QVector<double> &frangi_y,
                                          QVector<double> &frangi_euklid,
                                          QVector<double> &POC_x,
                                          QVector<double> &POC_y,
                                          QVector<double> &angleList)
{
    Mat plne_slicovany_snimek = cv::Mat::zeros(cv::Size(referencni_snimek.cols,referencni_snimek.rows), CV_32FC3);
    Point3d mira_translace;
    double celkovy_uhel = 0;
    int uspech_licovani = fullRegistration(cap,
                                           referencni_snimek,
                                           index_posunuty,
                                           iterace,
                                           oblastMaxima,
                                           uhel,
                                           vyrez_korelace_extra,
                                           vyrez_korelace_standard,
                                           zmena_meritka,
                                           plne_slicovany_snimek,
                                           mira_translace,
                                           celkovy_uhel);
    Mat refSnimek_vyrez;
    referencni_snimek(vyrez_korelace_standard).copyTo(refSnimek_vyrez);
    int rows = referencni_snimek.rows;
    int cols = referencni_snimek.cols;
    if (uspech_licovani == 0)
    {
        qWarning()<< "Licovani skoncilo chybou. Snimek "<<index_posunuty<<" se nepodarilo uspesne slicovat";
        Mat posunuty;
        cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
        if (cap.read(posunuty)!=1)
        {
             qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
        }
        else
        {
            qDebug()<< "Snimek "<<index_posunuty<<" zapsan beze zmen. Licovani skoncilo chybou.";
            posunuty.release();
            plne_slicovany_snimek.release();
        }
        POC_x[index_posunuty]=999.0;
        POC_y[index_posunuty]=999.0;
        angleList[index_posunuty]=999.0;
        frangi_x[index_posunuty]=999.0;
        frangi_y[index_posunuty]=999.0;
        frangi_euklid[index_posunuty]=999.0;
    }
    else
    {
        if (std::abs(mira_translace.x) >= 55 || std::abs(mira_translace.y) >= 55)
        {
            Mat posunuty;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty)!=1)
            {
                 qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
            }
            else
            {
                qDebug()<< "Snimek "<<index_posunuty<<" zapsan beze zmen. Hodnota translace prekrocila prah.";
                posunuty.release();
            }
            POC_x[index_posunuty]=999.0;
            POC_y[index_posunuty]=999.0;
            angleList[index_posunuty]=999.0;
            frangi_x[index_posunuty]=999.0;
            frangi_y[index_posunuty]=999.0;
            frangi_euklid[index_posunuty]=999.0;
        }
        else
        {
            Mat posunuty_temp;
            cap.set(CV_CAP_PROP_POS_FRAMES,index_posunuty);
            if (cap.read(posunuty_temp)!=1)
            {
                 qWarning()<<"Snimek "<<index_posunuty<<" nelze zapsat do videa!";
            }
            Mat mezivysledek32f,mezivysledek32f_vyrez,posunuty;
            if (zmena_meritka == true)
            {
                posunuty_temp(vyrez_korelace_extra).copyTo(posunuty);
            }
            else
            {
                posunuty_temp.copyTo(posunuty);
            }
            plne_slicovany_snimek.copyTo(mezivysledek32f);
            kontrola_typu_snimku_32C1(mezivysledek32f);
            mezivysledek32f(vyrez_korelace_standard).copyTo(mezivysledek32f_vyrez);
            double R_prvni = vypocet_KK(referencni_snimek,plne_slicovany_snimek,vyrez_korelace_standard);
            Point3d frangi_bod_slicovany_reverse = frangi_analyza(plne_slicovany_snimek,2,2,0,"",2,false,mira_translace,parametry_frangi);
            double yydef = bod_RefS_reverse.x - frangi_bod_slicovany_reverse.x;
            double xxdef = bod_RefS_reverse.y - frangi_bod_slicovany_reverse.y;
            //cout << yydef << " " << xxdef << endl;
            Point3d vysledne_posunuti;
            vysledne_posunuti.y = mira_translace.y - yydef;
            vysledne_posunuti.x = mira_translace.x - xxdef;
            vysledne_posunuti.z = 0;
            //cout << vysledne_posunuti.y << " " << vysledne_posunuti.x << endl;//pt3.x <<endl;
            Mat posunuty2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
            Mat finalni_licovani = rotace_snimku(posunuty2,celkovy_uhel);
            Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
            finalni_licovani.copyTo(finalni_licovani_32f);
            kontrola_typu_snimku_32C1(finalni_licovani_32f);
            finalni_licovani_32f(vyrez_korelace_standard).copyTo(finalni_licovani_32f_vyrez);
            finalni_licovani_32f.release();
            posunuty2.release();
            double R_druhy = vypocet_KK(referencni_snimek,finalni_licovani,vyrez_korelace_standard);
            finalni_licovani_32f_vyrez.release();
            if (R_prvni >= R_druhy)
            {
                qDebug()<< "Snimek "<<index_posunuty<<" zapsan pouze po standardnim slicovani.";
                qDebug()<<" Mira translace: "<<mira_translace.x<<" "<<mira_translace.y<<" "<<celkovy_uhel;
                if (zmena_meritka == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,mira_translace,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                    plne_slicovany_snimek.release();
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
                qDebug()<< "Snimek "<<index_posunuty<<" zapsan po uprave translace skrze cevy.";
                qDebug()<<" Mira translace: "<<vysledne_posunuti.x<<" "<<vysledne_posunuti.y<<" "<<celkovy_uhel;
                if (zmena_meritka == true)
                {
                    int radky = posunuty_temp.rows;
                    int sloupce = posunuty_temp.cols;
                    Mat posunuty_original = translace_snimku(posunuty_temp,vysledne_posunuti,radky,sloupce);
                    Mat finalni_licovani2 = rotace_snimku(posunuty_original,celkovy_uhel);
                    plne_slicovany_snimek.release();
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
            mezivysledek32f.release();
            mezivysledek32f_vyrez.release();
        }
    }
}
int RegistrationThread::imagePreprocessing(cv::Mat &reference,
                                           cv::Mat &obraz,
                                           QVector<double> &parFrang,
                                           cv::Point3d &frangiMaxEstimated,
                                           cv::Point2f &hraniceAnomalie,
                                           cv::Point2f &hraniceCasu,
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
            frangiMaxEstimated = frangi_bod;
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
            frangiMaxEstimated = frangi_bod;
            //obraz_temp.release();
        }
    }
}

int RegistrationThread::registrationCorrection(cv::Mat& slicovany_snimek,
                                               cv::Mat& obraz,
                                               cv::Rect& vyrez_korelace_standard,
                                               cv::Point3d& korekce_bod,
                                               int index_posunuty,
                                               QVector<double>& finalni_POC_x,
                                               QVector<double>& finalni_POC_y)
{

}
