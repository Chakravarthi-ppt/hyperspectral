////#include "LulcDialog.h"
////#include "../MainWindow.h"
////#include "../RasterPreviewWidget.h"
////#include "../Utils.h"

////#include <QFormLayout>
////#include <QVBoxLayout>
////#include <QHBoxLayout>
////#include <QPushButton>
////#include <QFileDialog>
////#include <QMessageBox>
////#include <QLabel>
////#include <QGroupBox>
////#include <map>

////using namespace hsi;

////LulcDialog::LulcDialog(MainWindow* mainWindow, QWidget* parent) : QDialog(parent), mw_(mainWindow) {
////    setWindowTitle("Land Use / Land Cover Classification");
////    setMinimumWidth(560);

////    kSpin_ = new QSpinBox(); kSpin_->setRange(2, 20); kSpin_->setValue(4);
////    auto* unsupForm = new QFormLayout();
////    unsupForm->addRow("Number of clusters (k):", kSpin_);
////    auto* unsupRunBtn = new QPushButton("Run Unsupervised (k-means)");
////    auto* unsupGroup = new QGroupBox("Unsupervised classification");
////    auto* unsupLayout = new QVBoxLayout();
////    unsupLayout->addLayout(unsupForm);
////    unsupLayout->addWidget(unsupRunBtn);
////    unsupGroup->setLayout(unsupLayout);

////    samplesCsvEdit_ = new QLineEdit();
////    auto* browseSamplesBtn = new QPushButton("Browse...");
////    auto* samplesRow = new QHBoxLayout();
////    samplesRow->addWidget(samplesCsvEdit_);
////    samplesRow->addWidget(browseSamplesBtn);
////    auto* supForm = new QFormLayout();
////    supForm->addRow("Labeled samples CSV (class_name,row,col):", samplesRow);
////    auto* supRunBtn = new QPushButton("Run Supervised (SVM)");
////    auto* supGroup = new QGroupBox("Supervised classification");
////    auto* supLayout = new QVBoxLayout();
////    supLayout->addLayout(supForm);
////    supLayout->addWidget(supRunBtn);
////    supGroup->setLayout(supLayout);

////    dateBStackEdit_ = new QLineEdit();
////    auto* browseDateBBtn = new QPushButton("Browse...");
////    auto* dateBRow = new QHBoxLayout();
////    dateBRow->addWidget(dateBStackEdit_);
////    dateBRow->addWidget(browseDateBBtn);
////    auto* dateBForm = new QFormLayout();
////    dateBForm->addRow("Second-date 206-band stack (.tif):", dateBRow);
////    auto* dateBRunBtn = new QPushButton("Classify Second Date (reuses trained model)");
////    auto* dateBGroup = new QGroupBox("Two-date change input");
////    auto* dateBLayout = new QVBoxLayout();
////    dateBLayout->addLayout(dateBForm);
////    dateBLayout->addWidget(dateBRunBtn);
////    dateBGroup->setLayout(dateBLayout);

////    auto* layout = new QVBoxLayout(this);
////    layout->addWidget(unsupGroup);
////    layout->addWidget(supGroup);
////    layout->addWidget(dateBGroup);

////    connect(browseSamplesBtn, &QPushButton::clicked, this, &LulcDialog::browseSamplesCsv);
////    connect(browseDateBBtn, &QPushButton::clicked, this, &LulcDialog::browseDateBStack);
////    connect(unsupRunBtn, &QPushButton::clicked, this, &LulcDialog::runUnsupervised);
////    connect(supRunBtn, &QPushButton::clicked, this, &LulcDialog::runSupervised);
////    connect(dateBRunBtn, &QPushButton::clicked, this, &LulcDialog::classifyDateB);
////}

////void LulcDialog::browseSamplesCsv() {
////    QString path = QFileDialog::getOpenFileName(this, "Select samples CSV", QString(), "CSV files (*.csv);;All files (*)");
////    if (!path.isEmpty()) samplesCsvEdit_->setText(path);
////}

////void LulcDialog::browseDateBStack() {
////    QString path = QFileDialog::getOpenFileName(this, "Select second-date stack", QString(), "Raster files (*.tif *.tiff);;All files (*)");
////    if (!path.isEmpty()) dateBStackEdit_->setText(path);
////}

////// Returns the best available cube for LULC: fused stack if built, else surface reflectance.
////// This enables standalone operation when the fusion team TIFF has not been provided yet.
////static const hsi::RasterCube* bestCube(const AppState& s) {
////    if (s.stackFused)         return &(*s.stackFused);
////    if (s.surfaceReflectance) return &(*s.surfaceReflectance);
////    return nullptr;
////}

