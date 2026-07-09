
# Hyperspectral Built-up / LULC Suite

A Qt5 + C++17 desktop application and core processing library for the
Hyperion-hyperspectral built-up extraction and LULC change-detection
workflow: orthorectification check, radiometric/atmospheric correction,
surface-vs-object masking, PCA, a 204-band hyperspectral+Sentinel-2+SAR
stack, SAM+SVM built-up classification, raster-to-vector export, and
supervised/unsupervised LULC with a change matrix.

This was built and verified in this session against GDAL 3.8.4, OpenCV 4.6.0,
Eigen 3.4.0 and Qt 5.15.13 on Ubuntu 24.04 -- the same APIs are present on
Ubuntu 20.04 (GDAL 3.0.x, OpenCV 4.2.x, Qt 5.12.x) with only minor version
differences; see "Building on Ubuntu 20.04" below.

## Architecture

```
hyperspectral_suite/
  hyperspectral_suite.pro   the ONLY .pro file -- builds everything into one executable: hsi_desktop
  core/                   processing code -- no Qt dependency, GDAL/OpenCV/Eigen only
    include/hsi/          one header per pipeline stage
    src/                  matching implementations
  ui/                     Qt5 Widgets application layer
    MainWindow.*           menu shell, shared AppState, log console, raster preview
    RasterPreviewWidget.*  percentile-stretched grayscale/RGB raster viewer
    dialogs/                one dialog per pipeline stage, each calling the core code directly
  tools/cli_test/         hsi_cli_smoketest -- OPTIONAL standalone headless test (own main(),
                          not part of hyperspectral_suite.pro -- see "Optional smoke test" below)
```

`core/` and `ui/` are still separate folders for readability (algorithms vs.
UI), but `hyperspectral_suite.pro` compiles every `.cpp` from both straight
into one `hsi_desktop` binary -- no static library, no subdirs, no
cross-project linking to go wrong. Just:

```bash
qmake hyperspectral_suite.pro && make
```

### Pipeline stages (`core/include/hsi/`)

| Stage | Header | Notes |
|---|---|---|
| Raster I/O | `RasterIO.h` | GDAL-backed load/save, georeferencing inspection |
| Orthorectification | `Orthorectifier.h` | Skips automatically if the scene already has a valid geotransform/SRS and no GCPs; otherwise warps using embedded GCPs/RPCs |
| Band selection | `BandSelector.h` | Keeps Hyperion VNIR 8-57 + SWIR 77-224 (198 bands), drops the 58-76 overlap zone and the 225-242 uncalibrated tail |
| Radiometric calibration | `RadiometricCalibrator.h` | DN -> radiance, /40 (VNIR) or /80 (SWIR) |
| Atmospheric correction | `AtmosphericCorrector.h` | Radiance -> TOA reflectance (`pi*L*d^2/(ESUN*cos(theta_s))`, Earth-Sun distance from day-of-year) -> surface reflectance via simplified Dark Object Subtraction (a stand-in for full QUAC/ELM -- see Limitations) |
| Surface/object mask | `SurfaceObjectMask.h` | Three selectable methods: fixed spectral threshold, unsupervised k-means, trained SVM; includes validation against an independent reference mask (IoU/precision/recall) |
| PCA | `PcaReducer.h` | Eigen-based, over a configurable band range |
| Band stacking | `BandStacker.h` | Concatenates co-registered cubes (built for hyperspectral 0-197 + Sentinel-2 198-201 + SAR VV/VH 202-203 = 204 bands) |
| Spectral library + SAM | `SpectralLibrary.h`, `SamClassifier.h` | Mean per-class signatures from labeled samples; Spectral Angle Mapper classification |
| SVM | `SvmModel.h` | Generic reusable `cv::ml::SVM` wrapper (mask, built-up, LULC) |
| Built-up fusion | `BuiltUpClassifier.h` | Runs SAM and SVM, fuses to one 0/1 band (AND / OR / majority) |
| Raster -> vector | `RasterToVector.h` | `GDALPolygonize` wrapper (GeoJSON/Shapefile/GPKG) |
| LULC | `LulcClassifier.h` | Unsupervised (k-means) and supervised (SVM) classification |
| Change matrix | `ChangeDetector.h` | From-class x to-class pixel/area cross-tabulation, CSV export |
| Orchestration | `Pipeline.h` | Wires ortho -> band-select -> calibrate -> atmospheric correction into one call |

## Building on Ubuntu 20.04

```bash
sudo apt-get update
sudo apt-get install -y build-essential pkg-config \
    qt5-qmake qtbase5-dev qttools5-dev-tools \
    libgdal-dev gdal-bin \
    libopencv-dev \
    libeigen3-dev

mkdir build && cd build
qmake ../hyperspectral_suite.pro
make -j$(nproc)
```

This produces a single binary: `build/hsi_desktop`.

