TEMPLATE = app
TARGET = hsi_desktop
CONFIG += c++17
QT += widgets

INCLUDEPATH += \
    $$PWD/core/include \
    $$PWD/ui

HEADERS += \
    core/include/hsi/AtmosphericCorrector.h \
    core/include/hsi/BandSelector.h \
    core/include/hsi/BandStacker.h \
    core/include/hsi/BuiltUpClassifier.h \
    core/include/hsi/ChangeDetector.h \
    core/include/hsi/Database.h \
    core/include/hsi/EsunTable.h \
    core/include/hsi/Hsi.h \
    core/include/hsi/Logger.h \
    core/include/hsi/LulcClassifier.h \
    core/include/hsi/Orthorectifier.h \
    core/include/hsi/PcaReducer.h \
    core/include/hsi/Pipeline.h \
    core/include/hsi/RadiometricCalibrator.h \
    core/include/hsi/RasterIO.h \
    core/include/hsi/RasterToVector.h \
    core/include/hsi/SamClassifier.h \
    core/include/hsi/SpectralLibrary.h \
    core/include/hsi/SurfaceObjectMask.h \
    core/include/hsi/SvmModel.h \
    core/include/hsi/SpectralIndices.h \
    core/include/hsi/LandCoverMapper.h \
    core/include/hsi/RxDetector.h \
    core/include/hsi/Types.h \
    ui/MainWindow.h \
    ui/PipelineWorker.h \
    ui/ProgressDialog.h \
    ui/RasterPreviewWidget.h \
    ui/Utils.h \
    ui/dialogs/PreprocessingDialog.h \
    ui/dialogs/SurfaceObjectMaskDialog.h \
    ui/dialogs/PcaStackDialog.h \
    ui/dialogs/BuiltUpClassificationDialog.h \
    ui/dialogs/LulcDialog.h \
    ui/dialogs/ChangeDetectionDialog.h \
    ui/dialogs/LandCoverMapperDialog.h \
    ui/dialogs/AnomalyDetectorDialog.h \

SOURCES += \
    core/src/AtmosphericCorrector.cpp \
    core/src/BandSelector.cpp \
    core/src/BandStacker.cpp \
    core/src/BuiltUpClassifier.cpp \
    core/src/ChangeDetector.cpp \
    core/src/Database.cpp \
    core/src/EsunTable.cpp \
    core/src/LulcClassifier.cpp \
    core/src/Orthorectifier.cpp \
    core/src/PcaReducer.cpp \
    core/src/Pipeline.cpp \
    core/src/RadiometricCalibrator.cpp \
    core/src/RasterIO.cpp \
    core/src/RasterToVector.cpp \
    core/src/SamClassifier.cpp \
    core/src/SpectralLibrary.cpp \
    core/src/SurfaceObjectMask.cpp \
    core/src/SvmModel.cpp \
    core/src/SpectralIndices.cpp \
    core/src/LandCoverMapper.cpp \
    core/src/RxDetector.cpp \
    ui/main.cpp \
    ui/MainWindow.cpp \
    ui/RasterPreviewWidget.cpp \
    ui/Utils.cpp \
    ui/dialogs/PreprocessingDialog.cpp \
    ui/dialogs/SurfaceObjectMaskDialog.cpp \
    ui/dialogs/PcaStackDialog.cpp \
    ui/dialogs/BuiltUpClassificationDialog.cpp \
    ui/dialogs/LulcDialog.cpp \
    ui/dialogs/ChangeDetectionDialog.cpp \
    ui/dialogs/LandCoverMapperDialog.cpp \
    ui/dialogs/AnomalyDetectorDialog.cpp

# Note: tools/cli_test/main.cpp is a separate standalone smoke-test program
# (its own main()) and is intentionally NOT part of this .pro -- see the
# README for the one-line command to compile it on its own if you want it.

unix:!macx {
    QMAKE_CXXFLAGS += -Wall -Wextra

    # --- GDAL: gdal-config ships with libgdal-dev and is reliable, so it's
    # the primary path. Hardcoded fallback covers the rare box without it
    # on PATH. ---
    GDAL_CFLAGS = $$system(gdal-config --cflags 2>/dev/null)
    isEmpty(GDAL_CFLAGS) {
        message("gdal-config not found -- falling back to /usr/include/gdal and -lgdal. Edit this block if your GDAL lives elsewhere.")
        INCLUDEPATH += /usr/include/gdal
        LIBS += -lgdal
    } else {
        QMAKE_CXXFLAGS += $$GDAL_CFLAGS
        LIBS += $$system(gdal-config --libs)
    }

    # --- OpenCV: pkg-config's opencv4.pc is frequently MISSING from Ubuntu
    # 20.04's libopencv-dev package even though OpenCV itself is installed
    # fine -- that's what "opencv4 development package not found" means.
    # Try pkg-config first; if it's not there, fall back to the standard
    # Ubuntu install location and link only the two modules this project
    # actually uses (core + ml), rather than failing outright. ---
    OPENCV_HAS_PC = $$system(pkg-config --exists opencv4 2>/dev/null && echo yes)
    isEmpty(OPENCV_HAS_PC) {
        OPENCV_HAS_PC2 = $$system(pkg-config --exists opencv 2>/dev/null && echo yes)
        isEmpty(OPENCV_HAS_PC2) {
            message("pkg-config could not find opencv4 or opencv -- falling back to /usr/include/opencv4 and -lopencv_core -lopencv_ml. If your OpenCV is installed elsewhere, edit this block.")
            exists(/usr/include/opencv4) {
                INCLUDEPATH += /usr/include/opencv4
            } else {
                INCLUDEPATH += /usr/local/include/opencv4
            }
            LIBS += -lopencv_core -lopencv_ml
        } else {
            QMAKE_CXXFLAGS += $$system(pkg-config --cflags opencv)
            LIBS += $$system(pkg-config --libs opencv)
        }
    } else {
        QMAKE_CXXFLAGS += $$system(pkg-config --cflags opencv4)
        LIBS += $$system(pkg-config --libs opencv4)
    }

    # --- Eigen3 (header-only) ---
    EIGEN_CFLAGS = $$system(pkg-config --cflags eigen3 2>/dev/null)
    isEmpty(EIGEN_CFLAGS) {
        message("pkg-config could not find eigen3 -- falling back to /usr/include/eigen3.")
        INCLUDEPATH += /usr/include/eigen3
    } else {
        QMAKE_CXXFLAGS += $$EIGEN_CFLAGS
    }

    # --- PostgreSQL (libpq): used by hsi::Database for the local-then-DB
    # raster cache. Found via pg_config rather than pkg-config/a bare -lpq,
    # since Postgres here was built from source into a non-standard prefix
    # (/usr/local/pgsql) that those wouldn't resolve on their own. ---
    PG_CONFIG_BIN = $$system(which pg_config 2>/dev/null)
    isEmpty(PG_CONFIG_BIN) {
        message("pg_config not found -- falling back to /usr/local/pgsql/include and -lpq. Edit this block if your PostgreSQL lives elsewhere.")
        INCLUDEPATH += /usr/local/pgsql/include
        LIBS += -L/usr/local/pgsql/lib -lpq
    } else {
        INCLUDEPATH += $$system(pg_config --includedir)
        LIBS += -L$$system(pg_config --libdir) -lpq
    }

    LIBS += -lpthread
}

# ---------------------------------------------------------------
# Output directories (keeps build clutter out of the source tree)
# ---------------------------------------------------------------
DESTDIR     = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
RCC_DIR     = $$PWD/build/rcc
UI_DIR      = $$PWD/build/ui
