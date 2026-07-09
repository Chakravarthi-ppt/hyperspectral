#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>

class MainWindow;

class PcaStackDialog : public QDialog {
    Q_OBJECT
public:
    explicit PcaStackDialog(MainWindow* mainWindow, QWidget* parent = nullptr);

private slots:
    void browseFused();
    void runPca();
    void runStack();

private:
    MainWindow* mw_;

    QSpinBox*  pcaStartSpin_;
    QSpinBox*  pcaEndSpin_;
    QSpinBox*  pcaComponentsSpin_;

    QLineEdit* fusedPathEdit_;   // single 9-band fused TIFF from fusion team
};
