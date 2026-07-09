#pragma once
#include <QDialog>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QLabel>
#include "hsi/Types.h"

class MainWindow;

class ChangeDetectionDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChangeDetectionDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void run();
    void saveCsv();

private:
    MainWindow* mw_;
    QDoubleSpinBox* pixelAreaSpin_;
    QTableWidget* table_;
    QLabel* summaryLabel_;
    hsi::ChangeMatrixResult lastResult_;
    bool hasResult_ = false;
};
