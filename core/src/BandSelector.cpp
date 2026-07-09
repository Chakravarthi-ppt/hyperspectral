#include "hsi/BandSelector.h"
#include "hsi/Logger.h"
#include <algorithm>

namespace hsi {

RasterCube BandSelector::selectCalibratedBands(const RasterCube& fullCube) {
    return selectCalibratedBands(fullCube, Rule());
}

RasterCube BandSelector::selectCalibratedBands(const RasterCube& fullCube, const Rule& rule) {
    if (fullCube.bandNumbers.empty() || static_cast<int>(fullCube.bandNumbers.size()) != fullCube.bands) {
        throw HsiError("BandSelector: cube has no per-band sensor band numbers.");
    }

    std::vector<int> keepIndices;
    for (int b = 0; b < fullCube.bands; ++b) {
        int sb = fullCube.bandNumbers[b];
        bool inVnir = sb >= rule.vnirStartBand && sb <= rule.vnirEndBand;
        bool inSwir = sb >= rule.swirStartBand && sb <= rule.swirEndBand;
        if (inVnir || inSwir) keepIndices.push_back(b);
    }

    if (keepIndices.empty()) {
        throw HsiError("BandSelector: no bands matched the VNIR/SWIR calibrated ranges.");
    }

    RasterCube out;
    out.allocate(fullCube.width, fullCube.height, static_cast<int>(keepIndices.size()));
    out.geoTransform = fullCube.geoTransform;
    out.projectionWkt = fullCube.projectionWkt;

    for (size_t i = 0; i < keepIndices.size(); ++i) {
        int srcBand = keepIndices[i];
        size_t srcBase = static_cast<size_t>(srcBand) * fullCube.width * fullCube.height;
        size_t dstBase = i * fullCube.width * fullCube.height;
        std::copy(fullCube.data.begin() + srcBase, fullCube.data.begin() + srcBase + fullCube.pixelCount(),
                  out.data.begin() + dstBase);
        out.bandNumbers[i] = fullCube.bandNumbers[srcBand];
        out.bandNames[i] = fullCube.bandNames[srcBand];
    }

    Logger::log("BandSelector", "Kept " + std::to_string(keepIndices.size()) + " of " +
                std::to_string(fullCube.bands) + " bands (dropped overlap zone " +
                std::to_string(rule.vnirEndBand + 1) + "-" + std::to_string(rule.swirStartBand - 1) +
                " and any band outside " + std::to_string(rule.vnirStartBand) + "-" +
                std::to_string(rule.swirEndBand) + ").");
    return out;
}

} // namespace hsi
