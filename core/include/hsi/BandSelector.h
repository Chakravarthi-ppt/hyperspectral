#pragma once
#include "hsi/Types.h"

namespace hsi {

// Implements the band-selection rule from the workbook:
//   VNIR calibrated:  bands 8-57   (kept, divide by 40 upstream)
//   overlap zone:     bands 58-76  (omitted)
//   SWIR calibrated:  bands 77-224 (kept, divide by 80 upstream)
//   SWIR tail:        bands 225-242 (omitted, uncalibrated)
class BandSelector {
public:
    struct Rule {
        int vnirStartBand = 8,  vnirEndBand = 57;
        int swirStartBand = 77, swirEndBand = 224;
    };

    // Requires `fullCube.bandNumbers` to hold the original sensor band index
    // per stored band. Returns a new cube containing only the kept bands, in
    // ascending sensor-band order.
    static RasterCube selectCalibratedBands(const RasterCube& fullCube, const Rule& rule);
    static RasterCube selectCalibratedBands(const RasterCube& fullCube); // uses default Rule
};

} // namespace hsi
