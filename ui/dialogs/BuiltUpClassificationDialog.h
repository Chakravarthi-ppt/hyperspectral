#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QComboBox>

class MainWindow;

class BuiltUpClassificationDialog : public QDialog {
    Q_OBJECT
public:
    explicit BuiltUpClassificationDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void browseSamplesCsv();
    void run();
    void exportVector();

private:
    MainWindow* mw_;
    QLineEdit* samplesCsvEdit_;
    QLineEdit* builtUpClassNamesEdit_;
    QDoubleSpinBox* samAngleSpin_;
    QComboBox* fusionCombo_;
};
