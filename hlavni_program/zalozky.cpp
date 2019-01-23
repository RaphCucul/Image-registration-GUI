#include "hlavni_program/zalozky.h"
#include "ui_zalozky.h"

#include <QTabWidget>
#include <QIcon>
#include <QStandardItem>
#include <QTransform>
#include <QPixmap>
#include <QSize>
#include <QGridLayout>

zalozky::zalozky(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::zalozky)
{
    ui->setupUi(this);
    ui->stranky->setTabPosition(QTabWidget::West);
    //ui->stranky->setStyleSheet("QTabBar::tab {height: 32px;width: 32px;padding-top:0px;padding-bottom:,0px}");
    //ui->stranky->setStyleSheet("background-color: white");

    QIcon ikonka_slozky(":/images/adresar.png");
    QIcon ikonka_frangi(":/images/frangi.png");
    QIcon ikonka_licovaniDvou(":/images/licovaniDvou.png");
    QIcon ikonka_licovaniVidea(":/images/video-camera-icon.png");

    ui->stranky->setIconSize(QSize(32,32));
    QIcon icon_slozky = rotace_ikonky(ikonka_slozky);
    QIcon icon_frangi = rotace_ikonky(ikonka_frangi);
    QIcon icon_licovaniDvou = rotace_ikonky(ikonka_licovaniDvou);
    QIcon icon_licovaniVidea = rotace_ikonky(ikonka_licovaniVidea);

    volba_slozek = new t_b_HO;
    detektor = new Frangi_detektor;
    licovaniDvou = new LicovaniDvou;
    licovaniVidea = new LicovaniVidea;

    ui->stranky->addTab(volba_slozek,icon_slozky,"");
    ui->stranky->addTab(detektor,icon_frangi,"");    
    ui->stranky->addTab(licovaniDvou,icon_licovaniDvou,"");
    ui->stranky->addTab(licovaniVidea,icon_licovaniVidea,"");

    bool folderPresent = volba_slozek->checkFileFolderExistence();
    if (!folderPresent){
        ui->stranky->setTabEnabled(1,false);
        ui->stranky->setTabEnabled(2,false);
        ui->stranky->setTabEnabled(3,false);
    }

    connect(volba_slozek,SIGNAL(fileFolderDirectoryFound()),this,SLOT(fileFolderDirectoryLocated()));
}

QIcon zalozky::rotace_ikonky(QIcon ikona_vstup)
{
    QTransform trans;
    QSize sz(32,32);
    QPixmap pix = ikona_vstup.pixmap(sz);
    QIcon icon = QIcon(pix);
    trans.rotate(+90);
    pix = pix.transformed(trans);
    icon = QIcon(pix);
    return icon;
}

zalozky::~zalozky()
{
    delete ui;
}

void zalozky::fileFolderDirectoryLocated()
{
    ui->stranky->setTabEnabled(1,true);
    ui->stranky->setTabEnabled(2,true);
    ui->stranky->setTabEnabled(3,true);
}
void zalozky::on_stranky_tabBarClicked(int index)
{
    if (index == 1){
        detektor->checkPaths();
    }
    if (index == 2){
        licovaniDvou->checkPaths();
    }
}
