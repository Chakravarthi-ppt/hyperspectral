#include "hsi/Pipeline.h"
#include "hsi/RasterIO.h"
#include "hsi/Logger.h"

namespace hsi {

namespace {
RasterCube applyCorrection(const RasterCube& toaReflectance, const Pipeline::PreprocessConfig& cfg) {
    if (cfg.correctionMethod == AtmosphericCorrector::CorrectionMethod::DOS) {
        return AtmosphericCorrector::toaToSurfaceReflectanceDOS(toaReflectance, cfg.darkObjectPercentile);
    }

    // --- ELM ---
    auto darkIt   = cfg.elmSamplePixels.find("dark");
    auto brightIt = cfg.elmSamplePixels.find("bright");
    if (darkIt == cfg.elmSamplePixels.end() || darkIt->second.empty() ||
        brightIt == cfg.elmSamplePixels.end() || brightIt->second.empty()) {
        throw HsiError("Pipeline: ELM correction selected but the sample set needs a non-empty 'dark' "
                        "and 'bright' class (e.g. CSV rows like 'dark,120,340' and 'bright,55,210').");
    }

    // Bounds-check every sample pixel against the TOA cube before handing
    // them to SpectralLibrary -- RasterCube::at()/pixelSpectrum() do not
    // range-check, so a typo'd row/col here would otherwise read out of
    // the underlying buffer instead of failing cleanly.
    for (const auto& classEntry : cfg.elmSamplePixels) {
        for (const auto& rc : classEntry.second) {
            if (rc.first < 0 || rc.first >= toaReflectance.height ||
                rc.second < 0 || rc.second >= toaReflectance.width) {
                throw HsiError("Pipeline: ELM sample pixel for class '" + classEntry.first + "' at (row=" +
                                std::to_string(rc.first) + ", col=" + std::to_string(rc.second) +
                                ") is outside the " + std::to_string(toaReflectance.width) + "x" +
                                std::to_string(toaReflectance.height) + " scene.");
            }
        }
    }

    SpectralLibrary lib = SpectralLibrary::buildFromSamples(toaReflectance, cfg.elmSamplePixels);
    const SpectralSignature* darkSig = nullptr;
    const SpectralSignature* brightSig = nullptr;
    for (const auto& sig : lib.signatures) {
        if (sig.className == "dark") darkSig = &sig;
        if (sig.className == "bright") brightSig = &sig;
    }
    if (!darkSig || !brightSig) {
        throw HsiError("Pipeline: could not build 'dark'/'bright' calibration spectra from the sample set.");
    }

    AtmosphericCorrector::CalibrationTarget dark;
    dark.meanToaPerBand = darkSig->meanReflectance;
    dark.knownReflectance = cfg.elmDarkKnownReflectance;

    AtmosphericCorrector::CalibrationTarget bright;
    bright.meanToaPerBand = brightSig->meanReflectance;
    bright.knownReflectance = cfg.elmBrightKnownReflectance;

    return AtmosphericCorrector::toaToSurfaceReflectanceELM(toaReflectance, dark, bright);
}
} // namespace

Pipeline::PreprocessResult Pipeline::preprocess(const PreprocessConfig& cfg) {
    Logger::log("Pipeline", "=== Preprocessing started: " + cfg.inputPath + " ===");

    auto orthoOutcome = Orthorectifier::ensureOrthorectified(cfg.inputPath, cfg.orthoOutputPath, cfg.orthoOptions);

    RasterCube dnCube = RasterIO::loadCube(orthoOutcome.outputPath);
    RasterCube selected = BandSelector::selectCalibratedBands(dnCube, cfg.bandRule);
    RasterCube radiance = RadiometricCalibrator::dnToRadiance(selected, cfg.calibrationOptions);
    RasterCube toaReflectance = AtmosphericCorrector::radianceToToaReflectance(radiance, cfg.solarGeometry, cfg.esunPerBand);
    RasterCube surfaceReflectance = applyCorrection(toaReflectance, cfg);

    Logger::log("Pipeline", "=== Preprocessing complete: " + std::to_string(surfaceReflectance.bands) +
                " bands, " + std::to_string(surfaceReflectance.width) + "x" + std::to_string(surfaceReflectance.height) + " ===");

    PreprocessResult result;
    result.wasAlreadyOrthorectified = orthoOutcome.wasAlreadyOrthorectified;
    result.surfaceReflectance = std::move(surfaceReflectance);
    return result;
}

} // namespace hsi
