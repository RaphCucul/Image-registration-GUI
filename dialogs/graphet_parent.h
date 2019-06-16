#ifndef GRAPHET_PARENT_H
#define GRAPHET_PARENT_H

#include <QWidget>
#include <QDialog>

namespace Ui {
class GraphET_parent;
}

class GraphET_parent : public QDialog
{
    Q_OBJECT

public:
    explicit GraphET_parent(QStringList i_chosenList, QWidget *parent = nullptr);
    explicit GraphET_parent(QStringList i_chosenList,
                            QVector<QVector<double>> i_entropy,
                            QVector<QVector<double>> i_tennengrad,
                            QVector<QVector<int>> i_FirstEvalEntropy,
                            QVector<QVector<int>> i_FirstEvalTennengrad,
                            QVector<QVector<int>> i_FirstDecisionResults,
                            QVector<QVector<int>> i_SecondDecisionResults,
                            QVector<QVector<int>> i_CompleteEvaluation,
                            QWidget *parent = nullptr);
    ~GraphET_parent();
signals:
private:
    /**
     * @brief Function analyses the length of all given video names.
     */
    void analyseNames();

    /**
     * @brief Function loads data from chosen *.dat files, creates GraphET class object and adds a new tab
     * to the tabWidget.
     */
    void loadAndShow();

    /**
     * @brief Function corresponds to loadAndShow() function but it does not load data from the *.dat file,
     * it creates the grap directly from given data. Used for SingleVideoET and MultipleVideoET classes.
     */
    void processAndShow(QVector<QVector<double> > i_entropy,
                        QVector<QVector<double> > i_tennengrad,
                        QVector<QVector<int> > i_FirstEvalEntropy,
                        QVector<QVector<int> > i_FirstEvalTennengrad,
                        QVector<QVector<int> > i_FirstDecisionResults,
                        QVector<QVector<int> > i_SecondDecisionResults,
                        QVector<QVector<int> > i_CompleteEvaluation);

    Ui::GraphET_parent *ui;
    QStringList fileList;
    QStringList neededParameters = {"entropy","tennengrad","firstEvalEntropy","firstEvalTennengrad",
                                   "firstEval","secondEval","evaluation"};
};

#endif // GRAPHET_PARENT_H