////void LulcDialog::runUnsupervised() {
////    const hsi::RasterCube* cube = bestCube(mw_->appState());
////    if (!cube) {
////        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
////        return;
////    }
////    bool usingStack = mw_->appState().stackFused.has_value();
////    mw_->log("LulcClassifier", usingStack ? "Using fused stack for LULC." : "Standalone mode: using surface reflectance (198 bands) for LULC.");
////    try {
////        std::map<int, std::string> clusterLabels;
////        RasterCube result = LulcClassifier::unsupervisedKMeans(*cube, kSpin_->value(), 5, &clusterLabels);
////        mw_->appState().lulcUnsupervised = result;
////        mw_->log("LulcClassifier", QString("Unsupervised LULC complete, k=%1.").arg(kSpin_->value()));
////        // Show with distinct colours per cluster, each labeled with a guessed
////        // land-cover type (from the cluster's centroid NDVI/NDWI/NDBI/BSI) so
////        // "Cluster 3" isn't just a meaningless number -- it's a starting point,
////        // not a certainty, so it's shown as "likely X" rather than stated flatly.
////        QString summaryText = QString("%1 clusters computed:\n\n").arg(kSpin_->value());
////        {
////            using CS = RasterPreviewWidget::CategoryStyle;
////            std::map<int,CS> pal;
////            static const QColor kColors[] = {
////                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
////                {210,160,80},{160,50,180},{50,190,190},{240,140,30},
////                {120,80,200},{80,160,80},{200,100,50},{50,120,180}
////            };
////            for (int i = 0; i < kSpin_->value(); ++i) {
////                QString guess = clusterLabels.count(i+1) ? QString::fromStdString(clusterLabels[i+1]) : QString();
////                QString label = guess.isEmpty() ? QString("Cluster %1").arg(i+1)
////                                                 : QString("Cluster %1 (%2)").arg(i+1).arg(guess);
////                pal[i+1] = CS{kColors[i % 12], label};
////                summaryText += QString("  Cluster %1: %2\n").arg(i+1).arg(guess.isEmpty() ? "?" : guess);
////            }
////            pal[0] = CS{QColor(30,30,30), "Unclassified"};
////            mw_->previewWidget()->showCategorical(result, 0, pal);
////        }
////        summaryText += "\nThese are automatic guesses based on each cluster's average spectral "
////                       "signature (NDVI/NDWI/NDBI/BSI) \u2014 spot-check a few pixels from each "
////                       "cluster against the image before relying on the label.";
////        QMessageBox::information(this, "Unsupervised LULC complete", summaryText);
////    } catch (const std::exception& e) {
////        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
////        QMessageBox::critical(this, "Unsupervised classification failed", e.what());
////    }
////}

////void LulcDialog::runSupervised() {
////    const hsi::RasterCube* cube = bestCube(mw_->appState());
////    if (!cube) {
////        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
////        return;
////    }
////    bool usingStack = mw_->appState().stackFused.has_value();
////    mw_->log("LulcClassifier", usingStack ? "Using fused stack for supervised LULC." : "Standalone mode: using surface reflectance for supervised LULC.");
////    if (samplesCsvEdit_->text().isEmpty()) {
////        QMessageBox::warning(this, "Missing samples", "Choose a labeled samples CSV.");
////        return;
////    }
////    try {
////        const RasterCube& stack = *cube;
////        auto samples = ui_util::loadSampleCsv(samplesCsvEdit_->text().toStdString());

////        // Assign each distinct class name an integer label in encounter order.
////        std::map<std::string, int> classToLabel;
////        int nextLabel = 1;
////        for (const auto& kv : samples) classToLabel[kv.first] = nextLabel++;

////        std::vector<std::vector<float>> features;
////        std::vector<int> labels;
////        for (const auto& kv : samples) {
////            for (auto rc : kv.second) {
////                if (rc.first < 0 || rc.first >= stack.height || rc.second < 0 || rc.second >= stack.width) continue;
////                features.push_back(stack.pixelSpectrum(rc.first, rc.second));
////                labels.push_back(classToLabel[kv.first]);
////            }
////        }
////        if (features.empty()) {
////            QMessageBox::warning(this, "No usable samples", "No sample pixels fell inside the stack's extent.");
////            return;
////        }

////        auto result = LulcClassifier::supervised(stack, features, labels);
////        mw_->appState().lulcSupervisedA = result.classified;
////        mw_->appState().lulcModel = result.model;

////        QString legend;
////        for (const auto& kv : classToLabel) legend += QString("%1=%2  ").arg(kv.second).arg(QString::fromStdString(kv.first));

////        mw_->log("LulcClassifier", QString("Supervised LULC complete (%1 classes). Legend: %2")
////                      .arg(classToLabel.size()).arg(legend));
////        {
////            using CS = RasterPreviewWidget::CategoryStyle;
////            std::map<int,CS> pal;
////            static const QColor kColors[] = {
////                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
////                {210,160,80},{160,50,180},{50,190,190},{240,140,30}
////            };
////            int ci = 0;
////            for (const auto& kv : classToLabel)
////                pal[kv.second] = CS{kColors[(ci++) % 8], QString::fromStdString(kv.first)};
////            pal[0] = CS{QColor(30,30,30), "Unclassified"};
////            mw_->previewWidget()->showCategorical(result.classified, 0, pal);
////        }
////        QMessageBox::information(this, "Supervised LULC complete", QString("Classes:\n%1").arg(legend));
////    } catch (const std::exception& e) {
////        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
////        QMessageBox::critical(this, "Supervised classification failed", e.what());
////    }
////}

////void LulcDialog::classifyDateB() {
////    if (!mw_->appState().lulcModel) {
////        QMessageBox::warning(this, "Train a model first", "Run the supervised classification (date A) first.");
////        return;
////    }
////    if (dateBStackEdit_->text().isEmpty()) {
////        QMessageBox::warning(this, "Missing input", "Choose a second-date 206-band stack raster.");
////        return;
////    }
////    try {
////        RasterCube stackB = RasterIO::loadCube(dateBStackEdit_->text().toStdString());
////        RasterCube classifiedB = mw_->appState().lulcModel->classifyCube(stackB);
////        mw_->appState().lulcSupervisedB = classifiedB;

////        mw_->log("LulcClassifier", "Second-date classification complete using the date-A trained model.");
////        mw_->previewWidget()->showSingleBand(classifiedB, 0);  // date B uses same trained palette
////        QMessageBox::information(this, "Second-date classification complete",
////            "Ready for Hyperspectral \u2192 Change Detection.");
////        accept();
////    } catch (const std::exception& e) {
////        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
////        QMessageBox::critical(this, "Second-date classification failed", e.what());
////    }
////}


//#include "LulcDialog.h"
//#include "../MainWindow.h"
//#include "../RasterPreviewWidget.h"
//#include "../Utils.h"

//#include <QFormLayout>
//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QPushButton>
//#include <QFileDialog>
//#include <QMessageBox>
//#include <QLabel>
//#include <QGroupBox>
//#include <map>

//using namespace hsi;

//LulcDialog::LulcDialog(MainWindow* mainWindow, QWidget* parent) : QDialog(parent), mw_(mainWindow) {
//    setWindowTitle("Land Use / Land Cover Classification");
//    setMinimumWidth(560);

