#include "licovanidvou.h"
#include "ui_licovanidvou.h"
#include "util/upravy_obrazu.h"
#include "util/pouzij_frangiho.h"
#include "util/entropie.h"
#include "licovani/multiPOC_Ai1.h"
#include "licovani/korekce_zapis.h"
#include "licovani/fazova_korelace_funkce.h"
#include "dialogy/errordialog.h"
#include "dialogy/clickimageevent.h"
#include "dialogy/frangi_detektor.h"
#include "dialogy/vysledeklicovanidvou.h"

#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <QFileDialog>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QtGui>
#include <QPixmap>
#include <QEvent>
#include <QMouseEvent>
#include <QPointF>
#include <QSpacerItem>
#include <array>
#include <QSignalMapper>

/*static int slider_position=0;
static int alpha_slider;*/
static cv::Mat src1;
static cv::Mat src2;


/*static void on_trackbar( int, void* )
{
    if(slider_position == 0){cv::imshow("Kontrola_licovani",src1);}
    else {cv::imshow("Kontrola_licovani",src2);}
}*/

LicovaniDvou::LicovaniDvou(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicovaniDvou)
{
    ui->setupUi(this);
    /***********************************************************************************/
    // nastavení výchozích parametrů pro lícování
    ui->oblastMaxima->setText("10");
    oblastMaxima = 10.0;
    ui->uhelRotace->setText("0.1");
    uhel = 0.1;
    ui->pocetIteraci->setText("-1");
    iterace = -1;
    ui->casovaZnacka->setChecked(false);
    ui->anomalie->setChecked(false);
    /***********************************************************************************/
    // nastavení parametrů pro frangiho detektor - buď soubor existuje, nebo neexistuje
    // a zvolí se výchozí parametry pro algoritmus
    maximum_frangi = detekovane_frangiho_maximum;
    connect(ui->anomalie,SIGNAL(stateChanged(int)),this,SLOT(zobrazKliknutelnyDialog()));
    velikost_frangi_opt(6);
    QFile soubor(QDir::currentPath()+"/"+"frangi_parametry.txt");
    //bool soubor_pritomen = false;
    if (soubor.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        //soubor_pritomen = true;
        QTextStream in(&soubor);
        int pocitadlo = 0;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            inicializace_frangi_opt(line,pocitadlo);
            pocitadlo+=1;
        }
    }
    else
    {
        parametry_frangi[0] = 1;
        parametry_frangi[1] = 10;
        parametry_frangi[2] = 1;
        parametry_frangi[3] = 8.0;
        parametry_frangi[4] = 8.0;
        parametry_frangi[5] = 1;
    }
    /***********************************************************************************/
    // protože se pracuje s layouty a comboboxy, je potřeba definovat defaultní zobrazení
    // nabídky a odpovídajících widgetů
    // první v nabídce je lícování dvou snímků -> layout bude zaplněn prvky pro tuto nabídku
    QLineEdit* vybraneVideoLE = new QLineEdit();
    QPushButton* vyberVideaPB = new QPushButton();
    QLineEdit* cisloReferenceLE = new QLineEdit();
    QLineEdit* cisloPosunutehoLE = new QLineEdit();
    QSpacerItem* horizontalSpacer1 = new QSpacerItem(20,20);
    /*******************************************************/
    vybraneVideoLE->setPlaceholderText("Vybrané video");
    vybraneVideoLE->setMinimumWidth(110);
    vybraneVideoLE->setMinimumHeight(20);    
    vyberVideaPB->setText("Výběr videa");
    vyberVideaPB->setMinimumWidth(71);
    vyberVideaPB->setMinimumHeight(23);
    cisloReferenceLE->setPlaceholderText("Ref");
    cisloReferenceLE->setMinimumWidth(31);
    cisloReferenceLE->setMinimumHeight(20);
    cisloPosunutehoLE->setPlaceholderText("Posun");
    cisloPosunutehoLE->setMinimumWidth(41);
    cisloPosunutehoLE->setMinimumHeight(20);
    /*******************************************************/
    ui->nabidkaAnalyzy->addWidget(vybraneVideoLE,0,0);
    ui->nabidkaAnalyzy->addWidget(vyberVideaPB,0,1);
    ui->nabidkaAnalyzy->addWidget(cisloReferenceLE,0,2);
    ui->nabidkaAnalyzy->addWidget(cisloPosunutehoLE,0,3);
    ui->nabidkaAnalyzy->addItem(horizontalSpacer1,0,4);
    /*******************************************************/
    QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
    QPushButton* VideoPB = qobject_cast<QPushButton*>(widgetVideoPB);
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    QLineEdit* VideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    QSignalMapper* m_sigmapper = new QSignalMapper(this);
    connect(VideoPB,SIGNAL(clicked()),m_sigmapper,SLOT(map()));
    m_sigmapper->setMapping(VideoPB,VideoLE);
    QObject::connect(m_sigmapper, SIGNAL(mapped(QWidget *)),this, SLOT(vyberVideaPB_clicked(QWidget *)));
    QObject::connect(widgetVideoLE,SIGNAL(textChanged(const QString &)),this,
                     SLOT(Slot_VideoLE_textChanged(const QString&)));
    /********************************************************/
    QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
    QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
    QWidget* widgetPosunutehoLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
    QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutehoLE);
    QObject::connect(ReferenceLE,SIGNAL(textChanged(const QString &)),
                     this,SLOT(ReferenceLE_textChanged(const QString&)));
    QObject::connect(PosunutyLE,SIGNAL(textChanged(const QString &)),
                     this,SLOT(PosunutyLE_textChanged(const QString&)));
}