`hyperspectral_suite.pro` pulls in GDAL/OpenCV/Eigen via
`CONFIG += link_pkgconfig` / `PKGCONFIG += gdal opencv4 eigen3` -- all three
ship a `.pc` file on 20.04/22.04/24.04, so no manual include/lib paths are
needed. If your GDAL build doesn't install `gdal.pc` (rare, but happens on
some older repackagings), replace the `PKGCONFIG` line with:
```
INCLUDEPATH += $$system(gdal-config --cflags | sed 's/-I//')
LIBS += $$system(gdal-config --libs)
PKGCONFIG += opencv4 eigen3
```

In Qt Creator: **File -> Open File or Project -> `hyperspectral_suite.pro`**.
One project, one target (`hsi_desktop`), the usual build/run/debug buttons.

> Note: `CMakeLists.txt` files are also still included in every directory
> (`cmake .. && make`) if you ever want that path instead -- both build
> systems compile the exact same source files and were both verified in
> this session. `hyperspectral_suite.pro` is the primary/recommended path.

On 20.04, GDAL ships as 3.0.x and OpenCV as 4.2.x -- both expose the same
APIs used here (`GDALWarpOperation`, `GDALPolygonize`, `cv::ml::SVM`,
`cv::kmeans`), so no source changes should be needed. If your 20.04 box's
GDAL is older still (2.x, from the default Ubuntu repo before any PPA), add
the `ubuntugis-unstable` PPA first for GDAL 3.x.

## Optional: standalone smoke test

`tools/cli_test/main.cpp` is a headless test that synthesizes a small
Hyperion-like cube plus matching Sentinel-2/SAR rasters and drives every
pipeline stage end to end. It has its own `main()`, so it's deliberately
**not** part of `hyperspectral_suite.pro` (a second `main()` would conflict
with the desktop app's). To build it on its own:

```bash
cd tools/cli_test
g++ -std=c++17 -O2 main.cpp -o hsi_cli_smoketest \
    -I../../core/include \
    $(pkg-config --cflags --libs gdal opencv4 eigen3)
./hsi_cli_smoketest
```

Expect ~10 stage banners ending in `ALL STAGES COMPLETED SUCCESSFULLY`, with
outputs (built-up mask GeoTIFF, built-up GeoJSON, spectral library CSV,
change matrix CSV) written to `smoketest_output/`. This is the fastest way
to confirm the core algorithms are healthy on a new machine before touching
real data or the UI.

## Running the desktop app

```bash
./build/hsi_desktop
```

The **Hyperspectral** menu lists the stages in order:

1. **Load + Check Orthorectification** -- picks a scene, reports whether it's
   already orthorectified or needs warping (and warps it if so).
2. **Preprocessing** -- band selection, DN->radiance, TOA->surface reflectance.
3. **Surface vs Object Mask** -- pick threshold / k-means / trained-SVM, with
   optional validation against an EOS-04- or optical-derived reference mask.
4. **PCA & 204-band Stack** -- PCA over a chosen band range, then stack the
   reflectance cube with Sentinel-2 and SAR rasters.
5. **Built-up Classification** -- spectral library + SAM, SVM, fused to one band.
6. **LULC Classification** -- unsupervised k-means and supervised SVM, plus
   applying the trained model to a second date for change detection.
7. **Change Detection Matrix** -- cross-tabulates the two LULC dates, exports CSV.

**Run Full Pipeline** walks through dialogs 2-7 in sequence, pre-filled with
whatever has already been computed in the session.

### Sample CSV format

The mask/built-up/LULC dialogs that need labeled training pixels all use the
same simple format, one sample per line:

```
class_name,row,col
built_up,5,5
built_up,7,9
vegetation,40,42
bare_soil,32,4
```

`row`/`col` are 0-based pixel coordinates into whichever cube the dialog is
operating on (surface reflectance for the mask dialog, the 204-band stack
for built-up/LULC).

## Known limitations / next steps

- **ESUN table**: atmospheric correction needs a real per-band Thuillier-derived
  solar irradiance table (`EsunTable::loadCsv`); without one it falls back to
  a flat placeholder and logs a loud warning. Build this once for your sensor
  and reuse it.
- **Surface reflectance**: the DOS (dark-object subtraction) step is a
  simplified stand-in for full QUAC/ELM atmospheric modeling, as agreed --
  swap in a real implementation (or shell out to an existing one) when ready.
- **Large scenes**: `RasterCube` holds the entire cube in memory as float32.
  A full Hyperion strip (100km x 7.5km at 30m, 198 bands) is a few hundred MB
  and fine; very large mosaics would need a windowed/tiled processing mode.
- **ISODATA**: the unsupervised LULC path uses k-means rather than full
  ISODATA (no automatic cluster split/merge) -- swap in a real ISODATA if
  that matters for your accuracy requirements.
- **Resampling to a common grid**: `BandStacker` requires all inputs to
  already share the same width/height/grid; reprojecting Sentinel-2/SAR onto
  the Hyperion grid before stacking is on you for now (a `gdalwarp`-backed
  helper would be a natural next addition, reusing `Orthorectifier`'s warp code).
=======
# hyperspectral
WESEE_POC