//    kSpin_ = new QSpinBox(); kSpin_->setRange(2, 20); kSpin_->setValue(4);
//    auto* unsupForm = new QFormLayout();
//    unsupForm->addRow("Number of clusters (k):", kSpin_);
//    auto* unsupRunBtn = new QPushButton("Run Unsupervised (k-means)");
//    auto* unsupGroup = new QGroupBox("Unsupervised classification");
//    auto* unsupLayout = new QVBoxLayout();
//    unsupLayout->addLayout(unsupForm);
//    unsupLayout->addWidget(unsupRunBtn);
//    unsupGroup->setLayout(unsupLayout);

//    samplesCsvEdit_ = new QLineEdit();
//    auto* browseSamplesBtn = new QPushButton("Browse...");
//    auto* samplesRow = new QHBoxLayout();
//    samplesRow->addWidget(samplesCsvEdit_);
//    samplesRow->addWidget(browseSamplesBtn);
//    auto* supForm = new QFormLayout();
//    supForm->addRow("Labeled samples CSV (class_name,row,col):", samplesRow);
//    auto* supRunBtn = new QPushButton("Run Supervised (SVM)");
//    auto* supGroup = new QGroupBox("Supervised classification");
//    auto* supLayout = new QVBoxLayout();
//    supLayout->addLayout(supForm);
//    supLayout->addWidget(supRunBtn);
//    supGroup->setLayout(supLayout);

//    dateBStackEdit_ = new QLineEdit();
//    auto* browseDateBBtn = new QPushButton("Browse...");
//    auto* dateBRow = new QHBoxLayout();
//    dateBRow->addWidget(dateBStackEdit_);
//    dateBRow->addWidget(browseDateBBtn);
//    auto* dateBForm = new QFormLayout();
//    dateBForm->addRow("Second-date 206-band stack (.tif):", dateBRow);
//    auto* dateBRunBtn = new QPushButton("Classify Second Date (reuses trained model)");
//    auto* dateBGroup = new QGroupBox("Two-date change input");
//    auto* dateBLayout = new QVBoxLayout();
//    dateBLayout->addLayout(dateBForm);
//    dateBLayout->addWidget(dateBRunBtn);
//    dateBGroup->setLayout(dateBLayout);

//    auto* layout = new QVBoxLayout(this);
//    layout->addWidget(unsupGroup);
//    layout->addWidget(supGroup);
//    layout->addWidget(dateBGroup);

//    connect(browseSamplesBtn, &QPushButton::clicked, this, &LulcDialog::browseSamplesCsv);
//    connect(browseDateBBtn, &QPushButton::clicked, this, &LulcDialog::browseDateBStack);
//    connect(unsupRunBtn, &QPushButton::clicked, this, &LulcDialog::runUnsupervised);
//    connect(supRunBtn, &QPushButton::clicked, this, &LulcDialog::runSupervised);
//    connect(dateBRunBtn, &QPushButton::clicked, this, &LulcDialog::classifyDateB);
//}

//void LulcDialog::browseSamplesCsv() {
//    QString path = QFileDialog::getOpenFileName(this, "Select samples CSV", QString(), "CSV files (*.csv);;All files (*)");
//    if (!path.isEmpty()) samplesCsvEdit_->setText(path);
//}

//void LulcDialog::browseDateBStack() {
//    QString path = QFileDialog::getOpenFileName(this, "Select second-date stack", QString(), "Raster files (*.tif *.tiff);;All files (*)");
//    if (!path.isEmpty()) dateBStackEdit_->setText(path);
//}

//// Returns the best available cube for LULC: fused stack if built, else surface reflectance.
//// This enables standalone operation when the fusion team TIFF has not been provided yet.
//static const hsi::RasterCube* bestCube(const AppState& s) {
//    if (s.stackFused)         return &(*s.stackFused);
//    if (s.surfaceReflectance) return &(*s.surfaceReflectance);
//    return nullptr;
//}

//void LulcDialog::runUnsupervised() {
//    const hsi::RasterCube* cube = bestCube(mw_->appState());
//    if (!cube) {
//        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
//        return;
//    }
//    bool usingStack = mw_->appState().stackFused.has_value();
//    mw_->log("LulcClassifier", usingStack ? "Using fused stack for LULC." : "Standalone mode: using surface reflectance (198 bands) for LULC.");
//    try {
//        std::map<int, std::string> clusterLabels;
//        RasterCube result = LulcClassifier::unsupervisedKMeans(*cube, kSpin_->value(), 5, &clusterLabels);
//        mw_->appState().lulcUnsupervised = result;
//        mw_->log("LulcClassifier", QString("Unsupervised LULC complete, k=%1.").arg(kSpin_->value()));
//        // Show with distinct colours per cluster, each labeled with a guessed
//        // land-cover type (from the cluster's centroid NDVI/NDWI/NDBI/BSI) so
//        // "Cluster 3" isn't just a meaningless number -- it's a starting point,
//        // not a certainty, so it's shown as "likely X" rather than stated flatly.
//        QString summaryText = QString("%1 clusters computed:\n\n").arg(kSpin_->value());
//        {
//            using CS = RasterPreviewWidget::CategoryStyle;
//            std::map<int,CS> pal;
//            static const QColor kColors[] = {
//                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
//                {210,160,80},{160,50,180},{50,190,190},{240,140,30},
//                {120,80,200},{80,160,80},{200,100,50},{50,120,180}
//            };
//            for (int i = 0; i < kSpin_->value(); ++i) {
//                QString guess = clusterLabels.count(i+1) ? QString::fromStdString(clusterLabels[i+1]) : QString();
//                QString label = guess.isEmpty() ? QString("Cluster %1").arg(i+1)
//                                                 : QString("Cluster %1 (%2)").arg(i+1).arg(guess);
//                pal[i+1] = CS{kColors[i % 12], label};
//                summaryText += QString("  Cluster %1: %2\n").arg(i+1).arg(guess.isEmpty() ? "?" : guess);
//            }
//            pal[0] = CS{QColor(30,30,30), "Unclassified"};
//            mw_->previewWidget()->showCategorical(result, 0, pal);
//        }
//        summaryText += "\nThese are automatic guesses based on each cluster's average spectral "
//                       "signature (NDVI/NDWI/NDBI/BSI) \u2014 spot-check a few pixels from each "
//                       "cluster against the image before relying on the label.";
//        QMessageBox::information(this, "Unsupervised LULC complete", summaryText);
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Unsupervised classification failed", e.what());
//    }
//}

