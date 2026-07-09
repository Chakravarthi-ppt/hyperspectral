#include "hsi/BandStacker.h"
#include "hsi/Logger.h"
#include <algorithm>

namespace hsi {

RasterCube BandStacker::stack(const std::vector<Input>& inputs) {
    if (inputs.empty()) throw HsiError("BandStacker: no inputs given.");

    const RasterCube* first = inputs[0].cube;
    int totalBands = 0;
    for (const auto& in : inputs) {
        if (!in.cube) throw HsiError("BandStacker: null cube in input '" + in.label + "'.");
        if (!in.cube->sameGridAs(*first)) {
            throw HsiError("BandStacker: '" + in.label + "' (" + std::to_string(in.cube->width) + "x" +
                            std::to_string(in.cube->height) + ") does not match the grid of '" + inputs[0].label +
                            "' (" + std::to_string(first->width) + "x" + std::to_string(first->height) +
                            "). Resample it first.");
        }
        totalBands += in.cube->bands;
    }

    RasterCube out;
    out.allocate(first->width, first->height, totalBands);
    out.geoTransform = first->geoTransform;
    out.projectionWkt = first->projectionWkt;

    int dstBand = 0;
    for (const auto& in : inputs) {
        for (int b = 0; b < in.cube->bands; ++b, ++dstBand) {
            size_t srcBase = static_cast<size_t>(b) * in.cube->width * in.cube->height;
            size_t dstBase = static_cast<size_t>(dstBand) * out.width * out.height;
            std::copy(in.cube->data.begin() + srcBase, in.cube->data.begin() + srcBase + in.cube->pixelCount(),
                      out.data.begin() + dstBase);
            out.bandNames[dstBand] = in.label + ":" +
                (in.cube->bandNames.size() > static_cast<size_t>(b) && !in.cube->bandNames[b].empty()
                     ? in.cube->bandNames[b] : ("band" + std::to_string(b + 1)));
        }
    }

    Logger::log("BandStacker", "Stacked " + std::to_string(inputs.size()) + " input(s) into " +
                std::to_string(totalBands) + " total bands.");
    return out;
}

} // namespace hsi
