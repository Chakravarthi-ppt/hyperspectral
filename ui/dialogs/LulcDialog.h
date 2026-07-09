#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>

class MainWindow;

class LulcDialog : public QDialog {
    Q_OBJECT
public:
    explicit LulcDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void browseSamplesCsv();
    void browseDateBStack();
    void runUnsupervised();
    void runSupervised();
    void classifyDateB();

private:
    MainWindow* mw_;
    QSpinBox* kSpin_;
    QLineEdit* samplesCsvEdit_;
    QLineEdit* dateBStackEdit_;
};