//void LulcDialog::runSupervised() {
//    const hsi::RasterCube* cube = bestCube(mw_->appState());
//    if (!cube) {
//        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
//        return;
//    }
//    bool usingStack = mw_->appState().stackFused.has_value();
//    mw_->log("LulcClassifier", usingStack ? "Using fused stack for supervised LULC." : "Standalone mode: using surface reflectance for supervised LULC.");
//    if (samplesCsvEdit_->text().isEmpty()) {
//        QMessageBox::warning(this, "Missing samples", "Choose a labeled samples CSV.");
//        return;
//    }
//    try {
//        const RasterCube& stack = *cube;
//        auto samples = ui_util::loadSampleCsv(samplesCsvEdit_->text().toStdString());

//        // Assign each distinct class name an integer label in encounter order.
//        std::map<std::string, int> classToLabel;
//        int nextLabel = 1;
//        for (const auto& kv : samples) classToLabel[kv.first] = nextLabel++;

//        std::vector<std::vector<float>> features;
//        std::vector<int> labels;
//        for (const auto& kv : samples) {
//            for (auto rc : kv.second) {
//                if (rc.first < 0 || rc.first >= stack.height || rc.second < 0 || rc.second >= stack.width) continue;
//                features.push_back(stack.pixelSpectrum(rc.first, rc.second));
//                labels.push_back(classToLabel[kv.first]);
//            }
//        }
//        if (features.empty()) {
//            QMessageBox::warning(this, "No usable samples", "No sample pixels fell inside the stack's extent.");
//            return;
//        }

//        auto result = LulcClassifier::supervised(stack, features, labels);
//        mw_->appState().lulcSupervisedA  = result.classified;
//        mw_->appState().lulcModel         = result.model;
//        mw_->appState().lulcClassToLabel  = classToLabel;  // persist for dateB preview

//        QString legend;
//        for (const auto& kv : classToLabel) legend += QString("%1=%2  ").arg(kv.second).arg(QString::fromStdString(kv.first));

//        mw_->log("LulcClassifier", QString("Supervised LULC complete (%1 classes). Legend: %2")
//                      .arg(classToLabel.size()).arg(legend));
//        {
//            using CS = RasterPreviewWidget::CategoryStyle;
//            std::map<int,CS> pal;
//            static const QColor kColors[] = {
//                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
//                {210,160,80},{160,50,180},{50,190,190},{240,140,30}
//            };
//            int ci = 0;
//            for (const auto& kv : classToLabel)
//                pal[kv.second] = CS{kColors[(ci++) % 8], QString::fromStdString(kv.first)};
//            pal[0] = CS{QColor(30,30,30), "Unclassified"};
//            mw_->previewWidget()->showCategorical(result.classified, 0, pal);
//        }
//        QMessageBox::information(this, "Supervised LULC complete", QString("Classes:\n%1").arg(legend));
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Supervised classification failed", e.what());
//    }
//}

//void LulcDialog::classifyDateB() {
//    if (!mw_->appState().lulcModel) {
//        QMessageBox::warning(this, "Train a model first", "Run the supervised classification (date A) first.");
//        return;
//    }
//    if (dateBStackEdit_->text().isEmpty()) {
//        QMessageBox::warning(this, "Missing input", "Choose a second-date 206-band stack raster.");
//        return;
//    }
//    try {
//        RasterCube stackB = RasterIO::loadCube(dateBStackEdit_->text().toStdString());

//        // Date B may come from a different scene (different acquisition, different size).
//        // The change matrix requires both classified rasters on the same grid.
//        // Resample Date B to Date A's grid before classifying — same approach as Step 5
//        // fused-data resampling. This is the root cause of "not on the same grid" errors.
//        const RasterCube* refGrid = nullptr;
//        if (mw_->appState().lulcSupervisedA)
//            refGrid = &(*mw_->appState().lulcSupervisedA);
//        else if (mw_->appState().stackFused)
//            refGrid = &(*mw_->appState().stackFused);
//        else if (mw_->appState().surfaceReflectance)
//            refGrid = &(*mw_->appState().surfaceReflectance);

//        if (refGrid && !stackB.sameGridAs(*refGrid)) {
//            mw_->log("LulcClassifier",
//                QString("Date B grid (%1x%2) differs from Date A (%3x%4) — resampling to match.")
//                    .arg(stackB.width).arg(stackB.height)
//                    .arg(refGrid->width).arg(refGrid->height));
//            stackB = RasterIO::resampleToGrid(stackB, *refGrid);
//        }

//        RasterCube classifiedB = mw_->appState().lulcModel->classifyCube(stackB);
//        mw_->appState().lulcSupervisedB = classifiedB;

//        // Show with same palette as dateA
//        {
//            using CS = RasterPreviewWidget::CategoryStyle;
//            std::map<int,CS> pal;
//            static const QColor kCols[] = {
//                {220,50,50},{34,100,34},{100,200,80},{60,120,220},{210,160,80},{160,50,180},{50,190,190},{240,140,30}
//            };
//            int ci = 0;
//            if (mw_->appState().lulcClassToLabel.has_value()) {
//                for (const auto& kv : *mw_->appState().lulcClassToLabel)
//                    pal[kv.second] = CS{kCols[(ci++) % 8], QString::fromStdString(kv.first)};
//            }
//            pal[0] = CS{QColor(30,30,30), "Unclassified"};
//            mw_->previewWidget()->showCategorical(classifiedB, 0, pal);
//        }