LicovaniDvou::~LicovaniDvou()
{
    delete ui;
}

void LicovaniDvou::velikost_frangi_opt(int velikost){
    parametry_frangi = (QVector<double>(velikost));
}

void LicovaniDvou::inicializace_frangi_opt(QString &hodnota, int &pozice)
{
    parametry_frangi[pozice] = hodnota.toDouble();
}
void LicovaniDvou::on_comboBox_activated(int index)
{
    qDebug()<<"Index Comboboxu: "<<index;
    if (index == 2)
    {
        if (predchozi_index != 2)
            clearLayout(ui->nabidkaAnalyzy);

        predchozi_index = 2;
    }
    if (index == 0)
    {
        if (predchozi_index != 2)
            clearLayout(ui->nabidkaAnalyzy);

        predchozi_index = 0;
        QLineEdit* vybraneVideoLE = new QLineEdit(this);
        QPushButton* vyberVideaPB = new QPushButton(this);
        QLineEdit* cisloReferenceLE = new QLineEdit(this);
        QLineEdit* cisloPosunutehoLE = new QLineEdit(this);
        QSpacerItem* horizontalSpacer1 = new QSpacerItem(20,20);        
        /**************************************************/
        vybraneVideoLE->setPlaceholderText("Vybrané video");
        vybraneVideoLE->setMinimumWidth(110);
        vybraneVideoLE->setMinimumHeight(20);
        vyberVideaPB->setText("Výběr videa");
        vyberVideaPB->setMinimumWidth(71);
        vyberVideaPB->setMinimumHeight(23);
        cisloReferenceLE->setPlaceholderText("Ref");
        cisloReferenceLE->setMinimumWidth(31);
        cisloReferenceLE->setMinimumHeight(20);
        cisloPosunutehoLE->setPlaceholderText("Posun");
        cisloPosunutehoLE->setMinimumWidth(41);
        cisloPosunutehoLE->setMinimumHeight(20);
        /**************************************************/
        ui->nabidkaAnalyzy->addWidget(vybraneVideoLE,0,0);
        ui->nabidkaAnalyzy->addWidget(vyberVideaPB,0,1);
        ui->nabidkaAnalyzy->addWidget(cisloReferenceLE,0,2);
        ui->nabidkaAnalyzy->addWidget(cisloPosunutehoLE,0,3);
        ui->nabidkaAnalyzy->addItem(horizontalSpacer1,0,4);
        /**************************************************/
        QWidget* widgetVideoPB = ui->nabidkaAnalyzy->itemAt(1)->widget();
        QPushButton* VideoPB = qobject_cast<QPushButton*>(widgetVideoPB);
        //QWidget *widget = qobject_cast<QWidget *>(obj);
        QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
        QLineEdit* VideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
        /// Nelze, paměť je prázdná a indexace končí chybou
        //VideoLE->setText(rozborVybranehoSouboru[1]);
        /// Řešení s využitím emit funguje, ale opět je to vše limitováno místem tvorby LineEditu
        //QObject::connect(VideoPB,SIGNAL(clicked()),this,SLOT(vyberVideaPBWrapper()));
        /// Řešení se QSignalMapper konečně funkční, jen je třeba uvnitř funkce castit QLineEdit
        QSignalMapper* m_sigmapper = new QSignalMapper(this);
        connect(VideoPB,SIGNAL(clicked()),m_sigmapper,SLOT(map()));
        m_sigmapper->setMapping(VideoPB,VideoLE); // je jedno, jestli je tam VideoLE nebo widgetVideoLE
        // funguje obojí, ale do connectu nacpu stejně jen QWidget*, nikoliv QLineEdit*
        QObject::connect(m_sigmapper, SIGNAL(mapped(QWidget *)),this, SLOT(vyberVideaPB_clicked(QWidget *)));
        QObject::connect(widgetVideoLE,SIGNAL(textChanged(const QString &)),this,SLOT(Slot_VideoLE_textChanged(const QString&)));
        /**************************************************/
        QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
        QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
        QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
        QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
        QObject::connect(ReferenceLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(ReferenceLE_textChanged(const QString&)));
        QObject::connect(PosunutyLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(PosunutyLE_textChanged(const QString&)));
}
    else if (index == 1)
    {
        if (predchozi_index != 2)
            clearLayout(ui->nabidkaAnalyzy);

        predchozi_index = 1;
        QSpacerItem* horizontalSpacer2 = new QSpacerItem(5,20);
        QLineEdit* volbaReferenceLE = new QLineEdit();
        QLineEdit* volbaPosunutehoLE = new QLineEdit();
        QPushButton* vyberObrazekReferencePB = new QPushButton();
        QPushButton* vyberObrazekPosunutyPB = new QPushButton();
        /*******************************************************/
        volbaReferenceLE->setMinimumWidth(20);
        volbaReferenceLE->setMinimumHeight(20);
        volbaReferenceLE->setPlaceholderText("Ref");
        volbaPosunutehoLE->setMinimumWidth(20);
        volbaPosunutehoLE->setMinimumHeight(20);
        volbaPosunutehoLE->setPlaceholderText("Pos");
        vyberObrazekReferencePB->setText("Výběr reference");
        vyberObrazekReferencePB->setMinimumWidth(90);
        vyberObrazekReferencePB->setMinimumHeight(23);
        vyberObrazekPosunutyPB->setText("Výběr posunutého");
        vyberObrazekPosunutyPB->setMinimumWidth(90);
        vyberObrazekPosunutyPB->setMinimumHeight(23);
        /***************************************************/
        ui->nabidkaAnalyzy->addWidget(volbaReferenceLE,0,0);
        ui->nabidkaAnalyzy->addWidget(vyberObrazekReferencePB,0,1);        
        ui->nabidkaAnalyzy->addWidget(volbaPosunutehoLE,0,2);
        ui->nabidkaAnalyzy->addWidget(vyberObrazekPosunutyPB,0,3);
        ui->nabidkaAnalyzy->addItem(horizontalSpacer2,0,4);
        /***************************************************/
        QWidget* widgetReferencePB = ui->nabidkaAnalyzy->itemAt(1)->widget();
        QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
        QWidget* widgetPosunutyPB = ui->nabidkaAnalyzy->itemAt(3)->widget();
        QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
        QPushButton* ReferencePB = qobject_cast<QPushButton*>(widgetReferencePB);
        QPushButton* PosunutyPB = qobject_cast<QPushButton*>(widgetPosunutyPB);
        QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
        QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);

        QSignalMapper* m_sigmapper1 = new QSignalMapper(this);

        connect(ReferencePB,SIGNAL(clicked()),m_sigmapper1,SLOT(map()));
        m_sigmapper1->setMapping(ReferencePB,ReferenceLE);
        QObject::connect(m_sigmapper1, SIGNAL(mapped(QWidget *)),this, SLOT(vyberReferenceObrPB_clicked(QWidget *)));

        QSignalMapper* m_sigmapper2 = new QSignalMapper(this);
        connect(PosunutyPB,SIGNAL(clicked()),m_sigmapper2,SLOT(map()));
        m_sigmapper2->setMapping(PosunutyPB,PosunutyLE);
        QObject::connect(m_sigmapper2, SIGNAL(mapped(QWidget *)),this, SLOT(vyberPosunutehoObrPB_clicked(QWidget *)));

        QObject::connect(ReferenceLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(ReferenceObrLE_textChanged(const QString&)));
        QObject::connect(PosunutyLE,SIGNAL(textChanged(const QString &)),
                         this,SLOT(PosunutyObrLE_textChanged(const QString&)));

    }
}

