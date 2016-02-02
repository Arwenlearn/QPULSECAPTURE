#ifndef QSETTINGSDIALOG_H
#define QSETTINGSDIALOG_H

#include <QDialog>
#include <QString>
#include <QFileDialog>

namespace Ui {
class QSettingsDialog;
}

class QSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QSettingsDialog(QWidget *parent = 0);
    ~QSettingsDialog();


public:
    bool get_FFTflag() const;
    bool get_flagCascade() const;
    bool get_flagVideoFile() const;
    QString get_stringCascade() const;
    quint32 get_datalength() const;
    int get_timerValue() const;
    int get_patientPercentile() const;
    QString get_stringDistribution() const;
    int get_patientAge() const;
    bool get_customPatientFlag() const;
    int get_patientSex() const;

private slots:

    void on_ButtonAccept_clicked();

    void on_ButtonCancel_clicked();

    void on_dialDatalength_valueChanged(int value);

    void on_ButtonCascade_clicked();

    void on_ButtonDefault_clicked();

    void on_checkBoxCascade_stateChanged(int arg1);

    void on_horizontalSliderTimer_valueChanged(int value);

    void on_checkBoxPatient_stateChanged(int arg1);

    void on_pushButtonPatient_clicked();

    void on_radioButtonMale_clicked(bool checked);

    void on_radioButtonFemale_clicked(bool checked);

private:
    Ui::QSettingsDialog *ui;
};

#endif // QSETTINGSDIALOG_H
