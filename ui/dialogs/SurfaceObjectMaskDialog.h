#pragma once
#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QStackedWidget>
#include <QCheckBox>

class MainWindow;

class SurfaceObjectMaskDialog : public QDialog {
    Q_OBJECT
public:
    explicit SurfaceObjectMaskDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void browseSamplesCsv();
    void browseReference();
    void run();

private:
    MainWindow* mw_;
    QComboBox* methodCombo_;
    QStackedWidget* methodStack_;

    // Threshold page
    QSpinBox* nirBandSpin_;
    QSpinBox* swirBandSpin_;
    QDoubleSpinBox* thresholdSpin_;
    QCheckBox* autoThresholdCheck_;

    // K-means page
    QSpinBox* attemptsSpin_;

    // Trained SVM page
    QLineEdit* samplesCsvEdit_;
    QLineEdit* objectClassNameEdit_;

    // Validation (shared)
    QLineEdit* referencePathEdit_;
};
