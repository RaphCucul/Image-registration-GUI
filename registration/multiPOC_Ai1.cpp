#include "multiPOC_Ai1.h"
#include "image_analysis/image_processing.h"
#include "registration/phase_correlation_function.h"
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>

#include <QDebug>
#include "image_analysis/frangi_utilization.h"

using cv::Mat;
using cv::Rect;
using cv::Point3d;
using std::cout;
using std::endl;
bool completeRegistration(cv::VideoCapture& cap,
                          cv::Mat& i_referencial,
                          cv::Mat &i_shifted,
                          int i_translatedNo,
                          double i_iteration,
                          double i_areaMaximum,
                          double i_angleLimit,
                          cv::Rect &i_cutoutExtra,
                          cv::Rect &i_cutoutStandard,
                          int i_scaleChanged,
                          cv::Mat& i_completelyRegistrated,
                          QVector<double> &i_pocX,
                          QVector<double> &i_pocY,
                          QVector<double> &i_maxAngles,
                          bool video)
{
    try {
        i_pocX.push_back(0.0);
        i_pocY.push_back(0.0);
        i_maxAngles.push_back(0.0);
        cv::Rect adjustedStandardCutout(0,0,0,0);
        qDebug()<<"Extra: "<<i_cutoutExtra.width<<" "<<i_cutoutExtra.height;
        qDebug()<<"Standard: "<<i_cutoutStandard.width<<" "<<i_cutoutStandard.height;
        double totalAngle = 0.0;

        //transformMatTypeTo8C3(i_referencial);
        //transformMatTypeTo8C3(shifted_temp);
        Mat referencialFrame_32f,referencialFrame_cutout;
        i_referencial.copyTo(referencialFrame_32f);
        transformMatTypeTo32C1(referencialFrame_32f);

        if (i_scaleChanged == 2){
            referencialFrame_32f(i_cutoutExtra).copyTo(referencialFrame_32f);
            adjustedStandardCutout = adjustStandardCutout(i_cutoutExtra,i_cutoutStandard,
                                                          i_referencial.rows,i_referencial.cols);
            referencialFrame_32f(adjustedStandardCutout).copyTo(referencialFrame_cutout);
        }
        else
            referencialFrame_32f(i_cutoutStandard).copyTo(referencialFrame_cutout);

        qDebug()<<referencialFrame_cutout.rows<<" "<<referencialFrame_cutout.cols;
        int rows = referencialFrame_32f.rows;
        int cols = referencialFrame_32f.cols;
        qDebug()<<rows<<" "<<cols;

        Mat hann;
        createHanningWindow(hann, referencialFrame_32f.size(), CV_32FC1);

        Mat shifted_temp,shifted_32f,shifted_cutout;
        if (video) {
            cap.set(CV_CAP_PROP_POS_FRAMES,i_translatedNo);
            if(!cap.read(shifted_temp))
                return false;
        }
        else {
            i_shifted.copyTo(shifted_temp);
        }

        shifted_temp.copyTo(shifted_32f);
        shifted_temp.release();
        transformMatTypeTo32C1(shifted_32f);
        if (i_scaleChanged == 2)
        {
            shifted_32f(i_cutoutExtra).copyTo(shifted_32f);
            qDebug()<<"Extra shifted done";
            shifted_32f(adjustedStandardCutout).copyTo(shifted_cutout);
        }
        else
            shifted_32f(i_cutoutStandard).copyTo(shifted_cutout);

        qDebug()<<shifted_32f.rows<<" "<<shifted_32f.cols;
        qDebug()<<shifted_cutout.rows<<" "<<shifted_cutout.cols;

        Point3d pt1(0.0,0.0,0.0);
        if (i_scaleChanged != 3)
        {
            pt1 = pc_translation_hann(referencialFrame_32f,shifted_32f,i_areaMaximum);
            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = pc_translation(referencialFrame_32f,shifted_32f,i_areaMaximum);

            if (std::abs(pt1.x)>=290 || std::abs(pt1.y)>=290)
                pt1 = pc_translation(referencialFrame_cutout,shifted_cutout,i_areaMaximum);
        }
        else
        {
            pt1 = pc_translation_hann(referencialFrame_32f,shifted_32f,i_areaMaximum);
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
            //if (i_translatedNo == 0)
                qDebug()<<"PT1: "<<pt1.x<<" "<<pt1.y;
            Mat registrated1;
            registrated1 = frameTranslation(shifted_32f,pt1,rows,cols);
            qDebug()<<"registrated1 done";
            cv::Mat registrated1_32f_rotace,registrated1_32f,registrated1_cutout;
            registrated1.copyTo(registrated1_32f);
            transformMatTypeTo32C1(registrated1_32f);
            Point3d rotation_result = pc_rotation(referencialFrame_32f,registrated1_32f,i_angleLimit,pt1.z,pt1);
            if (std::abs(rotation_result.y) > i_angleLimit)
                rotation_result.y=0;
            qDebug()<<"rotation1 done ("<<rotation_result.y<<")";

            i_maxAngles[0] = rotation_result.y;
            registrated1_32f_rotace = frameRotation(registrated1_32f,rotation_result.y);

            if (i_scaleChanged != 2)
                registrated1_32f_rotace(i_cutoutStandard).copyTo(registrated1_cutout);
            else
                registrated1_32f_rotace(adjustedStandardCutout).copyTo(registrated1_cutout);
            qDebug()<<"rotated";

            Point3d pt2(0.0,0.0,0.0);
            pt2 = pc_translation(referencialFrame_cutout,registrated1_cutout,i_areaMaximum);
            if (pt2.x >= 55 || pt2.y >= 55)
            {
                i_pocX[0] = 999.0;
                i_pocY[0] = 999.0;
                i_maxAngles[0] = 999.0;
                registrated1.copyTo(i_completelyRegistrated);
                registrated1.release();
                return false;
            }
            else
            {
                double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt2.z);
                double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
                qDebug()<<"FWHM for "<<i_translatedNo<<" = "<<FWHM;
                registrated1.release();
                registrated1_32f.release();
                registrated1_cutout.release();
                //if (i_translatedNo == 0)
                    qDebug()<<"PT2: "<<pt2.x<<" "<<pt2.y;

                Point3d pt3(0.0,0.0,0.0);
                pt3.x = pt1.x+pt2.x;
                pt3.y = pt1.y+pt2.y;
                pt3.z = pt2.z;
                i_pocX[0] = pt3.x;
                i_pocY[0] = pt3.y;
                Mat registrated2 = frameTranslation(shifted_32f,pt3,rows,cols);
                Mat registrated2_32f,registrated2_cutout;
                registrated2.copyTo(registrated2_32f);
                transformMatTypeTo32C1(registrated2_32f);
                Mat registrated2_rotace = frameRotation(registrated2_32f,rotation_result.y);
                qDebug()<<"translation2 done";

                if (i_scaleChanged != 2)
                    registrated2_rotace(i_cutoutStandard).copyTo(registrated2_cutout);
                else
                    registrated2_rotace(adjustedStandardCutout).copyTo(registrated2_cutout);

                Mat interresult_cutout,interresult;
                registrated2_rotace.copyTo(interresult);
                registrated2_cutout.copyTo(interresult_cutout);
                //registrated2.release();
                registrated2_cutout.release();
                registrated2_32f.release();
                registrated2_rotace.release();
                totalAngle+=rotation_result.y;
                rotation_result.y = 0;
                int maxIterationCount = 0;
                if (i_iteration == -1.0)
                {
                    if (FWHM <= 20){maxIterationCount = 2;}
                    else if (FWHM > 20 && FWHM <= 30){maxIterationCount = 4;}
                    else if (FWHM > 30 && FWHM <= 35){maxIterationCount = 6;}
                    else if (FWHM > 35 && FWHM <= 40){maxIterationCount = 8;}
                    else if (FWHM > 40 && FWHM <= 45){maxIterationCount = 10;}
                    else if (FWHM > 45){maxIterationCount = 5;}
                }
                if (i_iteration >= 1.0)
                {
                    maxIterationCount = int(i_iteration);
                }
                qDebug()<<"Entering loop ("<<maxIterationCount<<")";
                for (int i = 0; i < maxIterationCount; i++)
                {
                    Point3d rotace_ForLoop(0.0,0.0,0.0);
                    rotace_ForLoop = pc_rotation(referencialFrame_32f,interresult,i_angleLimit,pt3.z,pt3);
                    if (std::abs(rotace_ForLoop.y) > i_angleLimit)
                        rotace_ForLoop.y = 0.0;
                    else if (std::abs(totalAngle+rotace_ForLoop.y)>i_angleLimit)
                        rotace_ForLoop.y=0.0;
                    else
                        totalAngle+=rotace_ForLoop.y;

                    qDebug()<<"rotation done ("<<totalAngle<<")";
                    Mat rotated;
                    if (rotace_ForLoop.y != 0.0)
                        rotated = frameRotation(interresult,rotace_ForLoop.y);
                    else
                        interresult.copyTo(rotated);
                    qDebug()<<"rotated";

                    rotace_ForLoop.y = 0.0;
                    Mat rotated_cutout;

                    if (i_scaleChanged != 2)
                        rotated(i_cutoutStandard).copyTo(rotated_cutout);
                    else
                        rotated(adjustedStandardCutout).copyTo(rotated_cutout);

                    rotated.release();
                    Point3d pt4(0.0,0.0,0.0);
                    qDebug()<<referencialFrame_cutout.cols<<" "<<referencialFrame_cutout.rows;
                    qDebug()<<rotated_cutout.cols<<" "<<rotated_cutout.rows;
                    pt4 = pc_translation(referencialFrame_cutout,rotated_cutout,i_areaMaximum);
                    rotated_cutout.release();
                    if (pt4.x >= 55 || pt4.y >= 55)
                    {
                        registrated2.copyTo(i_completelyRegistrated);
                        qDebug()<<"Frame "<<i_translatedNo<<" terminated because \"condition 55\" reached.";
                        break;
                    }
                    else
                    {
                        pt3.x += pt4.x;
                        pt3.y += pt4.y;
                        pt3.z = pt4.z;
                        //if (i_translatedNo == 0)
                            qDebug()<<"PT3 loop: "<<pt3.x<<" "<<pt3.y;
                        i_pocX[0] = pt3.x;
                        i_pocY[0] = pt3.y;
                        i_maxAngles[0] = totalAngle;
                        Mat shifted_temp = frameTranslation(shifted_32f,pt3,rows,cols);
                        Mat rotated_temp = frameRotation(shifted_temp,totalAngle);
                        shifted_temp.release();
                        rotated_temp.copyTo(interresult);
                        transformMatTypeTo32C1(rotated_temp);
                        rotated_temp.release();
                    }
                }
                interresult.copyTo(i_completelyRegistrated);
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

bool preprocessingCompleteRegistration(cv::Mat &i_referencial,
                                       cv::Point3d &i_frangi_point,
                                       cv::Rect &i_standardCutout,
                                       QMap<QString, double> i_frangiParameters,
                                       QMap<QString, double> frangiRatios,
                                       QMap<QString, int> frangiMargins)
{
    try {
        cv::Point3d pt_temp(0.0,0.0,0.0);
        int rows = i_referencial.rows;
        int cols = i_referencial.cols;

        // calculate frangi
        i_frangi_point = frangi_analysis(i_referencial,1,1,0,"",1,pt_temp,i_frangiParameters,frangiMargins);
        if (i_frangi_point.z == 0.0)
            return false;

        // extra cutout is not necessary, just calculate standard cutout
        i_standardCutout = calculateStandardCutout(i_frangi_point,frangiRatios,rows,cols);

        return true;

        /*if (i_standardCutout.y != 0.0) // x for vertical anomaly
        {
            if (i_standardCutout.y < (width/2))
            {
                i_anomalyArea.y = int(i_standardCutout.y);
                i_anomalyArea.height = int(height-int(i_standardCutout.y));
                i_anomalyArea.width = int(i_cap.get(CV_CAP_PROP_FRAME_WIDTH));
            }
            if (i_standardCutout.y > (width/2))
            {
                i_anomalyArea.height = int(i_standardCutout.y);
                i_anomalyArea.width = int(i_cap.get(CV_CAP_PROP_FRAME_WIDTH));
            }
            verticalAnomaly = true;
        }
        if (i_extraCutout.x != 0.0) // horizontal anomaly
        {
            if (i_extraCutout.x < (height/2))
            {
                i_anomalyArea.x = int(i_extraCutout.x);
                if (!verticalAnomaly)
                    i_anomalyArea.height = int(i_cap.get(CV_CAP_PROP_FRAME_HEIGHT));
                i_anomalyArea.width = int(width-int(i_extraCutout.x));
            }
            if (i_extraCutout.x > (width/2))
            {
                if (!verticalAnomaly)
                    i_anomalyArea.height = int(i_cap.get(CV_CAP_PROP_FRAME_HEIGHT));
                i_anomalyArea.width = int(i_extraCutout.x);
            }
            horizontalAnomaly = true;
        }*/
        /*if (verticalAnomaly == true || horizontalAnomaly == true)
            frangi_point = frangi_analysis(i_referencial(i_anomalyArea),1,1,0,"",3,pt_temp,i_frangiParameters,frangiMargins);
        else
            frangi_point = frangi_analysis(i_referencial,1,1,0,"",1,pt_temp,i_frangiParameters,frangiMargins);

        if (frangi_point.z == 0.0)
        {
            return false;
        }
        else
        {
            qDebug()<<"Preprocessing - frangi: "<<frangi_point.x<<frangi_point.y;
            bool needToChangeScale = false;
            int rows = i_referencial.rows;
            int cols = i_referencial.cols;
            int rowFrom = int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y));
            int rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(rows - frangi_point.y)));
            int columnFrom = 0;
            int columnTo = 0;

            if (verticalAnomaly == true && standardCutoutEdited.y != 0.0 && int(standardCutoutEdited.y)<(cols/2))
            {
                columnFrom = int(standardCutoutEdited.y);
                needToChangeScale = true;
            }
            else
                columnFrom = int(round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x)));

            if (verticalAnomaly == true && standardCutoutEdited.y != 0.0 &&  int(standardCutoutEdited.y)>(cols/2))
            {
                columnTo = int(standardCutoutEdited.y);
                needToChangeScale = true;
            }
            else
                columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(cols - frangi_point.x)));

            int cutout_width = columnTo-columnFrom;
            int cutout_height = rowTo - rowFrom;

            if (needToChangeScale)
            {
                i_cutoutExtra.x = columnFrom;
                i_cutoutExtra.y = rowFrom;
                i_cutoutExtra.width = cutout_width;
                i_cutoutExtra.height = cutout_height;

                i_referencial(i_cutoutExtra).copyTo(i_newreferencial);

                frangi_point = frangi_analysis(i_newreferencial,1,1,0,"",1,pt_temp,i_frangiParameters,frangiMargins);
                rows = i_newreferencial.rows;
                cols = i_newreferencial.cols;
                rowFrom = int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y));
                rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(rows - frangi_point.y)));
                columnFrom = int(round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x)));
                columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(cols - frangi_point.x)));
                cutout_width = columnTo-columnFrom;
                cutout_height = rowTo - rowFrom;
                i_cutoutStandard.x = columnFrom;
                i_cutoutStandard.y = rowFrom;
                i_cutoutStandard.width = cutout_width;
                i_cutoutStandard.height = cutout_height;
                i_scaleChange = true;
            }
            else
            {
                i_cutoutStandard.x = int(round(frangi_point.x-frangiRatios["left_r"]*(frangi_point.x)));
                i_cutoutStandard.y = int(round(frangi_point.y-frangiRatios["top_r"]*frangi_point.y));
                rowTo = int(round(frangi_point.y+frangiRatios["bottom_r"]*(rows - frangi_point.y)));
                columnTo = int(round(frangi_point.x+frangiRatios["right_r"]*(cols - frangi_point.x)));
                i_cutoutStandard.width = columnTo-i_cutoutStandard.x;
                i_cutoutStandard.height = rowTo - i_cutoutStandard.y;

                i_referencial.copyTo(i_newreferencial);
            }
            return true;
        }*/
    } catch (std::exception &e) {
        qWarning()<<"Image preprocessing error: "<<e.what();
        return false;
    }
}

cv::Rect calculateStandardCutout(cv::Point3d i_frangi, QMap<QString, double> i_ratios,
                                 int i_rows, int i_cols){
    cv::Rect output;
    int rowFrom = int(round(i_frangi.y-i_ratios["top_r"]*i_frangi.y));
    int rowTo = int(round(i_frangi.y+i_ratios["bottom_r"]*(i_rows - i_frangi.y)));
    int columnFrom = int(round(i_frangi.x-i_ratios["left_r"]*(i_frangi.x)));
    int columnTo = int(round(i_frangi.x+i_ratios["right_r"]*(i_cols - i_frangi.x)));
    int cutout_width = columnTo-columnFrom;
    int cutout_height = rowTo - rowFrom;
    output.x = columnFrom;
    output.y = rowFrom;
    output.width = cutout_width;
    output.height = cutout_height;

    return output;
}

cv::Rect adjustStandardCutout(cv::Rect i_extraCutoutParameters, cv::Rect i_originalStandardCutout,
                              int i_rows, int i_cols){
    cv::Rect output;
    // when the extra cutout is applied, standard cutout must be recalculated to fit the new frame
    // x and y coordinates are adjusted to (0,0) point
    output.x = i_originalStandardCutout.x - i_extraCutoutParameters.x;
    output.y = i_originalStandardCutout.y - i_extraCutoutParameters.y;
    // width and height are adjusted to the row and column count of the standard "full" frame
    output.width = i_originalStandardCutout.width - (i_cols - i_extraCutoutParameters.width);
    output.height = i_originalStandardCutout.height - (i_rows - i_extraCutoutParameters.height);

    return output;
}
