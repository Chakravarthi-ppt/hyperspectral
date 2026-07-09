#pragma once
#include "hsi/Types.h"
#include "hsi/Orthorectifier.h"
#include "hsi/RadiometricCalibrator.h"
#include "hsi/BandSelector.h"
#include "hsi/AtmosphericCorrector.h"
#include "hsi/SpectralLibrary.h"
#include <map>

namespace hsi {

// Wires Orthorectifier -> BandSelector -> RadiometricCalibrator ->
// AtmosphericCorrector into the single preprocessing chain described in the
// workbook: ortho (if needed) -> DN->radiance -> radiance->TOA reflectance
// -> surface reflectance (DOS, or ELM if calibration targets are supplied).
//
// Stages downstream of this (mask, PCA, stacking, built-up classification,
// LULC, change detection) are deliberately left as separate calls -- each
// UI dialog drives the specific core class it needs directly, since they
// branch (built-up path vs LULC path) rather than chain linearly.
class Pipeline {
public:
    struct PreprocessConfig {
        std::string inputPath;
        std::string orthoOutputPath;          // used only if warping is actually needed
        Orthorectifier::Options orthoOptions;
        BandSelector::Rule bandRule;
        RadiometricCalibrator::Options calibrationOptions;
        AtmosphericCorrector::SolarGeometry solarGeometry;
        std::vector<double> esunPerBand;       // see EsunTable
        double darkObjectPercentile = 0.5;

        // --- ELM only (ignored unless correctionMethod == ELM) ---
        AtmosphericCorrector::CorrectionMethod correctionMethod = AtmosphericCorrector::CorrectionMethod::DOS;
        // Must contain a "dark" and a "bright" entry (row,col pixel
        // coordinates in the TOA cube -- same width/height as the input
        // scene). Typically loaded from a CSV via ui_util::loadSampleCsv.
        std::map<std::string, std::vector<std::pair<int, int>>> elmSamplePixels;
        double elmDarkKnownReflectance   = 0.01; // e.g. deep water
        double elmBrightKnownReflectance = 0.60; // e.g. concrete/sand
    };

    struct PreprocessResult {
        bool wasAlreadyOrthorectified = false;
        RasterCube surfaceReflectance;         // band-selected, calibrated, atmospherically corrected
    };

    static PreprocessResult preprocess(const PreprocessConfig& cfg);
};

} // namespace hsi
