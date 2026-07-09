#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>

class MainWindow;

class PreprocessingDialog : public QDialog {
    Q_OBJECT
public:
    explicit PreprocessingDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void browseInput();
    void browseEsun();
    void browseElmSamples();
    void run();

private:
    MainWindow* mw_;
    QLineEdit* inputPathEdit_;
    QLineEdit* esunCsvEdit_;
    QDoubleSpinBox* solarZenithSpin_;
    QSpinBox* dayOfYearSpin_;
    QDoubleSpinBox* pixelSizeSpin_;

    QComboBox* correctionMethodCombo_; // DOS vs ELM

    QGroupBox* dosGroup_;
    QDoubleSpinBox* darkObjectPercentileSpin_;

    QGroupBox* elmGroup_;
    QLineEdit* elmSampleCsvEdit_;
    QDoubleSpinBox* elmDarkReflectanceSpin_;
    QDoubleSpinBox* elmBrightReflectanceSpin_;
};