//        mw_->log("LulcClassifier", "Second-date classification complete using the date-A trained model.");
//        QMessageBox::information(this, "Second-date classification complete",
//            "Ready for Hyperspectral \u2192 Change Detection Matrix (Step 9).");
//        accept();
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Second-date classification failed", e.what());
//    }
//}


//#include "LulcDialog.h"
//#include "../MainWindow.h"
//#include "../RasterPreviewWidget.h"
//#include "../Utils.h"

//#include <QFormLayout>
//#include <QVBoxLayout>
//#include <QHBoxLayout>
//#include <QPushButton>
//#include <QFileDialog>
//#include <QMessageBox>
//#include <QLabel>
//#include <QGroupBox>
//#include <map>

//using namespace hsi;

//LulcDialog::LulcDialog(MainWindow* mainWindow, QWidget* parent) : QDialog(parent), mw_(mainWindow) {
//    setWindowTitle("Land Use / Land Cover Classification");
//    setMinimumWidth(560);

//    kSpin_ = new QSpinBox(); kSpin_->setRange(2, 20); kSpin_->setValue(4);
//    auto* unsupForm = new QFormLayout();
//    unsupForm->addRow("Number of clusters (k):", kSpin_);
//    auto* unsupRunBtn = new QPushButton("Run Unsupervised (k-means)");
//    auto* unsupGroup = new QGroupBox("Unsupervised classification");
//    auto* unsupLayout = new QVBoxLayout();
//    unsupLayout->addLayout(unsupForm);
//    unsupLayout->addWidget(unsupRunBtn);
//    unsupGroup->setLayout(unsupLayout);

//    samplesCsvEdit_ = new QLineEdit();
//    auto* browseSamplesBtn = new QPushButton("Browse...");
//    auto* samplesRow = new QHBoxLayout();
//    samplesRow->addWidget(samplesCsvEdit_);
//    samplesRow->addWidget(browseSamplesBtn);
//    auto* supForm = new QFormLayout();
//    supForm->addRow("Labeled samples CSV (class_name,row,col):", samplesRow);
//    auto* supRunBtn = new QPushButton("Run Supervised (SVM)");
//    auto* supGroup = new QGroupBox("Supervised classification");
//    auto* supLayout = new QVBoxLayout();
//    supLayout->addLayout(supForm);
//    supLayout->addWidget(supRunBtn);
//    supGroup->setLayout(supLayout);

//    dateBStackEdit_ = new QLineEdit();
//    auto* browseDateBBtn = new QPushButton("Browse...");
//    auto* dateBRow = new QHBoxLayout();
//    dateBRow->addWidget(dateBStackEdit_);
//    dateBRow->addWidget(browseDateBBtn);
//    auto* dateBForm = new QFormLayout();
//    dateBForm->addRow("Second-date 206-band stack (.tif):", dateBRow);
//    auto* dateBRunBtn = new QPushButton("Classify Second Date (reuses trained model)");
//    auto* dateBGroup = new QGroupBox("Two-date change input");
//    auto* dateBLayout = new QVBoxLayout();
//    dateBLayout->addLayout(dateBForm);
//    dateBLayout->addWidget(dateBRunBtn);
//    dateBGroup->setLayout(dateBLayout);

//    auto* layout = new QVBoxLayout(this);
//    layout->addWidget(unsupGroup);
//    layout->addWidget(supGroup);
//    layout->addWidget(dateBGroup);

//    connect(browseSamplesBtn, &QPushButton::clicked, this, &LulcDialog::browseSamplesCsv);
//    connect(browseDateBBtn, &QPushButton::clicked, this, &LulcDialog::browseDateBStack);
//    connect(unsupRunBtn, &QPushButton::clicked, this, &LulcDialog::runUnsupervised);
//    connect(supRunBtn, &QPushButton::clicked, this, &LulcDialog::runSupervised);
//    connect(dateBRunBtn, &QPushButton::clicked, this, &LulcDialog::classifyDateB);
//}

//void LulcDialog::browseSamplesCsv() {
//    QString path = QFileDialog::getOpenFileName(this, "Select samples CSV", QString(), "CSV files (*.csv);;All files (*)");
//    if (!path.isEmpty()) samplesCsvEdit_->setText(path);
//}

//void LulcDialog::browseDateBStack() {
//    QString path = QFileDialog::getOpenFileName(this, "Select second-date stack", QString(), "Raster files (*.tif *.tiff);;All files (*)");
//    if (!path.isEmpty()) dateBStackEdit_->setText(path);
//}

//// Returns the best available cube for LULC: fused stack if built, else surface reflectance.
//// This enables standalone operation when the fusion team TIFF has not been provided yet.
//static const hsi::RasterCube* bestCube(const AppState& s) {
//    if (s.stackFused)         return &(*s.stackFused);
//    if (s.surfaceReflectance) return &(*s.surfaceReflectance);
//    return nullptr;
//}

