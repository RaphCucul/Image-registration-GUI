#ifndef HDD_SETTINGS_H
#define HDD_SETTINGS_H

#include <QDialog>
#include <QTimer>
#include <windows.h>

namespace Ui {
class HDD_Settings;
}

/**
 * @class HDD_Settings
 * @brief The HDD_Settings class provides user interface for HDD counter and parameter processing.
 *
 * If a user wants to add
 * HDD counter and parameter names translated to the system language, the program must be started with administrator privileges
 */
class HDD_Settings : public QDialog
{
    Q_OBJECT

public:
    /**
     * @enum IconType
     * @brief The IconType enum helps to identify which icon should be displayed in the dialogue.
     */
    enum class IconType{
        GREY,
        ORANGE,
        GREEN,
        RED
    };
    explicit HDD_Settings(QWidget *parent = nullptr);
    ~HDD_Settings();
private slots:
    /**
     * @brief The function for QDialog buttonbox "OK" button.
     */
    void slot_accepted();

    /**
     * @brief The function for QDialog buttonbox "Cancel" button.
     */
    void slot_rejected();

    /**
     * @brief Calls powershell script to identify HDD counter and parameter names. Dialogue icon is updated according to the
     * actual progress.
     */
    void onRunScript();

private:
    Ui::HDD_Settings *ui;

    /**
     * @class RegistryError
     * @brief The RegistryError class stores error message created during Windows Registry analysis.
     */
    class RegistryError
        : public std::runtime_error
    {
    public:
        RegistryError() : runtime_error("Error when processing Windows Regitry."){};
        /**
         * @brief Saves data into RegistryError class object.
         * @param[in] message - text of the message
         * @param[in] errorCode - code representing Windows error
         */
        void setData(const char* message, LONG errorCode);
        /**
         * @brief Provides stored data from RegistryError class object.
         * @return Complete string, contains error message and code
         */
        QString ErrorMessage() const noexcept
        {
            return QString("Error: %1 %2").arg(QString::fromLatin1(m_errorMessage)).arg(m_errorCode);
        }
    private:
        LONG m_errorCode;
        const char* m_errorMessage;
    };

    /**
     * @class RegistryAccess
     * @brief The RegistryAccess class object accesses Windows Registry, analyses specific keys and returns
     * obtained information.
     */
    class RegistryAccess
    {
    public:
        /**
         * @brief Analyses Windows Registry key containing English original names for parameters. Index of the given
         * requested parameter is returned.
         * @param i_name - requested parameter name
         * @param o_rE - RegistryError class object
         * @return Index of the requested parameter in the complete list of parameters
         */
        int GetIndexForName(const QString i_name, RegistryError *o_rE);
        /**
         * @brief Creates vector of wstring objects from the REG_MULTI_SZ.
         * @param hKey - main HKEY category
         * @param subKey - complete path to the specific REG_MULTI_SZ
         * @param value - name of the REG_MULTI_SZ parameter
         * @param o_rE - RegistryError class object
         * @return Obtained vector of wstring objects
         */
        std::vector<std::wstring> RegGetMultiString(HKEY hKey, const std::wstring& subKey, const std::wstring& value, RegistryError *o_rE);
        /**
         * @brief Analyses Windows Registry key containing localised parameters names. Given index is used to extract specific parameter
         * name.
         * @param i_index - numeric index of the requested parameter
         * @param o_rE - RegistryError class object
         * @return Localised name of the requested parameter
         */
        QString GetTranslationForName(const int i_index, RegistryError* o_rE);
    private:
    };

    /**
     * @brief Loads the icon of the corresponding icon type to the dialogue.
     * @param i_type
     */
    void setLabelIcon(IconType i_type);
    /**
     * @brief Loads the text describing the progress of the algortihm to the dialogue.
     * @param i_text
     */
    void setLabelText(QString i_text);

    QTimer* timer = nullptr;

    RegistryAccess* _rA = nullptr;
    RegistryError* _rE = nullptr;
};

#endif // HDD_SETTINGS_H
