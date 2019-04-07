#ifndef KOREKCE_ZAPIS_H_INCLUDED
#define KOREKCE_ZAPIS_H_INCLUDED
#include <opencv2/opencv.hpp>
#include <QVector>
bool licovani_nejvhodnejsich_snimku(cv::VideoCapture& i_cap,
                                    cv::Mat& i_referencialFrame,
                                    cv::Point3d i_coordsFrangiStandardReferencialReverse,
                                    int i_index_noved,
                                    int i_iteration,
                                    double i_areaMaximum,
                                    double i_angle,
                                    cv::Rect& i_cutoutExtra,
                                    cv::Rect& i_cutoutStandard,
                                    bool i_scaleChanged,
                                    QVector<double> i_frangiParameters,
                                    QVector<double> &_pocX,
                                    QVector<double> &_pocY,
                                    QVector<double> &_frangiX,
                                    QVector<double> &_frangiY,
                                    QVector<double> &_frangiEucl,
                                    QVector<double> &_maxAngles);

bool eventualni_korekce_translace(cv::Mat& i_frameRegistrated,
                                  cv::Mat& i_frame,
                                  cv::Mat& i_frameCorrected,
                                  cv::Rect& i_cutoutStandard,
                                  cv::Point3d& i_correction);

#endif // KOREKCE_ZAPIS_H_INCLUDED
