#include "dialogs/errordialog.h"
#include "ui_errordialog.h"
#include "shared_staff/errors.h"

#include <QtGui>
#include <QPropertyAnimation>
#include <QEvent>

ErrorDialog::ErrorDialog(QWidget *widgetWithNotification, QWidget *parent) :
    QObject(parent)
{
    widgetWithError = widgetWithNotification;
    connect(this,SIGNAL(mouseClicked()),this,SLOT(hideErrorIcon()));
}

ErrorDialog::~ErrorDialog()
{

}

void ErrorDialog::initialiseErrorLabel()
{
    errorLabel = new QLabel();
    errorLabel->setMargin(0);
    errorLabel->setGeometry(0,0,16,16);
    errorLabel->setAttribute(Qt::WA_Hover,true);
    errorLabel->installEventFilter(this);

    errorLabelParent = new QWidget(parentOfTheWidget);
    errorLabelParent->setParent(parentOfTheWidget);
    errorLabelParent->setLayout(new QHBoxLayout());
    errorLabelParent->layout()->setMargin(0);
    errorLabelParent->layout()->setContentsMargins(0,0,0,0);
    errorLabelParent->layout()->addWidget(errorLabel);
}

void ErrorDialog::analyseParents()
{
    parentOfTheWidget = widgetWithError->parentWidget();

    initialiseErrorLabel();
    initColorEffect();
    initShadowEffect();
    initAnimation();
}

void ErrorDialog::evaluate(QString position, QString EventType, int errorNumber)
{
    analyseParents();
    evaluatePosition(position);
    if (EventType == "info"){
        showMessage(infoList[errorNumber].toString());
        Info();
    }
    else if (EventType == "softError"){
        showMessage(softErrorList[errorNumber].toString());
        SoftError();
    }
    else if (EventType == "hardError"){
        showMessage(hardErrorList[errorNumber].toString());
        HardError();
    }
    else {
        showMessage(whatToDoList[errorNumber].toString());
        What_todo();
    }
    errorLabelinitialized = true;
}

void ErrorDialog::evaluate(QString position, QString EventType, QString errorMessage){
    analyseParents();
    evaluatePosition(position);
    if (EventType == "info"){
        showMessage(errorMessage);
        Info();
    }
    else if (EventType == "softError"){
        showMessage(errorMessage);
        SoftError();
    }
    else if (EventType == "hardError"){
        showMessage(errorMessage);
        HardError();
    }
    else{
        showMessage(errorMessage);
        What_todo();
    }
    errorLabelinitialized = true;
}

bool ErrorDialog::isEvaluated()
{
    return errorLabelinitialized;
}

void ErrorDialog::evaluatePosition(QString position)
{    
    if (position == "left"){
        QPoint mappingCoordinates = widgetWithError->mapTo(parentOfTheWidget,widgetWithError->rect().topLeft());
        errorLabelParent->move(mappingCoordinates + QPoint(-6, -4));
        qDebug()<<"Map coords "<<mappingCoordinates<<" "<<widgetWithError->rect().topLeft();
    }
    if (position == "right"){
        int widgetWidth = widgetWithError->width();
        QPoint mappingCoordinates = widgetWithError->mapTo(parentOfTheWidget,QPoint(widgetWidth,0));
        errorLabelParent->move(mappingCoordinates + QPoint(0, -6));
        qDebug()<<"Map coords "<<mappingCoordinates;
    }
    if (position == "center"){
        int widgetWidth = widgetWithError->width();
        //QPoint widgetWidth = widgetWithError->rect().topRight();
        qDebug()<<widgetWidth;
        QPoint mappingCoordinates = widgetWithError->mapTo(parentOfTheWidget,QPoint(widgetWidth/2,0));
        errorLabelParent->move(mappingCoordinates + QPoint(0, -6));
        qDebug()<<"Map coords "<<mappingCoordinates;
    }

}

void ErrorDialog::fillErrorLabel(QPixmap& _pixmap){
    errorLabel->setFixedSize(errorLabel->pixmap()->size());
    errorLabelParent->setFixedSize(errorLabel->pixmap()->size());
    errorLabel->setMask(_pixmap.mask());
    errorLabelParent->setMask(_pixmap.mask());
}

