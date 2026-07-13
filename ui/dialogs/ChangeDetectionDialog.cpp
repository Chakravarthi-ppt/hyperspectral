#include "ChangeDetectionDialog.h"
#include "../MainWindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QHeaderView>
#include <QColor>

using namespace hsi;

ChangeDetectionDialog::ChangeDetectionDialog(MainWindow* mainWindow, QWidget* parent)
    : QDialog(parent), mw_(mainWindow) {
    setWindowTitle("Land-Use Change Matrix");
    setMinimumSize(640, 420);

    pixelAreaSpin_ = new QDoubleSpinBox();
    pixelAreaSpin_->setRange(1.0, 1000000.0);
    pixelAreaSpin_->setValue(900.0);
    pixelAreaSpin_->setSuffix(" sq.m per pixel");

    auto* form = new QFormLayout();
    form->addRow("Pixel area:", pixelAreaSpin_);

    auto* infoLabel = new QLabel(
        "Uses the supervised LULC results from Hyperspectral \u2192 LULC Classification "
        "(date A and date B) computed earlier in this session.<br>"
        "<b>Rows</b> = the class a pixel WAS in date A. <b>Columns</b> = the class it IS in date B. "
        "The <b>diagonal</b> (highlighted green) = pixels that stayed the same class -- everything "
        "<b>off the diagonal</b> (highlighted amber) is a change from one class to another.", this);
    infoLabel->setWordWrap(true);
    infoLabel->setTextFormat(Qt::RichText);
    infoLabel->setStyleSheet("color: #555;");

    auto* runBtn = new QPushButton("Compute Change Matrix");
    auto* saveBtn = new QPushButton("Save CSV...");
    auto* viewABtn = new QPushButton("View Date A");
    auto* viewBBtn = new QPushButton("View Date B");
    auto* viewChangeBtn = new QPushButton("View Change Map");
    viewChangeBtn->setToolTip("Shows WHERE pixels changed class (white) vs stayed the same (black), "
                               "pixel-for-pixel -- the visual counterpart to the numeric matrix below.");

    table_ = new QTableWidget(this);

    summaryLabel_ = new QLabel(this);
    summaryLabel_->setWordWrap(true);
    summaryLabel_->setStyleSheet("font-weight:bold;padding:4px;");

    auto* btnRow = new QHBoxLayout();
    btnRow->addWidget(runBtn);
    btnRow->addWidget(saveBtn);

    auto* viewRow = new QHBoxLayout();
    viewRow->addWidget(viewABtn);
    viewRow->addWidget(viewBBtn);
    viewRow->addWidget(viewChangeBtn);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(infoLabel);
    layout->addLayout(form);
    layout->addLayout(viewRow);
    layout->addLayout(btnRow);
    layout->addWidget(summaryLabel_);
    layout->addWidget(table_);

    connect(runBtn, &QPushButton::clicked, this, &ChangeDetectionDialog::run);
    connect(saveBtn, &QPushButton::clicked, this, &ChangeDetectionDialog::saveCsv);
    connect(viewABtn, &QPushButton::clicked, this, [this]() {
        if (!mw_->appState().lulcSupervisedA) {
            QMessageBox::warning(this, "Not available", "Run Step 8 (LULC, date A) first.");
            return;
        }
        mw_->previewWidget()->showSingleBand(*mw_->appState().lulcSupervisedA, 0);
    });
    connect(viewBBtn, &QPushButton::clicked, this, [this]() {
        if (!mw_->appState().lulcSupervisedB) {
            QMessageBox::warning(this, "Not available", "Run Step 8 (LULC, date B) first.");
            return;
        }
        mw_->previewWidget()->showSingleBand(*mw_->appState().lulcSupervisedB, 0);
    });
    connect(viewChangeBtn, &QPushButton::clicked, this, [this]() {
        if (!mw_->appState().lulcSupervisedA || !mw_->appState().lulcSupervisedB) {
            QMessageBox::warning(this, "Missing classified rasters",
                "Run Hyperspectral \u2192 LULC Classification for both date A and date B first.");
            return;
        }
        try {
            hsi::RasterCube lulcA = *mw_->appState().lulcSupervisedA;
            hsi::RasterCube lulcB = *mw_->appState().lulcSupervisedB;
            if (!lulcA.sameGridAs(lulcB))
                lulcB = hsi::RasterIO::resampleToGrid(lulcB, lulcA);
            RasterCube changeMap = ChangeDetector::computeChangeMap(lulcA, lulcB);
            using CS = RasterPreviewWidget::CategoryStyle;
            mw_->previewWidget()->showCategorical(changeMap, 0, {
                {0, CS{QColor(30,30,30), "Unchanged"}},
                {1, CS{QColor(255,80,0), "Changed"  }}
            });
            mw_->log("ChangeDetector", "Displaying change map (white = changed pixel, black = unchanged).");
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Change map failed", e.what());
        }
    });
}