//void LulcDialog::runUnsupervised() {
//    const hsi::RasterCube* cube = bestCube(mw_->appState());
//    if (!cube) {
//        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
//        return;
//    }
//    bool usingStack = mw_->appState().stackFused.has_value();
//    mw_->log("LulcClassifier", usingStack ? "Using fused stack for LULC." : "Standalone mode: using surface reflectance (198 bands) for LULC.");
//    try {
//        std::map<int, std::string> clusterLabels;
//        RasterCube result = LulcClassifier::unsupervisedKMeans(*cube, kSpin_->value(), 5, &clusterLabels);
//        mw_->appState().lulcUnsupervised = result;
//        mw_->log("LulcClassifier", QString("Unsupervised LULC complete, k=%1.").arg(kSpin_->value()));
//        // Show with distinct colours per cluster, each labeled with a guessed
//        // land-cover type (from the cluster's centroid NDVI/NDWI/NDBI/BSI) so
//        // "Cluster 3" isn't just a meaningless number -- it's a starting point,
//        // not a certainty, so it's shown as "likely X" rather than stated flatly.
//        QString summaryText = QString("%1 clusters computed:\n\n").arg(kSpin_->value());
//        {
//            using CS = RasterPreviewWidget::CategoryStyle;
//            std::map<int,CS> pal;
//            static const QColor kColors[] = {
//                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
//                {210,160,80},{160,50,180},{50,190,190},{240,140,30},
//                {120,80,200},{80,160,80},{200,100,50},{50,120,180}
//            };
//            for (int i = 0; i < kSpin_->value(); ++i) {
//                QString guess = clusterLabels.count(i+1) ? QString::fromStdString(clusterLabels[i+1]) : QString();
//                QString label = guess.isEmpty() ? QString("Cluster %1").arg(i+1)
//                                                 : QString("Cluster %1 (%2)").arg(i+1).arg(guess);
//                pal[i+1] = CS{kColors[i % 12], label};
//                summaryText += QString("  Cluster %1: %2\n").arg(i+1).arg(guess.isEmpty() ? "?" : guess);
//            }
//            pal[0] = CS{QColor(30,30,30), "Unclassified"};
//            mw_->previewWidget()->showCategorical(result, 0, pal);
//        }
//        summaryText += "\nThese are automatic guesses based on each cluster's average spectral "
//                       "signature (NDVI/NDWI/NDBI/BSI) \u2014 spot-check a few pixels from each "
//                       "cluster against the image before relying on the label.";
//        QMessageBox::information(this, "Unsupervised LULC complete", summaryText);
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Unsupervised classification failed", e.what());
//    }
//}

//void LulcDialog::runSupervised() {
//    const hsi::RasterCube* cube = bestCube(mw_->appState());
//    if (!cube) {
//        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
//        return;
//    }
//    bool usingStack = mw_->appState().stackFused.has_value();
//    mw_->log("LulcClassifier", usingStack ? "Using fused stack for supervised LULC." : "Standalone mode: using surface reflectance for supervised LULC.");
//    if (samplesCsvEdit_->text().isEmpty()) {
//        QMessageBox::warning(this, "Missing samples", "Choose a labeled samples CSV.");
//        return;
//    }
//    try {
//        const RasterCube& stack = *cube;
//        auto samples = ui_util::loadSampleCsv(samplesCsvEdit_->text().toStdString());

//        // Assign each distinct class name an integer label in encounter order.
//        std::map<std::string, int> classToLabel;
//        int nextLabel = 1;
//        for (const auto& kv : samples) classToLabel[kv.first] = nextLabel++;

//        std::vector<std::vector<float>> features;
//        std::vector<int> labels;
//        for (const auto& kv : samples) {
//            for (auto rc : kv.second) {
//                if (rc.first < 0 || rc.first >= stack.height || rc.second < 0 || rc.second >= stack.width) continue;
//                features.push_back(stack.pixelSpectrum(rc.first, rc.second));
//                labels.push_back(classToLabel[kv.first]);
//            }
//        }
//        if (features.empty()) {
//            QMessageBox::warning(this, "No usable samples", "No sample pixels fell inside the stack's extent.");
//            return;
//        }

//        auto result = LulcClassifier::supervised(stack, features, labels);
//        mw_->appState().lulcSupervisedA = result.classified;
//        mw_->appState().lulcModel = result.model;

//        QString legend;
//        for (const auto& kv : classToLabel) legend += QString("%1=%2  ").arg(kv.second).arg(QString::fromStdString(kv.first));

//        mw_->log("LulcClassifier", QString("Supervised LULC complete (%1 classes). Legend: %2")
//                      .arg(classToLabel.size()).arg(legend));
//        {
//            using CS = RasterPreviewWidget::CategoryStyle;
//            std::map<int,CS> pal;
//            static const QColor kColors[] = {
//                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
//                {210,160,80},{160,50,180},{50,190,190},{240,140,30}
//            };
//            int ci = 0;
//            for (const auto& kv : classToLabel)
//                pal[kv.second] = CS{kColors[(ci++) % 8], QString::fromStdString(kv.first)};
//            pal[0] = CS{QColor(30,30,30), "Unclassified"};
//            mw_->previewWidget()->showCategorical(result.classified, 0, pal);
//        }
//        QMessageBox::information(this, "Supervised LULC complete", QString("Classes:\n%1").arg(legend));
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Supervised classification failed", e.what());
//    }
//}

//void LulcDialog::classifyDateB() {
//    if (!mw_->appState().lulcModel) {
//        QMessageBox::warning(this, "Train a model first", "Run the supervised classification (date A) first.");
//        return;
//    }
//    if (dateBStackEdit_->text().isEmpty()) {
//        QMessageBox::warning(this, "Missing input", "Choose a second-date 206-band stack raster.");
//        return;
//    }
//    try {
//        RasterCube stackB = RasterIO::loadCube(dateBStackEdit_->text().toStdString());
//        RasterCube classifiedB = mw_->appState().lulcModel->classifyCube(stackB);
//        mw_->appState().lulcSupervisedB = classifiedB;

//        mw_->log("LulcClassifier", "Second-date classification complete using the date-A trained model.");
//        mw_->previewWidget()->showSingleBand(classifiedB, 0);  // date B uses same trained palette
//        QMessageBox::information(this, "Second-date classification complete",
//            "Ready for Hyperspectral \u2192 Change Detection.");
//        accept();
//    } catch (const std::exception& e) {
//        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
//        QMessageBox::critical(this, "Second-date classification failed", e.what());
//    }
//}


#include "LulcDialog.h"
#include "../MainWindow.h"
#include "../RasterPreviewWidget.h"
#include "../Utils.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QGroupBox>
#include <map>

using namespace hsi;

