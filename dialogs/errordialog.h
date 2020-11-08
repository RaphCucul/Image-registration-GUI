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
 * @brief Enables to create a special widget attached to another (standard program) widget. The special
 * widget can have one of four icons - soft/hard error, information and "what to do".
 *
 * A unique statement in tooltip appears when mouse is hovered over the ErrorDialog object. The statement is visible as a
 * tooltip. ErrorDialog object is closed by mouse click or after elapsed time.
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
     * @brief The main function where all parameters of the error dialog are set. A user needs to set only 3 parameters and then call
     * show() function. The third parameter can be an integer or string (used in thread classes).
     * @param[in] position - the position of the dialog against the widget - left, center, right
     * @param[in] EventType - defines the type of an error dialog, according to the ErrorType enum -
     * hardError, softError, info, whatToDo
     * @param[in] errorNumber - connected to the list of statements in errors.h
     */
    void evaluate(QString position,QString EventType,int errorNumber);

    /**
     * @brief The main function where all parameters of the error are set. A user needs to set only 3 parameters and then call
     * show() function. The third parameter can be an integer or string (used in thread classes).
     * @param[in] position - the position of the dialog against the widget - left, center, right
     * @param[in] EventType - defines the type of an error dialog, according to the ErrorType enum -
     * hardError, softError, info, whatToDo
     * @param[in] errorMessage - a custom statement defined by a user
     */
    void evaluate(QString position,QString EventType,QString errorMessage);

    /**
     * @brief Checks if the assigned error dialog is visible or not.
     * @return - true if the error dialog is visible
     */
    bool isEvaluated();

    /**
     * @brief Makes the error label visible. If a timer is set, the error dialog will be closed automatically.
     * @param[in] timerStop - hide the dialog automatically after defined time (ms)
     */
    void show(bool timerStop);

    /**
     * @brief Hides the error dialog completely.
     */
    void hide();

signals:
    /**
     * @brief If mouse click event is registered by an error dialog, this dialog is hidden.
     * @sa hide()
     */
    void mouseClicked();
private slots:
    /**
     * @brief Hides an error dialog icon.
     * @sa hide()
     */
    void hideErrorIcon();

private:
    /**
     * @brief Adds the hover event for the error dialog - if a user hovers over the label, signal
     * is emitted and the statement is shown.
     * @param[in] obj - error widget (error dialog)
     * @param[in] event - QEvent object
     */
    bool eventFilter(QObject *obj, QEvent *event);

    /**
     * @brief Analyses the position where the error label will be mapped to.
     * @param[in] position - left, center or right
     */
    void evaluatePosition(QString position);

    /**
     * @brief Determines the parent of a widget with an error label.
     */
    void analyseParents();

    /**
     * @brief Creates an error dialog based on the given error type.
     * @param[in] errorType - see ErrorType structure for more info
     */
    void setErrorType(ErrorDialog::ErrorType errorType);

    /**
     * @brief Initializes the error dialog widget for the parent widget.
     */
    void initialiseErrorLabel();

    /**
     * @brief Initializes the shadow of the error dialog.
     */
    void initShadowEffect();

    /**
     * @brief Initializes color effect of the error dialog.
     */
    void initColorEffect();

    /**
     * @brief Initializes the glow effect of the error dialog.
     */
    void initAnimation();

    /**
     * @brief Sets "info" type of the error dialog.
     */
    void Info();

    /**
     * @brief Sets "softError" type of the error dialog.
     */
    void SoftError();

    /**
     * @brief Sets "hardError" type of the error dialog.
     */
    void HardError();

    /**
     * @brief Sets "whatToDo" type of the error dialog.
     */
    void What_todo();

    /**
     * @brief Shows the message when the error dialog is hovered.
     * @param[in] errorMessage - showned error statement (predefined or custom)
     */
    void showMessage(const QString errorMessage);

    /**
     * @brief Adds pixmap to the error dialog. The pixmap contains the icon corresponding
     * with the error type.
     * @param[in] _pixmap
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