void ChangeDetectionDialog::run() {
    if (!mw_->appState().lulcSupervisedA || !mw_->appState().lulcSupervisedB) {
        QMessageBox::warning(this, "Missing classified rasters",
            "Run Hyperspectral \u2192 LULC Classification for both date A (supervised) and date B "
            "(classify second date) first.");
        return;
    }
    try {
        // Safety net: resample dateB to dateA grid if they differ.
        // This should already be done in classifyDateB(), but handles the case
        // where dateA and dateB were classified in different sessions or from
        // different source scenes without the resampling step.
        hsi::RasterCube lulcA = *mw_->appState().lulcSupervisedA;
        hsi::RasterCube lulcB = *mw_->appState().lulcSupervisedB;
        if (!lulcA.sameGridAs(lulcB)) {
            mw_->log("ChangeDetector",
                QString("Grid mismatch detected: A=%1x%2, B=%3x%4. Resampling B to A grid.")
                    .arg(lulcA.width).arg(lulcA.height)
                    .arg(lulcB.width).arg(lulcB.height));
            lulcB = hsi::RasterIO::resampleToGrid(lulcB, lulcA);
        }
        lastResult_ = ChangeDetector::computeChangeMatrix(lulcA, lulcB, pixelAreaSpin_->value());
        hasResult_ = true;
        mw_->appState().changeMatrix = lastResult_;

        int n = static_cast<int>(lastResult_.classIds.size());
        table_->setRowCount(n);
        table_->setColumnCount(n + 1);

        // Build id→name map from stored legend; fall back to "class N"
        std::map<int,QString> idToName;
        idToName[0] = "Unclassified";
        if (mw_->appState().lulcClassToLabel.has_value()) {
            for (const auto& kv : *mw_->appState().lulcClassToLabel)
                idToName[kv.second] = QString::fromStdString(kv.first);
        }
        auto className = [&](int id) -> QString {
            auto it = idToName.find(id);
            return (it != idToName.end()) ? it->second : QString("class %1").arg(id);
        };

        QStringList headers;
        headers << "From \\ To";
        for (int id : lastResult_.classIds) headers << className(id);
        table_->setHorizontalHeaderLabels(headers);

        long totalPixels = 0, changedPixels = 0;
        for (int i = 0; i < n; ++i) {
            table_->setItem(i, 0, new QTableWidgetItem(className(lastResult_.classIds[i])));
            for (int j = 0; j < n; ++j) {
                long count = lastResult_.matrix[i][j];
                totalPixels += count;
                auto* item = new QTableWidgetItem(QString::number(count));
                if (i == j) {
                    item->setBackground(QColor(200, 240, 200)); // unchanged -- diagonal
                } else {
                    changedPixels += count;
                    if (count > 0) item->setBackground(QColor(250, 220, 170)); // changed -- off-diagonal
                }
                table_->setItem(i, j + 1, item);
            }
        }
        table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        double changedPct = totalPixels > 0 ? (100.0 * changedPixels / totalPixels) : 0.0;
        double changedAreaSqm = changedPixels * pixelAreaSpin_->value();
        summaryLabel_->setText(QString(
            "%1 of %2 pixels changed class between date A and date B "
            "(%3%, \u2248 %4 sq. m). Green diagonal cells = unchanged; amber cells = changed "
            "(row = date-A class, column = date-B class it became).")
            .arg(changedPixels).arg(totalPixels)
            .arg(changedPct, 0, 'f', 1)
            .arg(changedAreaSqm, 0, 'f', 0));

        mw_->log("ChangeDetector", QString("Change matrix computed over %1 classes; %2%% of pixels changed.")
                                        .arg(n).arg(changedPct, 0, 'f', 1));
        QMessageBox::information(this, "Change matrix computed",
            QString("%1 classes found across both dates.\n%2%% of the area changed class.")
                .arg(n).arg(changedPct, 0, 'f', 1));
    } catch (const std::exception& e) {
        mw_->log("ChangeDetector", QString("ERROR: %1").arg(e.what()));
        QMessageBox::critical(this, "Change detection failed", e.what());
    }
}

void ChangeDetectionDialog::saveCsv() {
    if (!hasResult_) {
        QMessageBox::warning(this, "Nothing to save", "Compute the change matrix first.");
        return;
    }
    QString path = QFileDialog::getSaveFileName(this, "Save change matrix CSV", QString(), "CSV files (*.csv)");
    if (path.isEmpty()) return;
    try {
        ChangeDetector::saveMatrixCsv(lastResult_, path.toStdString());
        mw_->log("ChangeDetector", QString("Saved to '%1'.").arg(path));
        QMessageBox::information(this, "Saved", QString("Change matrix saved to:\n%1").arg(path));
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Save failed", e.what());
    }
}
