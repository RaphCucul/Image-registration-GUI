#ifndef VECTOR_OPERATIONS_H_INCLUDED
#define VECTOR_OPERATIONS_H_INCLUDED
#include <QVector>
#include <QString>
#include <QMap>
#include <opencv2/opencv.hpp>

/**
 * @brief Calculate median of the vector
 * @param values
 * @return
 */
double median_VectorDouble(QVector<double> i_input);

/**
 * @brief Check if the maximum of entropy or tennenfrad is not to high and find better value if so
 * @param inputValues
 * @return
 */
double checkMaximum(QVector<double>& i_input);

/**
 * @brief Simply merge two vectors into the only one and preserve only unique unputs
 * @param input1
 * @param input2
 * @return
 */
QVector<int> mergeVectors(QVector<int>& i_input1,QVector<int>& i_input2);

/**
 * @brief Creates implicitaly 9 windows = parts of the vector
 * @param inputValues
 * @param windows
 * @param restToEnd
 */
void vectorWindows(QVector<double>& i_inputValues, QVector<double>& i_windows, double& i_restToEnd);

/**
 * @brief Calculate median from the values in the vector window part
 * @param inputValues
 * @param inputWindows
 * @param restToEnd
 * @return
 */
QVector<double> mediansOfVector(QVector<double>& i_inputValues,
                                    QVector<double>& i_inputWindows,
                                    double i_restToEnd);

/**
 * @brief Function takes vector of entropy or tennengrad values and look for frames with values not fitting into
 * the value range defined by thresholds and tolerance. This is the first step of bad frames elimination process.
 * @param inputValues
 * @param medianVector
 * @param windowsVector
 * @param recalculatedMaximum
 * @param thresholds
 * @param tolerance
 * @param dmin
 * @param restToEnd
 * @param badFrames
 * @param forEvaluation
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
 * @brief Find referencial frame.
 * @param recalculatedMaximum
 * @param forEvaluation
 * @param inputValues
 * @return
 */
int findReferencialNumber(double& i_recalculatedMaximum, QVector<double>& i_forEvaluation,
                                 QVector<double>& i_inputValues);

/**
 * @brief Function checks the "integrity" of the bad frames vector and add those frames which could be skipped
 * by mistake
 * @param badFrames
 */
void integrityCheck(QVector<int>& i_badFrames);

/**
 * @brief Find the frame with number 2 index.
 * @param vectorEvaluation
 * @return
 */
int findReferenceFrame(QVector<int> i_vectorEvaluation);

/**
 * @brief Function creates ranges of values for each qthread object
 * @param totalLength
 * @param threadCount
 * @return
 */
QVector<QVector<int>> divideIntoPeaces(int i_totalLength, int i_threadCount);

/**
 * @brief Function analyse given path to videos, check the present *.avi files and load first video from the
 * list of found videos to cap variable and put the video name into line edit
 * @param analysedFolder
 * @param whereToSave
 */
void analyseAndSaveFirst(QString i_analysedFolder, QMap<QString, QString> &i_whereToSave);

template <typename T>
T vectorSum(QVector<T> i_input) {
    T output=0;
    for (int vectorElement = 0; vectorElement < i_input.length(); vectorElement++)
        output += i_input[vectorElement];

    return output;
}

/**
 * @brief vectorSum
 * @param i_input
 * @return
 */
//int vectorSum(QVector<int> i_input);

/**
 * @brief vectorSum
 * @param i_input
 * @return
 */
//double vectorSum(QVector<double> i_input);
#endif // VECTOR_OPERATIONS_H_INCLUDED