void ErrorDialog::setErrorType(ErrorDialog::ErrorType errorType)
{
    this->errorType = errorType;
    if (errorType == ErrorType::INFO){
        QPixmap pixInfo(":/images/circularInfo16_2.png");
        errorLabel->setPixmap(pixInfo);
        highlightingEffect->setColor(Qt::blue);
        glowEffect->setColor(Qt::blue);
        fillErrorLabel(pixInfo);
    }
    else if(errorType == ErrorType::SOFT_ERROR){
        QPixmap pixSE(":/images/circularSoftError16_2.png");
        errorLabel->setPixmap(pixSE);
        highlightingEffect->setColor(Qt::yellow);
        glowEffect->setColor(Qt::yellow);
        fillErrorLabel(pixSE);
    }
    else if(errorType == ErrorType::HARD_ERROR){
        QPixmap pixHE(":/images/circularHardError16_2.png");
        highlightingEffect->setColor(Qt::red);
        glowEffect->setColor(Qt::red);
        errorLabel->setPixmap(pixHE);
        fillErrorLabel(pixHE);
    }
    else{
        QPixmap pixWTD(":/images/What_todo.png");
        errorLabel->setPixmap(pixWTD);
        highlightingEffect->setColor(Qt::green);
        glowEffect->setColor(Qt::green);
        errorLabel->setPixmap(pixWTD);
        fillErrorLabel(pixWTD);
    }
}

void ErrorDialog::initColorEffect()
{
    highlightingEffect = new QGraphicsColorizeEffect();
    highlightingEffect->setStrength(0.2);
    highlightingEffect->setEnabled(true);
}

void ErrorDialog::initShadowEffect()
{
    glowEffect = new QGraphicsDropShadowEffect();
    glowEffect->setBlurRadius(10.0);
    glowEffect->setOffset(0.0);
    glowEffect->setEnabled(true);
}

void ErrorDialog::initAnimation()
{
    animation = new QSequentialAnimationGroup(this);
    animation->setLoopCount(-1);

    // Animation of glow efect
    QPropertyAnimation* varAnim = new QPropertyAnimation(glowEffect, "blurRadius");
    varAnim->setStartValue(3.0);
    varAnim->setEndValue(14.0);
    varAnim->setEasingCurve(QEasingCurve::InOutCubic);
    varAnim->setDuration(300);
    animation->addAnimation(varAnim);

    varAnim = new QPropertyAnimation(glowEffect, "blurRadius");
    varAnim->setStartValue(14.0);
    varAnim->setEndValue(3.0);
    varAnim->setEasingCurve(QEasingCurve::InOutCubic);
    varAnim->setDuration(300);
    animation->addAnimation(varAnim);
}
void ErrorDialog::Info()
{
    setErrorType(ErrorType::INFO);
}
void ErrorDialog::SoftError()
{
    setErrorType(ErrorType::SOFT_ERROR);
}
void ErrorDialog::HardError()
{
    setErrorType(ErrorType::HARD_ERROR);
}
void ErrorDialog::What_todo()
{
    setErrorType(ErrorType::WHAT_TODO);
}

bool ErrorDialog::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    if (event->type() == QEvent::MouseButtonPress)
        {
            emit mouseClicked();
            //qDebug() << "Click event";
        }
    return false;
}

void ErrorDialog::showMessage(const QString errorMessage)
{
    errorLabel->setToolTip(errorMessage);
}

void ErrorDialog::show(bool timerStop)
{
    errorLabelParent->setVisible(true);
    errorLabel->setGraphicsEffect(highlightingEffect);
    errorLabelParent->setGraphicsEffect(glowEffect);
    if (animation->state() != QAbstractAnimation::Running)
        animation->start();
    if (timerStop){
        timer->setTimerType(Qt::PreciseTimer);
        connect(timer, SIGNAL(timeout()), this, SLOT(hideErrorIcon()));
        timer->start(20000);
    }
}

void ErrorDialog::hideErrorIcon(){
    hide();
}
void ErrorDialog::hide()
{
    errorLabelParent->setVisible(false);
    if (animation->state() == QAbstractAnimation::Running)
        animation->stop();
    errorLabelinitialized = false;
}
