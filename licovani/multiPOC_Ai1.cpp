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
                        cv::Mat& referencialImage,
                        int numberTranslated,
                        double iteration,
                        double areaMaximum,
                        double angle,
                        cv::Rect& cutoutExtra,
                        cv::Rect& cutoutStandard,
                        bool scaleChange,
                        cv::Mat& fullyRegistratedImage,
                        cv::Point3d& definitiveTranslation,
                        double& finalRotationAngle)
{
    Mat posunuty_temp;//,,posunuty_32f;
    bool registrationSuccessfull;
    cap.set(CV_CAP_PROP_POS_FRAMES,numberTranslated);
    if(!cap.read(posunuty_temp))
    {
        return registrationSuccessfull = 0;
    }
    kontrola_typu_snimku_8C3(referencialImage);
    kontrola_typu_snimku_8C3(posunuty_temp);
    int rows = referencialImage.rows;
    int cols = referencialImage.cols;
    Mat hann;
    createHanningWindow(hann, referencialImage.size(), CV_32FC1);
    Mat referencni_snimek_32f,referencni_snimek_vyrez;
    referencialImage.copyTo(referencni_snimek_32f);
    kontrola_typu_snimku_32C1(referencni_snimek_32f);
    referencni_snimek_32f(cutoutStandard).copyTo(referencni_snimek_vyrez);
    Mat posunuty, posunuty_vyrez;
    if (scaleChange == true)
    {
        posunuty_temp(cutoutExtra).copyTo(posunuty);
        posunuty(cutoutStandard).copyTo(posunuty_vyrez);
        posunuty_temp.release();
    }
    else
    {
        posunuty_temp.copyTo(posunuty);
        posunuty(cutoutStandard).copyTo(posunuty_vyrez);
        posunuty_temp.release();
    }
    Mat posunuty_32f;
    posunuty.copyTo(posunuty_32f);
    kontrola_typu_snimku_32C1(posunuty_32f);

    /*******************************************************************************************/
    Mat slicovany1;
    Point3d pt1(0,0,0);
    if (scaleChange == true)
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
    if (scaleChange == false)
    {
        pt1 = fk_translace_hann(referencni_snimek_32f,posunuty_32f);
        //qDebug() << "PT1 bez zmeny velikosti Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
    }

    if (pt1.x>=55 || pt1.y>=55)
    {
        definitiveTranslation = pt1;
        //qDebug() << "PT1 nad 55 - Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        return registrationSuccessfull = false;
    }
    else
    {
        //qDebug() << "PT1 v normálu Y: "<< (pt1.y) <<" a X: "<<(pt1.x);
        slicovany1 = translace_snimku(posunuty,pt1,rows,cols);
        cv::Mat slicovany1_32f_rotace,slicovany1_32f,slicovany1_vyrez;
        slicovany1.copyTo(slicovany1_32f);
        kontrola_typu_snimku_32C1(slicovany1_32f);
        Point3d vysledek_rotace = fk_rotace(referencni_snimek_32f,slicovany1_32f,angle,pt1.z,pt1);
        if (std::abs(vysledek_rotace.y) > 0.1){vysledek_rotace.y=0;}
        slicovany1_32f_rotace = rotace_snimku(slicovany1_32f,vysledek_rotace.y);
        slicovany1_32f_rotace(cutoutStandard).copyTo(slicovany1_vyrez);
        /**************************************************************************************/
        Point3d pt2 = fk_translace(referencni_snimek_vyrez,slicovany1_vyrez);
        if (pt2.x >= 55 || pt2.y >= 55)
        {
            definitiveTranslation = pt1;
            slicovany1.copyTo(fullyRegistratedImage);
            slicovany1.release();
            return registrationSuccessfull = false;
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
            slicovany2_rotace(cutoutStandard).copyTo(slicovany2_vyrez);
            Mat mezivysledek_vyrez,mezivysledek;
            slicovany2_rotace.copyTo(mezivysledek);
            slicovany2_vyrez.copyTo(mezivysledek_vyrez);
            //slicovany2.release();
            slicovany2_vyrez.release();
            slicovany2_32f.release();
            slicovany2_rotace.release();
            finalRotationAngle+=vysledek_rotace.y;
            vysledek_rotace.y = 0;
            /*************************************************************/
            int max_pocet_iteraci = 0;
            if (iteration == -1)
            {
                if (FWHM <= 20){max_pocet_iteraci = 2;}
                else if (FWHM > 20 && FWHM <= 30){max_pocet_iteraci = 4;}
                else if (FWHM > 30 && FWHM <= 35){max_pocet_iteraci = 6;}
                else if (FWHM > 35 && FWHM <= 40){max_pocet_iteraci = 8;}
                else if (FWHM > 40 && FWHM <= 45){max_pocet_iteraci = 10;}
                else if (FWHM > 45){max_pocet_iteraci = 5;};
            }
            if (iteration >= 1)
            {
                max_pocet_iteraci = iteration;
            }
            /*************************************************************/
            for (int i = 0; i < max_pocet_iteraci; i++)
            {
                Point3d rotace_ForLoop = fk_rotace(referencialImage,mezivysledek,angle,pt3.z,pt3);
                if (std::abs(rotace_ForLoop.y) > 0.1){rotace_ForLoop.y = 0;}
                else if (std::abs(finalRotationAngle+rotace_ForLoop.y)>0.1){rotace_ForLoop.y=0;}
                else {finalRotationAngle+=rotace_ForLoop.y;}
                //qDebug() << "rotace " << celkovy_uhel <<" se zmenou " << rotace_ForLoop.y << endl;
                Mat rotovany = rotace_snimku(mezivysledek,rotace_ForLoop.y);
                rotace_ForLoop.y = 0;
                Mat rotovany_vyrez;
                rotovany(cutoutStandard).copyTo(rotovany_vyrez);
                rotovany.release();
                Point3d pt4 = fk_translace(referencni_snimek_vyrez,rotovany_vyrez);
                rotovany_vyrez.release();
                if (pt4.x >= 55 || pt4.y >= 55)
                {
                    definitiveTranslation = pt3;
                    slicovany2.copyTo(fullyRegistratedImage);
                    return registrationSuccessfull = true;
                }
                else
                {
                    //qDebug() << "Y: "<< (pt4.y) <<" a X: "<<(pt4.x)<<endl;
                    pt3.x += pt4.x;
                    pt3.y += pt4.y;
                    pt3.z = pt4.z;
                    //qDebug() << "Y: "<< (pt3.y) <<" a X: "<<(pt3.x)<<endl<<endl;
                    Mat posunuty_temp = translace_snimku(posunuty,pt3,rows,cols);
                    Mat rotovany_temp = rotace_snimku(posunuty_temp,finalRotationAngle);
                    posunuty_temp.release();
                    rotovany_temp.copyTo(mezivysledek);
                    Mat rotovany_temp32f_vyrez;
                    kontrola_typu_snimku_32C1(rotovany_temp);
                    rotovany_temp(cutoutStandard).copyTo(rotovany_temp32f_vyrez);
                    rotovany_temp32f_vyrez.copyTo(mezivysledek_vyrez);
                    rotovany_temp.release();
                    rotovany_temp32f_vyrez.release();
                }
            }
            definitiveTranslation = pt3;
            mezivysledek.copyTo(fullyRegistratedImage);
            return registrationSuccessfull = true;
        }
    }
}