LulcDialog::LulcDialog(MainWindow* mainWindow, QWidget* parent) : QDialog(parent), mw_(mainWindow) {
    setWindowTitle("Land Use / Land Cover Classification");
    setMinimumWidth(560);

    kSpin_ = new QSpinBox(); kSpin_->setRange(2, 20); kSpin_->setValue(4);
    auto* unsupForm = new QFormLayout();
    unsupForm->addRow("Number of clusters (k):", kSpin_);
    auto* unsupRunBtn = new QPushButton("Run Unsupervised (k-means)");
    auto* unsupGroup = new QGroupBox("Unsupervised classification");
    auto* unsupLayout = new QVBoxLayout();
    unsupLayout->addLayout(unsupForm);
    unsupLayout->addWidget(unsupRunBtn);
    unsupGroup->setLayout(unsupLayout);

    samplesCsvEdit_ = new QLineEdit();
    auto* browseSamplesBtn = new QPushButton("Browse...");
    auto* samplesRow = new QHBoxLayout();
    samplesRow->addWidget(samplesCsvEdit_);
    samplesRow->addWidget(browseSamplesBtn);
    auto* supForm = new QFormLayout();
    supForm->addRow("Labeled samples CSV (class_name,row,col):", samplesRow);
    auto* supRunBtn = new QPushButton("Run Supervised (SVM)");
    auto* supGroup = new QGroupBox("Supervised classification");
    auto* supLayout = new QVBoxLayout();
    supLayout->addLayout(supForm);
    supLayout->addWidget(supRunBtn);
    supGroup->setLayout(supLayout);

    dateBStackEdit_ = new QLineEdit();
    auto* browseDateBBtn = new QPushButton("Browse...");
    auto* dateBRow = new QHBoxLayout();
    dateBRow->addWidget(dateBStackEdit_);
    dateBRow->addWidget(browseDateBBtn);
    auto* dateBForm = new QFormLayout();
    dateBForm->addRow("Second-date 206-band stack (.tif):", dateBRow);
    auto* dateBRunBtn = new QPushButton("Classify Second Date (reuses trained model)");
    auto* dateBGroup = new QGroupBox("Two-date change input");
    auto* dateBLayout = new QVBoxLayout();
    dateBLayout->addLayout(dateBForm);
    dateBLayout->addWidget(dateBRunBtn);
    dateBGroup->setLayout(dateBLayout);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(unsupGroup);
    layout->addWidget(supGroup);
    layout->addWidget(dateBGroup);

    connect(browseSamplesBtn, &QPushButton::clicked, this, &LulcDialog::browseSamplesCsv);
    connect(browseDateBBtn, &QPushButton::clicked, this, &LulcDialog::browseDateBStack);
    connect(unsupRunBtn, &QPushButton::clicked, this, &LulcDialog::runUnsupervised);
    connect(supRunBtn, &QPushButton::clicked, this, &LulcDialog::runSupervised);
    connect(dateBRunBtn, &QPushButton::clicked, this, &LulcDialog::classifyDateB);
}

void LulcDialog::browseSamplesCsv() {
    QString path = QFileDialog::getOpenFileName(this, "Select samples CSV", QString(), "CSV files (*.csv);;All files (*)");
    if (!path.isEmpty()) samplesCsvEdit_->setText(path);
}

void LulcDialog::browseDateBStack() {
    QString path = QFileDialog::getOpenFileName(this, "Select second-date stack", QString(), "Raster files (*.tif *.tiff);;All files (*)");
    if (!path.isEmpty()) dateBStackEdit_->setText(path);
}

// Returns the best available cube for LULC: fused stack if built, else surface reflectance.
// This enables standalone operation when the fusion team TIFF has not been provided yet.
static const hsi::RasterCube* bestCube(const AppState& s) {
    if (s.stackFused)         return &(*s.stackFused);
    if (s.surfaceReflectance) return &(*s.surfaceReflectance);
    return nullptr;
}

void LulcDialog::runUnsupervised() {
    const hsi::RasterCube* cube = bestCube(mw_->appState());
    if (!cube) {
        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
        return;
    }
    bool usingStack = mw_->appState().stackFused.has_value();
    mw_->log("LulcClassifier", usingStack ? "Using fused stack for LULC." : "Standalone mode: using surface reflectance (198 bands) for LULC.");
    try {
        std::map<int, std::string> clusterLabels;
        RasterCube result = LulcClassifier::unsupervisedKMeans(*cube, kSpin_->value(), 5, &clusterLabels);
        mw_->appState().lulcUnsupervised = result;
        mw_->log("LulcClassifier", QString("Unsupervised LULC complete, k=%1.").arg(kSpin_->value()));
        // Show with distinct colours per cluster, each labeled with a guessed
        // land-cover type (from the cluster's centroid NDVI/NDWI/NDBI/BSI) so
        // "Cluster 3" isn't just a meaningless number -- it's a starting point,
        // not a certainty, so it's shown as "likely X" rather than stated flatly.
        QString summaryText = QString("%1 clusters computed:\n\n").arg(kSpin_->value());
        {
            using CS = RasterPreviewWidget::CategoryStyle;
            std::map<int,CS> pal;
            static const QColor kColors[] = {
                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
                {210,160,80},{160,50,180},{50,190,190},{240,140,30},
                {120,80,200},{80,160,80},{200,100,50},{50,120,180}
            };
            for (int i = 0; i < kSpin_->value(); ++i) {
                QString guess = clusterLabels.count(i+1) ? QString::fromStdString(clusterLabels[i+1]) : QString();
                QString label = guess.isEmpty() ? QString("Cluster %1").arg(i+1)
                                                 : QString("Cluster %1 (%2)").arg(i+1).arg(guess);
                pal[i+1] = CS{kColors[i % 12], label};
                summaryText += QString("  Cluster %1: %2\n").arg(i+1).arg(guess.isEmpty() ? "?" : guess);
            }
            pal[0] = CS{QColor(30,30,30), "Unclassified"};
            mw_->previewWidget()->showCategorical(result, 0, pal);
        }
        summaryText += "\nThese are automatic guesses based on each cluster's average spectral "
                       "signature (NDVI/NDWI/NDBI/BSI) \u2014 spot-check a few pixels from each "
                       "cluster against the image before relying on the label.";
        QMessageBox::information(this, "Unsupervised LULC complete", summaryText);
    } catch (const std::exception& e) {
        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Unsupervised classification failed", e.what());
    }
}

