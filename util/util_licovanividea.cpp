#include "util/util_licovanividea.h"
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSignalMapper>
#include <QLabel>
#include <QFileDialog>
#include <QSpacerItem>

void ET_SingleVideoAnalysis(QGridLayout* CB, QGridLayout *VV, QGridLayout *Met)
{
    QLineEdit* vybraneVideoLE = new QLineEdit;
    vybraneVideoLE->setPlaceholderText("Vybrané video");    
    QPushButton* vyberVideaPB = new QPushButton;
    vyberVideaPB->setText("Výběr videa");
    QPushButton* ETPB = new QPushButton;
    ETPB->setText("Entropie a tennengrad");
    ETPB->setEnabled(false);
    QPushButton* ZobrazitVysledky = new QPushButton;
    ZobrazitVysledky->setText("Zobraz graf");
    ZobrazitVysledky->setEnabled(false);
    QCheckBox* pritomnostCasoveZnackyCB = new QCheckBox;
    pritomnostCasoveZnackyCB->setText("Přítomnost časové značky");
    pritomnostCasoveZnackyCB->setEnabled(false);
    QCheckBox* pritomnostSvetelneAnomalieCB = new QCheckBox;
    pritomnostSvetelneAnomalieCB->setText("Přítomnost světelné značky");
    pritomnostSvetelneAnomalieCB->setEnabled(false);
    QSpacerItem* horizontalSpacer1 = new QSpacerItem(5,10);
    QSpacerItem* horizontalSpacer2 = new QSpacerItem(5,5);
    QSpacerItem* horizontalSpacer3 = new QSpacerItem(5,5);
    //QSpacerItem* horizontalSpacer4 = new QSpacerItem(5,3);
    QSpacerItem* horizontalSpacer5 = new QSpacerItem(5,15);
    QSpacerItem* horizontalSpacer6 = new QSpacerItem(5,15);
    QSpacerItem* horizontalSpacer7 = new QSpacerItem(5,15);
    QSpacerItem* horizontalSpacer8 = new QSpacerItem(5,15);
    ///    
    CB->addWidget(pritomnostCasoveZnackyCB,0,0);
    CB->addItem(horizontalSpacer1,0,1);
    CB->addWidget(pritomnostSvetelneAnomalieCB,0,2);
    ///
    VV->addItem(horizontalSpacer2,0,0);
    VV->addWidget(vybraneVideoLE,0,1);
    VV->addItem(horizontalSpacer3,0,2);
    VV->addWidget(vyberVideaPB,0,3);
    ///
    Met->addItem(horizontalSpacer5,0,0);
    Met->addWidget(ETPB,0,1);
    Met->addItem(horizontalSpacer6,0,2);
    Met->addItem(horizontalSpacer7,1,0);
    Met->addWidget(ZobrazitVysledky,1,1);
    Met->addItem(horizontalSpacer8,1,2);
}
