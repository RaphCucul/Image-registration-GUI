#include <string>
#include <io.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>  // For stat().
#include <sys/stat.h>   // For stat().
#include <cstdlib>
#include <unistd.h>
#include <string>
//#include "stdafx.h"

#include "hdd_settings.h"
#include "ui_hdd_settings.h"
#include "shared_staff/globalsettings.h"
#include "util/files_folders_operations.h"
#include "util/adminCheck.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
}

HDD_Settings::HDD_Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HDD_Settings)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(slot_accepted()));
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(slot_rejected()));
    connect(ui->runScript,SIGNAL(clicked()),this,SLOT(onRunScript()));

    setLabelIcon(IconType::GREY);
    timer = new QTimer(this);

    QString counter="",parameter="",powershell="";
    counter = GlobalSettings::getSettings()->getHDDCounterName();
    parameter = GlobalSettings::getSettings()->getHDDCounterParameter();
    if (!counter.isEmpty())
        ui->counterNameLE->setText(counter);
    if (!parameter.isEmpty())
        ui->counterParameterLE->setText(parameter);
    if (!counter.isEmpty() && !parameter.isEmpty()) {
        ui->diodeLabel->setPixmap(QPixmap(":/images/greenDiode.png"));
        setLabelText(tr("Counter and parameter found"));
    }
    _rA = new RegistryAccess();
    _rE = new RegistryError();

    // detect if the program is started with admin privilegies
    bool isAdmin = IsCurrentUserLocalAdministrator();
    if (isAdmin)
        ui->runScript->setEnabled(true);
    else
        ui->runScript->setEnabled(false);
}

HDD_Settings::~HDD_Settings()
{
    delete ui;
}

void HDD_Settings::slot_accepted(){
    GlobalSettings::getSettings()->setHDDCounter(ui->counterNameLE->text(),ui->counterParameterLE->text());
}

void HDD_Settings::slot_rejected(){
    timer->stop();
}

void HDD_Settings::setLabelIcon(IconType i_type){
    switch (i_type) {
    case IconType::GREEN:
        ui->diodeLabel->setPixmap(QPixmap(":/images/greenDiode.png"));
        break;
    case IconType::GREY:
        ui->diodeLabel->setPixmap(QPixmap(":/images/greyDiode.png"));
        break;
    case IconType::RED:
        ui->diodeLabel->setPixmap(QPixmap(":/images/redDiode.png"));
        break;
    case IconType::ORANGE:
        ui->diodeLabel->setPixmap(QPixmap(":/images/orangeDiode.png"));
        break;
    }
}

void HDD_Settings::setLabelText(QString i_text){
    ui->infoLabel->setText(i_text);
}


void HDD_Settings::onRunScript()
{
    setLabelIcon(IconType::ORANGE);
    QString message = QString(tr("Processing"));
    setLabelText(message);
    int disk = _rA->GetIndexForName("PhysicalDisk",_rE);
    QString diskLocated = "";
    if (disk != -1)
        diskLocated = _rA->GetTranslationForName(disk,_rE);
    int counter = _rA->GetIndexForName("% Disk Time",_rE);
    QString counterLocated = "";
    if (counter != -1)
        counterLocated = _rA->GetTranslationForName(counter,_rE);
    if (counter == -1 || disk == -1) {
        setLabelIcon(IconType::RED);
        QString errorMessage = QString(tr("An error occured when processing registry keys. %1")).arg(_rE->ErrorMessage());
        setLabelText(errorMessage);
    }
    else {
        setLabelIcon(IconType::GREEN);
        setLabelText(tr("Done"));
        ui->counterNameLE->setText(diskLocated);
        ui->counterParameterLE->setText(counterLocated);
    }
}

int HDD_Settings::RegistryAccess::GetIndexForName(const QString i_name, RegistryError *o_rE) {

    std::vector<std::wstring> counters = RegGetMultiString(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\009",
        L"Counter",
        o_rE);
    if (counters.size() != 0) {
        for (unsigned int i=0; i < counters.size(); i++) {
            if (counters.at(i) == i_name.toStdWString())
                return i;
        }
    }
    return -1;
}

void HDD_Settings::RegistryError::setData(const char *message, LONG errorCode) {
    m_errorMessage = message;
    m_errorCode = errorCode;
}

QString HDD_Settings::RegistryAccess::GetTranslationForName(const int i_index, RegistryError *o_rE) {
    std::vector<std::wstring> counters = RegGetMultiString(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\CurrentLanguage",
        L"Counter",
        o_rE);
    QString located = "";
    if (counters.size() >= unsigned(i_index)) {
        return QString::fromStdWString(counters.at(i_index));
    }
    else
        return located;
}

std::vector<std::wstring> HDD_Settings::RegistryAccess::RegGetMultiString(HKEY hKey,
                                                                          const std::wstring& subKey,
                                                                          const std::wstring& value,
                                                                          RegistryError* o_rE)
{
    DWORD dataSize{};
    std::vector<std::wstring> result = {};
    LONG retCode = ::RegGetValue(
        hKey,
        subKey.c_str(),
        value.c_str(),
        RRF_RT_REG_MULTI_SZ,
        nullptr,
        nullptr,
        &dataSize
        );
    if (retCode != ERROR_SUCCESS)
    {
        o_rE->setData("Cannot read multi-string from registry", retCode);
        return result;
    }
    std::vector<wchar_t> data;
    data.resize(dataSize / sizeof(wchar_t));
    retCode = ::RegGetValue(
        hKey,
        subKey.c_str(),
        value.c_str(),
        RRF_RT_REG_MULTI_SZ,
        nullptr,
        &data[0],
        &dataSize
        );
    if (retCode != ERROR_SUCCESS)
    {
        o_rE->setData("Cannot read multi-string from registry", retCode);
        return result;
    }
    data.resize( dataSize / sizeof(wchar_t) );
    // Parse the double-NUL-terminated string into a vector<wstring>
    const wchar_t* currStringPtr = &data[0];
    while (*currStringPtr != L'\0')
    {
        // Current string is NUL-terminated, so get its length with wcslen
        const size_t currStringLength = wcslen(currStringPtr);
        // Add current string to result vector
        result.push_back(std::wstring{ currStringPtr, currStringLength });
        // Move to the next string
        currStringPtr += currStringLength + 1;
    }
    return result;
}
