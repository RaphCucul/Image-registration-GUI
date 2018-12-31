#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QGraphicsColorizeEffect>

class ErrorDialog : public QObject
{
    Q_OBJECT

public:
    enum class ErrorType{
        INFO,
        SOFT_ERROR,
        HARD_ERROR,
        WHAT_TODO
    };
    explicit ErrorDialog(QWidget *parent = nullptr);
    ~ErrorDialog();
    //void createInfoDialog(QString& text,int typeOfEvent);
    void evaluatePosition();
    void analyseParents(QWidget* widgetWithLabel);
    void setErrorType(ErrorDialog::ErrorType errorType);
    void initialiseErrorLabel();
    void Info();
    void SoftError();
    void HardError();
    void What_todo();

private slots:
    //void on_tlacitko2_clicked();
    //void on_tlacitko1_clicked();
signals:
    /*void ok();
    void abort();
    void continueInCalculations();*/
private:
    QString errorStatementToDisplay;
    int errorSeverity;
    QLabel* errorLabel;
    QGraphicsColorizeEffect* errorLabelEffect;
    QWidget* parentOfTheWidget;
    QWidget* widgetWithError;
    QWidget* errorLabelParent;
    ErrorType errorType = ErrorType::INFO;
};

#endif // ERRORDIALOG_H
