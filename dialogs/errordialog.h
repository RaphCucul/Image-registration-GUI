#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QString>
#include <QWidget>
#include <QLabel>
#include <QGraphicsColorizeEffect>
#include <QGraphicsDropShadowEffect>
#include <QSequentialAnimationGroup>

/**
 * @class ErrorDialog
 * @brief The ErrorDialog class enables to create a special widget attached to another (standard program) widget. The special
 * widget can have one of four icons - soft/hard error, information and what to do. A unique statement is connected to invoked
 * ErrorDialog object. The statement is visible as a tooltip. ErrorDialog object is closed by mouse click or it is possible
 * to set close timeout.
 */
class ErrorDialog : public QObject
{
    Q_OBJECT

public:
    /**
     * @enum
     * @brief The ErrorType enum holds types of error dialog.
     */
    enum class ErrorType{
        INFO,
        SOFT_ERROR,
        HARD_ERROR,
        WHAT_TODO
    };
    explicit ErrorDialog(QWidget* widgetWithNotification,QWidget *parent = nullptr);
    ~ErrorDialog();
    /**
     * @brief The main function where all parameters of the error are set. A user needs to set only 3 parameters and then call
     * show() function. The third parameter can be int or string (used in thread classes).
     * @param position - the position of the dialog to the widget - left, center, right
     * @param EventType - defines type of error dialog, according to the ErrorType enum - hardError,softError,info,whatToDo
     * @param errorNumber - connected to the list of statements in errors.h
     */
    void evaluate(QString position,QString EventType,int errorNumber);

    /**
     * @brief The main function where all parameters of the error are set. A user needs to set only 3 parameters and then call
     * show() function. The third parameter can be int or string (used in thread classes).
     * @param position - the position of the dialog to the widget - left, center, right
     * @param EventType - defines type of error dialog, according to the ErrorType enum - hardError,softError,info,whatToDo
     * @param errorMessage - a custom statement defined by a user
     */
    void evaluate(QString position,QString EventType,QString errorMessage);

    /**
     * @brief Checks, if the widget has the error dialog visible or not.
     * @return - true if the error dialog is visible
     */
    bool isEvaluated();

    /**
     * @brief Makes the error label visible. If a timer is set, the error dialog will be closed automatically.
     * @param timerStop - hide the dialog automatically
     */
    void show(bool timerStop);

    /**
     * @brief Hides the error dialog.
     */
    void hide();

signals:
    void mouseClicked();
private slots:
    void hideErrorIcon();

private:
    /**
     * @brief Adds the hover event to the error label - if a user hovers over the label, signal
     * is emitted and the statement is shown.
     * @param obj = error QLabel
     * @param event - mouse hover
     */
    bool eventFilter(QObject *obj, QEvent *event);

    /**
     * @brief Analyses the position where the error label will be mapped to.
     * @param position - left, center or right
     */
    void evaluatePosition(QString position);

    /**
     * @brief The parent of the widget with the error label is determined.
     */
    void analyseParents();

    /**
     * @brief Creates an error label based on the type of error.
     * @param errorType - see ErrorType structure
     */
    void setErrorType(ErrorDialog::ErrorType errorType);

    /**
     * @brief Initializes the error label widget and the parent widget of the label.
     */
    void initialiseErrorLabel();

    /**
     * @brief Initializes the shadow of the error label.
     */
    void initShadowEffect();

    /**
     * @brief Initializes color effect of the error label.
     */
    void initColorEffect();

    /**
     * @brief Initializes the glow effect of the error label.
     */
    void initAnimation();

    /**
     * @brief Sets "info" type of the error label.
     */
    void Info();

    /**
     * @brief Sets "softError" type of the error label.
     */
    void SoftError();

    /**
     * @brief Sets "hardError" type of the error label.
     */
    void HardError();

    /**
     * @brief Sets "whatToDo" type of the error label.
     */
    void What_todo();

    /**
     * @brief Shows the message when the error label is hovered.
     * @param errorMessage - showned error statement (predefined or custom)
     */
    void showMessage(const QString errorMessage);

    /**
     * @brief Adds pixmap to the error label.
     * @param _pixmap
     */
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
