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
    /**
     * @brief The main function where all parameters of the error are set. Everything else
     * is hidden in this function, therefore user needs to set only 3 parameters and then call
     * show() function. The third parameter can be int or string (used in thread classes).
     * @param position - where to place the error label
     * @param EventType
     * @param errorNumber - errors.h
     */
    void evaluate(QString position,QString EventType,int errorNumber);

    /**
     * @brief The main function where all parameters of the error are set. Everything else
     * is hidden in this function, therefore user needs to set only 3 parameters and then call
     * show() function. The third parameter can be int or string (used in thread classes).
     * @param position
     * @param EventType
     * @param errorMessage
     */
    void evaluate(QString position,QString EventType,QString errorMessage);

    /**
     * @brief Function checks, if the corresponding element has the error label visible or not.
     * @return
     */
    bool isEvaluated();

    /**
     * @brief Function make the error label visible.
     */
    void show(bool timerStop);

    /**
     * @brief Function hides the error label.
     */
    void hide();

signals:
    void mouseClicked();
private slots:
    void hideErrorIcon();

private:
    /**
     * @brief The function adds the hover event to the error label - if user hover over the label, signal
     * is emitted and the error message is shown.
     * @param obj = error QLabel
     * @param event - mouse hover
     * @return
     */
    bool eventFilter(QObject *obj, QEvent *event);

    /**
     * @brief Function analyse the position of the widget where error label will be mapped to.
     * @param position - left, center or right
     */
    void evaluatePosition(QString position);

    /**
     * @brief The parent of the widget with error label is determined.
     */
    void analyseParents();

    /**
     * @brief Function creates the error label based on the type of error.
     * @param errorType - see ErrorType structure
     */
    void setErrorType(ErrorDialog::ErrorType errorType);

    /**
     * @brief Function initializes the error label widget and the parent widget of the label.
     */
    void initialiseErrorLabel();

    /**
     * @brief Function initializes the shadow of the error label.
     */
    void initShadowEffect();

    /**
     * @brief Function initializes color effect of the error label.
     */
    void initColorEffect();

    /**
     * @brief Function initializes the glow effect of the error label.
     */
    void initAnimation();

    void Info();
    void SoftError();
    void HardError();
    void What_todo();

    /**
     * @brief Function is responsible for showing the message when the error label is hovered.
     * @param errorMessage
     */
    void showMessage(const QString errorMessage);

    void fillErrorLabel(QPixmap &_pixmap);

    QString errorStatementToDisplay;
    int errorSeverity;
    QLabel* errorLabel = nullptr;
    QGraphicsColorizeEffect* highlightingEffect = nullptr;
    QGraphicsDropShadowEffect* glowEffect = nullptr;
    QSequentialAnimationGroup* animation = nullptr;
    QWidget* parentOfTheWidget = nullptr;
    QWidget* widgetWithError = nullptr;
    QWidget* errorLabelParent = nullptr;
    ErrorType errorType = ErrorType::INFO;
    bool errorLabelinitialized = false;
    QTimer* timer = nullptr;
};

#endif // ERRORDIALOG_H