void LulcDialog::runSupervised() {
    const hsi::RasterCube* cube = bestCube(mw_->appState());
    if (!cube) {
        QMessageBox::warning(this, "Step 2 required", "Run Step 2 (DN \u2192 Surface Reflectance) first.");
        return;
    }
    bool usingStack = mw_->appState().stackFused.has_value();
    mw_->log("LulcClassifier", usingStack ? "Using fused stack for supervised LULC." : "Standalone mode: using surface reflectance for supervised LULC.");
    if (samplesCsvEdit_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing samples", "Choose a labeled samples CSV.");
        return;
    }
    try {
        const RasterCube& stack = *cube;
        auto samples = ui_util::loadSampleCsv(samplesCsvEdit_->text().toStdString());

        // Assign each distinct class name an integer label in encounter order.
        std::map<std::string, int> classToLabel;
        int nextLabel = 1;
        for (const auto& kv : samples) classToLabel[kv.first] = nextLabel++;

        std::vector<std::vector<float>> features;
        std::vector<int> labels;
        for (const auto& kv : samples) {
            for (auto rc : kv.second) {
                if (rc.first < 0 || rc.first >= stack.height || rc.second < 0 || rc.second >= stack.width) continue;
                features.push_back(stack.pixelSpectrum(rc.first, rc.second));
                labels.push_back(classToLabel[kv.first]);
            }
        }
        if (features.empty()) {
            QMessageBox::warning(this, "No usable samples", "No sample pixels fell inside the stack's extent.");
            return;
        }

        auto result = LulcClassifier::supervised(stack, features, labels);
        mw_->appState().lulcSupervisedA  = result.classified;
        mw_->appState().lulcModel         = result.model;
        mw_->appState().lulcClassToLabel  = classToLabel;  // persist for dateB preview

        QString legend;
        for (const auto& kv : classToLabel) legend += QString("%1=%2  ").arg(kv.second).arg(QString::fromStdString(kv.first));

        mw_->log("LulcClassifier", QString("Supervised LULC complete (%1 classes). Legend: %2")
                      .arg(classToLabel.size()).arg(legend));
        {
            using CS = RasterPreviewWidget::CategoryStyle;
            std::map<int,CS> pal;
            static const QColor kColors[] = {
                {220,50,50},{34,100,34},{100,200,80},{60,120,220},
                {210,160,80},{160,50,180},{50,190,190},{240,140,30}
            };
            int ci = 0;
            for (const auto& kv : classToLabel)
                pal[kv.second] = CS{kColors[(ci++) % 8], QString::fromStdString(kv.first)};
            pal[0] = CS{QColor(30,30,30), "Unclassified"};
            mw_->previewWidget()->showCategorical(result.classified, 0, pal);
        }
        QMessageBox::information(this, "Supervised LULC complete", QString("Classes:\n%1").arg(legend));
    } catch (const std::exception& e) {
        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Supervised classification failed", e.what());
    }
}

void LulcDialog::classifyDateB() {
    if (!mw_->appState().lulcModel) {
        QMessageBox::warning(this, "Train a model first", "Run the supervised classification (date A) first.");
        return;
    }
    if (dateBStackEdit_->text().isEmpty()) {
        QMessageBox::warning(this, "Missing input", "Choose a second-date 206-band stack raster.");
        return;
    }
    try {
        RasterCube stackB = RasterIO::loadCube(dateBStackEdit_->text().toStdString());

        // Date B may come from a different scene (different acquisition, different size).
        // The change matrix requires both classified rasters on the same grid.
        // Resample Date B to Date A's grid before classifying — same approach as Step 5
        // fused-data resampling. This is the root cause of "not on the same grid" errors.
        const RasterCube* refGrid = nullptr;
        if (mw_->appState().lulcSupervisedA)
            refGrid = &(*mw_->appState().lulcSupervisedA);
        else if (mw_->appState().stackFused)
            refGrid = &(*mw_->appState().stackFused);
        else if (mw_->appState().surfaceReflectance)
            refGrid = &(*mw_->appState().surfaceReflectance);

        if (refGrid && !stackB.sameGridAs(*refGrid)) {
            mw_->log("LulcClassifier",
                QString("Date B grid (%1x%2) differs from Date A (%3x%4) — resampling to match.")
                    .arg(stackB.width).arg(stackB.height)
                    .arg(refGrid->width).arg(refGrid->height));
            stackB = RasterIO::resampleToGrid(stackB, *refGrid);
        }

        RasterCube classifiedB = mw_->appState().lulcModel->classifyCube(stackB);
        mw_->appState().lulcSupervisedB = classifiedB;

        // Show with same palette as dateA
        {
            using CS = RasterPreviewWidget::CategoryStyle;
            std::map<int,CS> pal;
            static const QColor kCols[] = {
                {220,50,50},{34,100,34},{100,200,80},{60,120,220},{210,160,80},{160,50,180},{50,190,190},{240,140,30}
            };
            int ci = 0;
            if (mw_->appState().lulcClassToLabel.has_value()) {
                for (const auto& kv : *mw_->appState().lulcClassToLabel)
                    pal[kv.second] = CS{kCols[(ci++) % 8], QString::fromStdString(kv.first)};
            }
            pal[0] = CS{QColor(30,30,30), "Unclassified"};
            mw_->previewWidget()->showCategorical(classifiedB, 0, pal);
        }

        mw_->log("LulcClassifier", "Second-date classification complete using the date-A trained model.");
        QMessageBox::information(this, "Second-date classification complete",
            "Ready for Hyperspectral \u2192 Change Detection Matrix (Step 9).");
        accept();
    } catch (const std::exception& e) {
        mw_->log("LulcClassifier", QString("ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Second-date classification failed", e.what());
    }
}
