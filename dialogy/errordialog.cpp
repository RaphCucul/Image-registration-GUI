#include "dialogy/errordialog.h"
#include "ui_errordialog.h"

#include <QtGui>

ErrorDialog::ErrorDialog(QWidget *parent) :
    QObject(parent)
{

}

ErrorDialog::~ErrorDialog()
{

}

void ErrorDialog::initialiseErrorLabel()
{
    /*errorLabelEffect = new QGraphicsColorizeEffect();
    errorLabelEffect->setColor(Qt::white);
    errorLabelEffect->setStrength(0.3);
    errorLabelEffect->setEnabled(false);*/
    errorLabel = new QLabel();
    errorLabel->setMargin(0);
    //errorLabel->installEventFilter(this);
   // errorLabel->setGraphicsEffect(errorLabelEffect);

    errorLabelParent = new QWidget(parentOfTheWidget);
    errorLabelParent->setParent(parentOfTheWidget);
    errorLabelParent->setLayout(new QHBoxLayout());
    errorLabelParent->layout()->setMargin(0);
    errorLabelParent->layout()->addWidget(errorLabel);
    errorLabelParent->setVisible(true);
}

void ErrorDialog::analyseParents(QWidget *widgetWithLabel)
{
    widgetWithError = widgetWithLabel;
    parentOfTheWidget = widgetWithLabel->parentWidget();

}

void ErrorDialog::evaluatePosition()
{
    //QRect widgetGeometry = widgetWithError->geometry();
    QPoint mappingCoordinates = widgetWithError->mapTo(parentOfTheWidget,QPoint(0,0));
    //QPoint mappingCoordinates = widgetWithError->mapToGlobal(widgetWithError->rect().topLeft());
    //QPoint coordsOfWidget = widgetWithError->mapFromParent(QPoint(0,0));
    //qDebug()<<coordsOfWidget;
    //qDebug()<<mappingCoordinates;
    errorLabelParent->move(mappingCoordinates + QPoint(-4, -4));
}

void ErrorDialog::setErrorType(ErrorDialog::ErrorType errorType)
{
    this->errorType = errorType;
    if (errorType == ErrorType::INFO){
        QPixmap pixInfo(":/images/Info.png");
        errorLabel->setPixmap(pixInfo);
    }
    else if(errorType == ErrorType::SOFT_ERROR){
        QPixmap pixSE(":/images/softError24_2.png");
        errorLabel->setPixmap(pixSE);
    }
    else if(errorType == ErrorType::HARD_ERROR){
        QPixmap pixHE(":/images/hardError24_2.png");
        errorLabel->setPixmap(pixHE);
    }
    else{
        QPixmap pixWTD(":/images/What_todo.png");
        errorLabel->setPixmap(pixWTD);
    }
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
/*void ErrorDialog::createInfoDialog(QString &text, int typeOfEvent)
{
    errorStatementToDisplay = text;
    errorSeverity = typeOfEvent;
    ui->textError->setText(text);
    if (typeOfEvent == 0) // soft error
    {
        QIcon soft(":/images/softError.png");
        setWindowIcon(soft);
        ui->tlacitko1->setEnabled(true);
        ui->tlacitko1->setText(tr("OK"));
    }
    if (typeOfEvent == 1){ // hard error
        QIcon hard(":/images/hardError.png");
        setWindowIcon(hard);
        ui->tlacitko1->setEnabled(true);
        ui->tlacitko2->setEnabled(true);
        ui->tlacitko1->setText(tr("Terminate"));
        ui->tlacitko2->setText(tr("Continue"));
    }
}

void ErrorDialog::on_tlacitko2_clicked()
{
    emit continueInCalculations();
    close();
}

void ErrorDialog::on_tlacitko1_clicked()
{
    if (errorSeverity == 0)
        emit ok();
    else
        emit abort();
    close();
}*/
