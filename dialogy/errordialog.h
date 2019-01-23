#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QGraphicsColorizeEffect>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>

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
    explicit ErrorDialog(QWidget* widgetWithNotification,QWidget *parent = nullptr);
    ~ErrorDialog();
    //void createInfoDialog(QString& text,int typeOfEvent);
    bool eventFilter(QObject *obj, QEvent *event);
    void evaluatePosition(QString position);
    void analyseParents();
    void setErrorType(ErrorDialog::ErrorType errorType);
    void evaluate(QString position,QString EventType,int errorNumber);
    bool isEvaluated();
    void initialiseErrorLabel();
    void Info();
    void SoftError();
    void HardError();
    void What_todo();
    void initShadowEffect();
    void initColorEffect();
    void initAnimation();
    void show();
    void showMessage(const QString errorMessage);
    void hide();

private slots:
    //void on_tlacitko2_clicked();
    //void on_tlacitko1_clicked();
signals:
    void mouseHovered(QLabel*);
    /*void ok();
    void abort();
    void continueInCalculations();*/
private:
    QString errorStatementToDisplay;
    int errorSeverity;
    QLabel* errorLabel;
    QGraphicsColorizeEffect* highlightingEffect;
    QGraphicsDropShadowEffect* glowEffect;
    QSequentialAnimationGroup* animation;
    QWidget* parentOfTheWidget;
    QWidget* widgetWithError;
    QWidget* errorLabelParent;
    ErrorType errorType = ErrorType::INFO;
    bool errorLabelinitialized = false;
};

#endif // ERRORDIALOG_H