bool preprocessingCompleteRegistration(cv::Mat &reference,
                                       cv::Mat &image,
                                       QVector<double> parFrang,
                                       cv::Point2d verticalAnomaly,
                                       cv::Point2d horizontalAnomaly,
                                       cv::Point3d &fraMax,
                                       cv::Rect &anomalyArea,
                                       cv::Rect &cutoutStandard,
                                       cv::Rect &cutoutExtra,
                                       cv::VideoCapture &cap,
                                       bool& anomalyPresence,
                                       bool &scalingNeeded)
{
    try {
        cv::Point3d pt_temp(0.0,0.0,0.0);
        cv::Point3d frangi_bod(0.0,0.0,0.0);
        double width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        double height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);

        if (verticalAnomaly.x != 0.0 && verticalAnomaly.y != 0.0) // světelná anomálie
        {
            anomalyPresence = true;
            if (verticalAnomaly.x < width/2.0)
            {
                anomalyArea.x = 0;
                anomalyArea.y = int(verticalAnomaly.x);
                anomalyArea.width = int(width-int(verticalAnomaly.x)-1);
                anomalyArea.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
            if (verticalAnomaly.x > width/2.0)
            {
                anomalyArea.x = 0;
                anomalyArea.y = 0;
                anomalyArea.width = int(verticalAnomaly.x);
                anomalyArea.height = int(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            }
        }
        if (horizontalAnomaly.x != 0.0 && horizontalAnomaly.y != 0.0) // časová anomálie
        {
            anomalyPresence = true;
            if (horizontalAnomaly.x < height/2.0)
            {
                anomalyArea.x = int(horizontalAnomaly.y);
                anomalyArea.y = 0;
                anomalyArea.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
                anomalyArea.height = int(height-int(horizontalAnomaly.y)-1);
            }
            if (horizontalAnomaly.x > height/2.0)
            {
                anomalyArea.x = 0;
                anomalyArea.y = 0;
                anomalyArea.width = int(cap.get(CV_CAP_PROP_FRAME_WIDTH));
                anomalyArea.height = int(height-int(horizontalAnomaly.y)-1);
            }
        }


        if (fraMax.x == 0.0 && fraMax.y == 0.0){
            if (anomalyPresence == true)
                fraMax = frangi_analysis(reference(anomalyArea),1,1,0,"",1,pt_temp,parFrang);
            else
                fraMax = frangi_analysis(reference,1,1,0,"",1,pt_temp,parFrang);

            if (fraMax.z == 0.0)
            {
                qDebug()<<"Nalezeni maxima frangiho funkce se nezdarilo, proverte snimek!"<<endl;
            }
            qDebug()<<"Frangi analysis completed"<<endl;
        }

        bool changeScaling = false;
        int rows = reference.rows;
        int cols = reference.cols;
        int rowFrom = int(round(frangi_bod.y-0.9*frangi_bod.y));
        int rowTo = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
        int columnFrom = 0;
        int columnTo = 0;

        if (anomalyPresence == true && verticalAnomaly.y != 0.0 && int(verticalAnomaly.y)<(cols/2))
        {
            columnFrom = int(verticalAnomaly.y);
            changeScaling = true;
        }
        else
            columnFrom = int(round(frangi_bod.x-0.9*(frangi_bod.x)));

        if (anomalyPresence == true && verticalAnomaly.y != 0.0 &&  int(verticalAnomaly.y)>(cols/2))
        {
            columnTo = int(verticalAnomaly.y);
            changeScaling = true;
        }
        else
            columnTo = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));

        int cutout_width = columnTo-columnFrom;
        int cutout_height = rowTo - rowFrom;

        if ((cutout_height>480 || cutout_width>640)|| changeScaling == true)
        {
            cutoutExtra.x = columnFrom;
            cutoutExtra.y = rowFrom;
            cutoutExtra.width = cutout_width;
            cutoutExtra.height = cutout_height;
            reference(cutoutExtra).copyTo(image);
            frangi_bod = frangi_analysis(image,1,1,0,"",1,pt_temp,parFrang);
            rows = image.rows;
            cols = image.cols;
            rowFrom = int(round(frangi_bod.y-0.9*frangi_bod.y));
            rowTo = int(round(frangi_bod.y+0.9*(rows - frangi_bod.y)));
            columnFrom = int(round(frangi_bod.x-0.9*(frangi_bod.x)));
            columnTo = int(round(frangi_bod.x+0.9*(cols - frangi_bod.x)));
            cutout_width = columnTo-columnFrom;
            cutout_height = rowTo - rowFrom;
            cutoutStandard.x = columnFrom;
            cutoutStandard.y = rowFrom;
            cutoutStandard.width = cutout_width;
            cutoutStandard.height = cutout_height;
            scalingNeeded = true;
            fraMax = frangi_bod;
        }
        else
        {
            fraMax=frangi_bod;
            cutoutStandard.x = int(round(fraMax.x-0.9*(fraMax.x)));
            cutoutStandard.y = int(round(fraMax.y-0.9*fraMax.y));
            rowTo = int(round(fraMax.y+0.9*(rows - fraMax.y)));
            columnTo = int(round(fraMax.x+0.9*(cols - fraMax.x)));
            cutoutStandard.width = columnTo-cutoutStandard.x;
            cutoutStandard.height = rowTo - cutoutStandard.y;
            reference.copyTo(image);
            //obraz_temp.release();
        }
        return true;
    } catch (std::exception e) {
        return false;
    }
}
