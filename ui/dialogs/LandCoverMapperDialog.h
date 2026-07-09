#pragma once
#include <QDialog>
#include <QTabWidget>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include "hsi/LandCoverMapper.h"

class MainWindow;

// Three-tab dialog exposing all three offline land-cover approaches:
//   Tab 1 — Spectral Indices  (zero training, pure math)
//   Tab 2 — Spectral Library  (SAM against bundled/user signatures)
//   Tab 3 — Click-to-train    (mark pixels, SVM trains on-device)
class LandCoverMapperDialog : public QDialog {
    Q_OBJECT
public:
    explicit LandCoverMapperDialog(MainWindow* mw, QWidget* parent = nullptr);

private slots:
    void runIndexBased();
    void runLibraryBased();
    void browseLibrary();
    void runSvm();
    void addSampleRow();
    void removeSampleRow();
    void exportResult();
    void exportVector();

private:
    void buildIndexTab(QWidget* tab);
    void buildLibraryTab(QWidget* tab);
    void buildSvmTab(QWidget* tab);
    void showResult(const hsi::RasterCube& result);

    MainWindow* mw_;
    QTabWidget* tabs_;

    // Tab 1 — index params
    QDoubleSpinBox* thrForest_;
    QDoubleSpinBox* thrVeg_;
    QDoubleSpinBox* thrWater_;
    QDoubleSpinBox* thrBuiltUp_;
    QDoubleSpinBox* thrBareSoil_;

    // Tab 2 — library
    QLineEdit*      libPathEdit_;
    QDoubleSpinBox* samAngleSpin_;

    // Tab 3 — click-to-train
    QTableWidget*   sampleTable_;   // columns: Class, Row, Col

    // Last result for export
    hsi::RasterCube lastResult_;
    bool            hasResult_ = false;
};