void LicovaniDvou::clearLayout(QGridLayout *layout)
{
    int pocetSLoupcu = layout->columnCount();
    int pocetRadku = layout->rowCount();
    qDebug()<<"pocetSLoupcu: "<<pocetSLoupcu<<" pocetRadku: "<<pocetRadku;
    for (int a = 1; a <= pocetRadku; a++)
    {
        for (int b = 1; b <= pocetSLoupcu; b++)
        {
            QWidget* widget = layout->itemAtPosition(a-1,b-1)->widget();
            layout->removeWidget(widget);
            delete widget;
        }
    }
}

void LicovaniDvou::vyberVideaPBWrapper()
{
    //vyberVideaPB_clicked(dynamic_cast<QLineEdit*>(sender()));
    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    QLineEdit* VideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    emit vyberVideaPB_clicked(VideoLE);
}

void LicovaniDvou::vyberVideaPB_clicked(QWidget *W)
{
    QString videoProDvojiciSnimku = QFileDialog::getOpenFileName(this,
         "Vyberte snímek pro Frangiho filtr", "","*.avi;;Všechny soubory (*)");
    //qDebug()<<"vybrane video: "<<videoProDvojiciSnimku;
    int lastindexSlash = videoProDvojiciSnimku.lastIndexOf("/");
    int lastIndexComma = videoProDvojiciSnimku.length() - videoProDvojiciSnimku.lastIndexOf(".");
    QString vybrana_slozka = videoProDvojiciSnimku.left(lastindexSlash);
    QString vybrany_soubor = videoProDvojiciSnimku.mid(lastindexSlash+1,
         (videoProDvojiciSnimku.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = videoProDvojiciSnimku.right(lastIndexComma-1);
    if (rozborVybranehoSouboru.length() == 0)
    {
        rozborVybranehoSouboru.push_back(vybrana_slozka);
        rozborVybranehoSouboru.push_back(vybrany_soubor);
        rozborVybranehoSouboru.push_back(koncovka);
    }
    else
    {
        rozborVybranehoSouboru.clear();
        rozborVybranehoSouboru.push_back(vybrana_slozka);
        rozborVybranehoSouboru.push_back(vybrany_soubor);
        rozborVybranehoSouboru.push_back(koncovka);
    }
    //QLineEdit cislo_snimku;
    //vybrane_video = rozborVybranehoSouboru[1];
    /*QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    QLineEdit* VideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);*/
    QLineEdit* VideoLE = qobject_cast<QLineEdit*>(W);
    //qDebug()<<rozborVybranehoSouboru[0];
    //qDebug()<<rozborVybranehoSouboru[1];
    //qDebug()<<rozborVybranehoSouboru[2];
    VideoLE->setText(rozborVybranehoSouboru[1]);
    QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+rozborVybranehoSouboru[1]+"."+rozborVybranehoSouboru[2];
    cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        VideoLE->setStyleSheet("QLineEdit#VideoLE{color: #FF0000}");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVidea = false;
    }
    else
    {
        VideoLE->setStyleSheet("color: #339900");
        spravnostVidea = true;
    }
}

void LicovaniDvou::vyberReferenceObrPB_clicked(QWidget* W)
{
    QString referencniObrazek = QFileDialog::getOpenFileName(this,
         "Vyberte referenční obrázek", "","*.bmp;;Všechny soubory (*)");
    int lastindexSlash = referencniObrazek.lastIndexOf("/");
    int lastIndexComma = referencniObrazek.length() - referencniObrazek.lastIndexOf(".");
    QString vybrana_slozka = referencniObrazek.left(lastindexSlash);
    QString vybrany_soubor = referencniObrazek.mid(lastindexSlash+1,
         (referencniObrazek.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = referencniObrazek.right(lastIndexComma-1);
    if (rozborObrReference.length() == 0)
    {
        rozborObrReference.push_back(vybrana_slozka);
        rozborObrReference.push_back(vybrany_soubor);
        rozborObrReference.push_back(koncovka);
    }
    else
    {
        rozborObrReference.clear();
        rozborObrReference.push_back(vybrana_slozka);
        rozborObrReference.push_back(vybrany_soubor);
        rozborObrReference.push_back(koncovka);
    }
    QLineEdit* ReferenceObrLE = qobject_cast<QLineEdit*>(W);
    ReferenceObrLE->setText(rozborObrReference[1]);
    QString kompletni_cesta = rozborObrReference[0]+"/"+rozborObrReference[1]+"."+rozborObrReference[2];
    cv::Mat referencniObr;
    referencniObr = cv::imread(kompletni_cesta.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
    if (referencniObr.empty())
    {
        ReferenceObrLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Obrazek zvoleny jako referenční nelze otevřít.";
        spravnostReferenceObr = false;
    }
    else
    {
        ReferenceObrLE->setStyleSheet("color: #339900");
        spravnostReferenceObr = true;
    }
    referencniObr.release();
}

void LicovaniDvou::vyberPosunutehoObrPB_clicked(QWidget* W)
{
    QString posunutyObrazek = QFileDialog::getOpenFileName(this,
         "Vyberte posunutý obrázek", "","*.bmp;;Všechny soubory (*)");
    int lastindexSlash = posunutyObrazek.lastIndexOf("/");
    int lastIndexComma = posunutyObrazek.length() - posunutyObrazek.lastIndexOf(".");
    QString vybrana_slozka = posunutyObrazek.left(lastindexSlash);
    QString vybrany_soubor = posunutyObrazek.mid(lastindexSlash+1,
         (posunutyObrazek.length()-lastindexSlash-lastIndexComma-1));
    QString koncovka = posunutyObrazek.right(lastIndexComma-1);
    if (rozborObrPosunuty.length() == 0)
    {
        rozborObrPosunuty.push_back(vybrana_slozka);
        rozborObrPosunuty.push_back(vybrany_soubor);
        rozborObrPosunuty.push_back(koncovka);
    }
    else
    {
        rozborObrPosunuty.clear();
        rozborObrPosunuty.push_back(vybrana_slozka);
        rozborObrPosunuty.push_back(vybrany_soubor);
        rozborObrPosunuty.push_back(koncovka);
    }
    QLineEdit* PosunutyObrLE = qobject_cast<QLineEdit*>(W);
    PosunutyObrLE->setText(rozborObrPosunuty[1]);
    QString kompletni_cesta = rozborObrPosunuty[0]+"/"+rozborObrPosunuty[1]+"."+rozborObrPosunuty[2];
    cv::Mat posunutyObr;
    posunutyObr = cv::imread(kompletni_cesta.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
    if (posunutyObr.empty())
    {
        PosunutyObrLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Obrazek zvoleny jako posunutý nelze otevřít.";
        spravnostReferenceObr = false;
    }
    else
    {
        PosunutyObrLE->setStyleSheet("color: #339900");
        spravnostReferenceObr = true;
    }
    posunutyObr.release();
}

void LicovaniDvou::ReferenceObrLE_textChanged(const QString &arg1)
{
    QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
    QString kompletni_cesta = rozborObrReference[0]+"/"+arg1+"."+rozborObrReference[2];
    cv::Mat referencniObr;
    referencniObr = cv::imread(kompletni_cesta.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
    if (referencniObr.empty())
    {
        ReferenceLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Obrazek zvoleny jako referenční nelze otevřít.";
        spravnostReferenceObr = false;
    }
    else
    {
        ReferenceLE->setStyleSheet("color: #339900");
        spravnostReferenceObr = true;
    }
    referencniObr.release();
}

void LicovaniDvou::PosunutyObrLE_textChanged(const QString &arg1)
{
    QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
    QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
    QString kompletni_cesta = rozborObrPosunuty[0]+"/"+arg1+"."+rozborObrPosunuty[2];
    cv::Mat posunutyObr;
    posunutyObr = cv::imread(kompletni_cesta.toLocal8Bit().constData(),CV_LOAD_IMAGE_UNCHANGED);
    if (posunutyObr.empty())
    {
        PosunutyLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Obrazek zvoleny jako posunutý nelze otevřít.";
        spravnostReferenceObr = false;
    }
    else
    {
        PosunutyLE->setStyleSheet("color: #339900");
        spravnostReferenceObr = true;
    }
    posunutyObr.release();
}

void LicovaniDvou::VideoLE_textChanged(QLineEdit *LE, QString &s)
{
    QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+s+"."+rozborVybranehoSouboru[2];
    cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        LE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVidea = false;
    }
    else
    {
        LE->setStyleSheet("color: #00FF00");
        spravnostVidea = true;
        rozborVybranehoSouboru[1] = s;
    }
}

void LicovaniDvou::Slot_VideoLE_textChanged(const QString &s)
{

    QWidget* widgetVideoLE = ui->nabidkaAnalyzy->itemAt(0)->widget();
    QLineEdit* VideoLE = qobject_cast<QLineEdit*>(widgetVideoLE);
    QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+s+"."+rozborVybranehoSouboru[2];
    cap = cv::VideoCapture(kompletni_cesta.toLocal8Bit().constData());
    if (!cap.isOpened())
    {
        VideoLE->setStyleSheet("color: #FF0000");
        qDebug()<<"video nelze otevrit pro potreby zpracovani";
        spravnostVidea = false;
    }
    else
    {
        VideoLE->setStyleSheet("color: #339900");
        spravnostVidea = true;
        rozborVybranehoSouboru[1] = s;
    }
}

void LicovaniDvou::ReferenceLE_textChanged(const QString &arg1)
{
    QWidget* widgetReferenceLE = ui->nabidkaAnalyzy->itemAt(2)->widget();
    QLineEdit* ReferenceLE = qobject_cast<QLineEdit*>(widgetReferenceLE);
    int pocet_snimku_videa = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 || zadane_cislo > pocet_snimku_videa)
    {
        ReferenceLE->setStyleSheet("color: #FF0000");
        //qDebug()<<"Referencni snimek nelze ve videu dohledat";
        spravnostReference = false;
        cisloReference = -1;
    }
    else
    {
        ReferenceLE->setStyleSheet("color: #339900");
        spravnostReference = true;
        cisloReference = zadane_cislo;
    }
}

void LicovaniDvou::PosunutyLE_textChanged(const QString &arg1)
{
    QWidget* widgetPosunutyLE = ui->nabidkaAnalyzy->itemAt(3)->widget();
    QLineEdit* PosunutyLE = qobject_cast<QLineEdit*>(widgetPosunutyLE);
    int pocet_snimku_videa = int(cap.get(CV_CAP_PROP_FRAME_COUNT));
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 || zadane_cislo > pocet_snimku_videa)
    {
        PosunutyLE->setStyleSheet("color: #FF0000");
        qDebug()<<"Snimek posunuty nelze ve videu dohledat";
        spravnostPosunuteho = false;
        cisloPosunuteho = -1;
    }
    else
    {
        PosunutyLE->setStyleSheet("color: #339900");
        spravnostPosunuteho = true;
        cisloPosunuteho = zadane_cislo;
    }
}

void LicovaniDvou::on_oblastMaxima_textChanged(const QString &arg1)
{
    double oblast_maxima_minimum = 0.0;
    double oblast_maxima_maximum = 20.0;
    double zadane_cislo = arg1.toDouble();
    if (zadane_cislo < oblast_maxima_minimum || zadane_cislo > oblast_maxima_maximum)
    {
        ui->oblastMaxima->setStyleSheet("QLineEdit#oblastMaxima{color: #FF0000}");
        spravnostOblasti = false;
        oblastMaxima = -1;
    }
    else
    {
        ui->oblastMaxima->setStyleSheet("QLineEdit#oblastMaxima{color: #00FF00}");
        spravnostOblasti = true;
        oblastMaxima = zadane_cislo;
    }
}

void LicovaniDvou::on_uhelRotace_textChanged(const QString &arg1)
{
    double oblast_maxima_minimum = 0.0;
    double oblast_maxima_maximum = 0.5;
    double zadane_cislo = arg1.toDouble();
    if (zadane_cislo < oblast_maxima_minimum || zadane_cislo > oblast_maxima_maximum)
    {
        ui->uhelRotace->setStyleSheet("QLineEdit#uhelRotace{color: #FF0000}");
        spravnostUhlu = false;
        uhel = 0.1;
    }
    else
    {
        ui->uhelRotace->setStyleSheet("QLineEdit#uhelRotace{color: #00FF00}");
        spravnostUhlu = true;
        uhel = zadane_cislo;
    }
}

void LicovaniDvou::on_pocetIteraci_textChanged(const QString &arg1)
{
    int zadane_cislo = arg1.toInt();
    if (zadane_cislo < 0 && zadane_cislo != -1)
    {
        ui->pocetIteraci->setStyleSheet("QLineEdit#pocetIteraci{color: #FF0000}");
        spravnostIteraci = false;
        iterace = -1;
    }
    if (zadane_cislo == -1 || zadane_cislo > 1)
    {
        ui->pocetIteraci->setStyleSheet("QLineEdit#pocetIteraci{color: #00FF00}");
        spravnostIteraci = true;
        if (zadane_cislo == -1)        
            iterace = -1;        
        else        
            iterace = zadane_cislo;        
    }
}

void LicovaniDvou::on_slicujDvaSnimky_clicked()
{
    //this->setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    //MainWindow* mojeplikace = new MainWindow();
    //mojeplikace->showMinimized();
    if (spravnostVidea == false || spravnostReference == false || spravnostPosunuteho == false
            || spravnostOblasti == false || spravnostUhlu == false || spravnostIteraci == false)
    {
        ErrorDialog* errordialog = new ErrorDialog;
        errordialog->setModal(true);
        errordialog->exec();
    }
    else
    {        
        //qDebug()<<parametry_frangi;
        //qDebug()<<parametry_frangi.length();
        QString cesta_k_souboru = rozborVybranehoSouboru[0]+"/"+rozborVybranehoSouboru[1]+"."+rozborVybranehoSouboru[2];
        cap = cv::VideoCapture(cesta_k_souboru.toLocal8Bit().constData());
        cap.set(CV_CAP_PROP_POS_FRAMES,double(cisloReference));
        cv::Mat referencni_snimek,posunuty;
        cap.read(referencni_snimek);
        kontrola_typu_snimku_8C3(referencni_snimek);
        qDebug()<<"reference má "<<referencni_snimek.channels()<<" kanálů a typu "<<referencni_snimek.type();
        cap.set(CV_CAP_PROP_POS_FRAMES,double(cisloPosunuteho));
        cap.read(posunuty);
        kontrola_typu_snimku_8C3(posunuty);

        double entropie_posunuteho,entropie_reference;
        cv::Scalar tennengrad_posunuteho,tennengrad_reference;
        vypocet_entropie(referencni_snimek,entropie_reference,tennengrad_reference);
        vypocet_entropie(posunuty,entropie_posunuteho,tennengrad_posunuteho);
        //cout << "E: " << entropie_reference <<" T: " << tennengrad_reference[0] << endl;
        //cout << "E: " << entropie_posunuteho << " T: " << tennengrad_posunuteho[0] << endl;
        ui->ER->setText(QString::number(entropie_reference));
        ui->TR->setText(QString::number(tennengrad_reference[0]));
        ui->EP->setText(QString::number(entropie_posunuteho));
        ui->TP->setText(QString::number(tennengrad_posunuteho[0]));

        cv::Rect vyrez_korelace_extra(0,0,0,0);
        cv::Rect vyrez_korelace_standard(0,0,0,0);
        cv::Rect vyrez_anomalie(0,0,0,0);
        cv::Point3d pt_temp(0.0,0.0,0.0);
        cv::Mat obraz;
        predzpracovaniKompletnihoLicovani(referencni_snimek,
                                          obraz,
                                          parametry_frangi,
                                          oznacena_hranice_svetelne_anomalie,
                                          oznacena_hranice_casove_znacky,
                                          maximum_frangi,
                                          vyrez_anomalie,
                                          vyrez_korelace_extra,
                                          vyrez_korelace_standard,
                                          cap,
                                          ui->anomalie,
                                          ui->casovaZnacka,
                                          pritomnost_svetelne_anomalie,
                                          pritomnost_casove_znacky,
                                          zmena_meritka);

        int rows = referencni_snimek.rows;
        int cols = referencni_snimek.cols;
        cv::Mat obraz_32f,obraz_vyrez;
        obraz.copyTo(obraz_32f);
        //posunuty.copyTo(posunuty_32f);
        kontrola_typu_snimku_32C1(obraz_32f);
        //kontrola_typu_snimku_32C1(posunuty_32f);
        //qDebug()<<"Obraz "<<obraz.channels();
        //qDebug()<<"Posunuty"<<posunuty.channels();
        //qDebug()<<"Obraz 32"<<obraz_32f.channels();
        //qDebug()<<"Posunuty 320"<<posunuty_32f.channels();
        obraz_32f(vyrez_korelace_standard).copyTo(obraz_vyrez);
        cv::Point3d maximum_frangi_reverse = frangi_analyza(obraz,2,2,0,"",1,false,pt_temp,parametry_frangi);
        qDebug()<<"Maximum frangi reverse "<<maximum_frangi_reverse.x<<" "<<maximum_frangi_reverse.y;
        /// Začátek
        cv::Mat mezivysledek = cv::Mat::zeros(obraz.size(), CV_32FC3);
        cv::Point3d pt3;
        double uhel = 0;
        double celkovy_uhel = 0;
        int uspech_slicovani = kompletni_slicovani(cap,
                                    referencni_snimek,
                                    cisloPosunuteho,
                                    iterace,
                                    oblastMaxima,
                                    uhel,
                                    vyrez_korelace_extra,
                                    vyrez_korelace_standard,
                                    zmena_meritka,
                                    mezivysledek,
                                    pt3,
                                    celkovy_uhel);
        /// Konec
        if (uspech_slicovani==0)
            qDebug()<<"Licovani skoncilo chybou";
        else
        {
            qDebug()<<"PT3 - posunutí po multiPOC "<<pt3.x<<" "<<pt3.y;
            cv::Mat korekce = eventualni_korekce_translace(mezivysledek,obraz,vyrez_korelace_standard,pt3,oblastMaxima);
            qDebug()<<"Mezivýsledek "<<mezivysledek.channels()<<" "<<mezivysledek.type();
            cv::Point3d pt5 = fk_translace_hann(obraz,korekce,oblastMaxima);
            qDebug()<<"PT5 obraz vs korekce"<<pt5.x<<" "<<pt5.y;
            double sigma_gauss = 1/(std::sqrt(2*CV_PI)*pt5.z);
            double FWHM = 2*std::sqrt(2*std::log(2)) * sigma_gauss;
            qDebug()<<"FWHM: "<<FWHM;
            cv::Point3d pt6 = fk_translace(obraz,korekce,oblastMaxima);
            qDebug()<<"Pt6 "<<pt6.x<<" "<<pt6.y;
            cv::Point3d souradnice_slicovany_frangi_reverse = frangi_analyza(mezivysledek,2,2,0,"",2,false,pt3,parametry_frangi);
            //cv::Point3d souradnice_slicovany_frangi = frangi_analyza(mezivysledek,1,1,0,"",2,false,pt3,parametry_frangi);
            double yydef = maximum_frangi_reverse.x - souradnice_slicovany_frangi_reverse.x;
            double xxdef = maximum_frangi_reverse.y - souradnice_slicovany_frangi_reverse.y;
            cv::Point3d vysledne_posunuti;
            vysledne_posunuti.y = pt3.y - yydef;
            vysledne_posunuti.x = pt3.x - xxdef;
            vysledne_posunuti.z = 0;
            cv::Mat posunuty_temp2 = translace_snimku(posunuty,vysledne_posunuti,rows,cols);
            cv::Mat finalni_licovani = rotace_snimku(posunuty_temp2,uhel);
            cv::Mat finalni_licovani_32f,finalni_licovani_32f_vyrez;
            finalni_licovani.copyTo(finalni_licovani_32f);
            kontrola_typu_snimku_32C1(finalni_licovani_32f);
            finalni_licovani_32f(vyrez_korelace_standard).copyTo(finalni_licovani_32f_vyrez);
            qDebug()<<"Licovani dokonceno";
            src1 = obraz;
            src2 = mezivysledek;
            kontrola_typu_snimku_8C3(src1);
            kontrola_typu_snimku_8C3(src2);

            /// originální verze z DP, která však tvoří okno ovladatelné až po ukončení samotné aplikace
            /*alpha_slider = 0;
            cv::namedWindow("Kontrola_licovani", CV_WINDOW_AUTOSIZE); // Create Window
            cv::createTrackbar("Vybrany snimek","Kontrola_licovani",&slider_position,1,on_trackbar);
            on_trackbar( alpha_slider, 0 );*/
            /// nová verze prohlížeče, která umožní ovládání bez vypnutí hlavního programu
            VysledekLicovaniDvou *vysledekLicovani = new VysledekLicovaniDvou(src1,src2);
            vysledekLicovani->setModal(true);
            vysledekLicovani->show();
        }
    }
}

void LicovaniDvou::zobrazKliknutelnyDialog()
{
    if (ui->anomalie->isChecked())
    {
        QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+rozborVybranehoSouboru[1]+"."+rozborVybranehoSouboru[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,cisloReference,1);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
    }
    if (ui->casovaZnacka->isChecked())
    {
        QString kompletni_cesta = rozborVybranehoSouboru[0]+"/"+rozborVybranehoSouboru[1]+"."+rozborVybranehoSouboru[2];
        ClickImageEvent* vyznac_anomalii = new ClickImageEvent(kompletni_cesta,cisloReference,2);
        vyznac_anomalii->setModal(true);
        vyznac_anomalii->show();
    }

        /*QObject::connect(vyznac_anomalii,SIGNAL(SendClickCoordinates(QPointF)),
                         this,SLOT(GetClickCoordinates(QPointF)));*/
        //connect(vyznac_anomalii, vyznac_anomalii->SendClickCoordinates, (=)[const auto &myString] {ui->label->setText(myString);});
}
