#ifndef VECTOR_OPERATIONS_H_INCLUDED
#define VECTOR_OPERATIONS_H_INCLUDED
#include <QVector>
#include <QString>
#include <QMap>
#include <opencv2/opencv.hpp>

/**
 * @file vector_operations.h
 * The file contains declarations of functions supporting different vector operations.
 */

/**
 * @brief Calculates median of the vector.
 * @param[in] values
 */
double median_VectorDouble(QVector<double> i_input);

/**
 * @brief Checks if the maximum of entropy or tennenfrad is not to high and if so, it finds better value.
 * @param[in] inputValues
 * @return Maximum value of entropy or tennegrad that suits the given conditions.
 */
double checkMaximum(QVector<double>& i_input);

/**
 * @brief Merges two vectors into the only one and preserves only unique inputs.
 * @param[in] input1
 * @param[in] input2
 * @return Merged vector.
 */
QVector<int> mergeVectors(QVector<int>& i_input1,QVector<int>& i_input2);

/**
 * @brief Divides input vector into 9 separate "windows" for processing.
 * @param[in] inputValues
 * @param[in] windows
 * @param[in] restToEnd
 */
void vectorWindows(QVector<double>& i_inputValues, QVector<double>& i_windows, double& i_restToEnd);

/**
 * @brief Calculates median from the values in the given part (window) of a input vector.
 * @param[in] inputValues
 * @param[in] inputWindows
 * @param[in] restToEnd
 */
QVector<double> mediansOfVector(QVector<double>& i_inputValues,
                                    QVector<double>& i_inputWindows,
                                    double i_restToEnd);

/**
 * @brief Takes a vector with entropy or tennengrad values and look for frames with values out of the range
 * defined by thresholds and tolerance.
 * @param[in] inputValues
 * @param[in] medianVector
 * @param[in] windowsVector
 * @param[in] recalculatedMaximum
 * @param[in] thresholds
 * @param[in] tolerance
 * @param[in] dmin
 * @param[in] restToEnd
 * @param[out] badFrames
 * @param[in] forEvaluation
 */
bool analysisFunctionValues(QVector<double>& i_inputValues,
                            QVector<double>& i_medianVector,
                            QVector<double>& i_windowsVector,
                            double& i_recalculatedMaximum,
                            QVector<double>& i_thresholds,
                            double& i_tolerance,
                            int& i_dmin,
                            double& i_restToEnd,
                            QVector<int>& i_badFrames,
                            QVector<double>& i_forEvaluation,
                            bool explicitThresholds);

/**
 * @brief Determines a referential frame from a vector of frames.
 * @param[in] recalculatedMaximum
 * @param[in] forEvaluation
 * @param[in] inputValues
 * @return The index of the referential frame.
 */
int findReferentialNumber(double& i_recalculatedMaximum, QVector<double>& i_forEvaluation,
                                 QVector<double>& i_inputValues);

/**
 * @brief Checks the "integrity" of the bad frames vector and add those frames which could be skipped
 * by mistake
 * @param[out] badFrames
 */
void integrityCheck(QVector<int>& i_badFrames);

/**
 * @brief Looks for a frame with evaluation value 2 (=referential frame).
 * @param[in] vectorEvaluation
 * @return The index of the referential frame.
 */
int findReferenceFrame(QVector<int> i_vectorEvaluation);

/**
 * @brief Creates sub-vectors of frames. The number of the sub-vectors corresponds with the total threadCount.
 * @param[in] totalLength
 * @param[in] threadCount
 * @return A vector of vectors with indexes of frames.
 */
QVector<QVector<int>> divideIntoPeaces(int i_totalLength, int i_threadCount);

/**
 * @brief Analyses given path to videos, checks for the presence of *.avi files and loads first video from the
 * list of found videos to cap variable and put the video name into a line edit widget.
 * @param[in] analysedFolder
 * @param whereToSave
 */
void analyseAndSaveFirst(QString i_analysedFolder, QMap<QString, QString> &i_whereToSave);

/**
 * @brief Template function which calculates the sum of a vector.
 * @tparam[in] T - defines type of the vector (double,int)
 * @tparam[out] output - the sum of the vector
 */
template <typename T>
T vectorSum(QVector<T> i_input) {
    T output=0;
    for (int vectorElement = 0; vectorElement < i_input.length(); vectorElement++)
        output += i_input[vectorElement];

    return output;
}

#endif // VECTOR_OPERATIONS_H_INCLUDED
