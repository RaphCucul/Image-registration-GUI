#include "main_program/tabs.h"
#include "ui_tabs.h"

#include <QTabWidget>
#include <QIcon>
#include <QStandardItem>
#include <QTransform>
#include <QPixmap>
#include <QSize>
#include <QGridLayout>

tabs::tabs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tabs)
{
    ui->setupUi(this);
    ui->pages->setTabPosition(QTabWidget::West);

    QIcon icon_Folder_png(":/images/adresar.png");
    QIcon icon_Frangi_png(":/images/frangi.png");
    QIcon icon_RegistrateTwo_png(":/images/licovaniDvou.png");
    QIcon icon_RegistrateVideo_png(":/images/video-camera-icon.png");
    QIcon icon_Graph_png(":/images/upload.png");

    ui->pages->setIconSize(QSize(32,32));
    QIcon icon_Folder = iconRotation(icon_Folder_png);
    QIcon icon_Frangi = iconRotation(icon_Frangi_png);
    QIcon icon_RegistrateTwo = iconRotation(icon_RegistrateTwo_png);
    QIcon icon_RegistrateVideo = iconRotation(icon_RegistrateVideo_png);
    QIcon icon_Graph = iconRotation(icon_Graph_png);

    chooseFolders = new DirectoriesLoader;
    frangiDetector = new Frangi_detektor;
    registrateTwo = new RegistrateTwo;
    registrateVideo = new RegistrateVideo;
    initializeGraph = new ChartInit;

    ui->pages->addTab(chooseFolders,icon_Folder,"");
    ui->pages->addTab(frangiDetector,icon_Frangi,"");
    ui->pages->addTab(registrateTwo,icon_RegistrateTwo,"");
    ui->pages->addTab(registrateVideo,icon_RegistrateVideo,"");
    ui->pages->addTab(initializeGraph,icon_Graph,"");

    if (!chooseFolders->checkFileFolderExistence()){
        disableTabs();
    }

    connect(chooseFolders,SIGNAL(fileFolderDirectoryFound()),this,SLOT(fileFolderDirectoryLocated()));
    connect(chooseFolders,SIGNAL(fileFolderDirectoryNotFound()),this,SLOT(disableTabs_slot()));

    connect(frangiDetector,SIGNAL(calculationStarted()),this,SLOT(disableTabs_slot()));
    connect(frangiDetector,SIGNAL(calculationStopped()),this,SLOT(enableTabs_slot()));

    connect(registrateTwo,SIGNAL(calculationStarted()),this,SLOT(disableTabs_slot()));
    connect(registrateTwo,SIGNAL(calculationStopped()),this,SLOT(enableTabs_slot()));

    connect(registrateVideo,SIGNAL(calculationStarted()),this,SLOT(disableTabs_slot()));
    connect(registrateVideo,SIGNAL(calculationStopped()),this,SLOT(enableTabs_slot()));
}

QIcon tabs::iconRotation(QIcon i_icon)
{
    QTransform trans;
    QSize sz(32,32);
    QPixmap pix = i_icon.pixmap(sz);
    QIcon icon = QIcon(pix);
    trans.rotate(+90);
    pix = pix.transformed(trans);
    icon = QIcon(pix);
    return icon;
}

tabs::~tabs()
{
    delete ui;
}

void tabs::enableTabs_slot(){
    enableTabs();
}

void tabs::enableTabs(){
    for (int var = 0; var < ui->pages->tabBar()->count(); var++) {
        if (var != ui->pages->currentIndex())
            ui->pages->setTabEnabled(var,true);
    }
}

void tabs::disableTabs_slot(){
    disableTabs();
}

void tabs::disableTabs(){
    for (int var = 0; var < ui->pages->tabBar()->count(); var++) {
        if (var != ui->pages->currentIndex())
            ui->pages->setTabEnabled(var,false);
    }
}

void tabs::default_disabled(){
    ui->pages->setTabEnabled(2,false);
    ui->pages->setTabEnabled(3,false);
}

void tabs::fileFolderDirectoryLocated()
{
    enableTabs();
}
void tabs::on_pages_tabBarClicked(int i_index)
{
    if (i_index == 1){
        frangiDetector->checkPaths();
    }
    if (i_index == 2){
        registrateTwo->checkPaths();
    }
    if (i_index == 3){
        registrateVideo->checkPathinitialization();
    }
}
