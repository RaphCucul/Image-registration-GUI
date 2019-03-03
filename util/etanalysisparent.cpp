#include "etanalysisparent.h"

ETanalysisParent::ETanalysisParent(QWidget *parent) : QWidget(parent)
{

}

void ETanalysisParent::checkInputNumber(double input, double lower, double upper, QLineEdit *editWidget, double &finalValue, bool &evaluation){
    if (input < lower || (input > upper && upper != 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else if (input < lower || (upper == 0.0 && input == 0.0)){
        editWidget->setStyleSheet("color: #FF0000");
        editWidget->setText("");
        finalValue = -99;
        evaluation = false;
    }
    else{
        editWidget->setStyleSheet("color: #33aa00");
        evaluation = true;
        finalValue = input;
    }
}
