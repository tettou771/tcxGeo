#pragma once

// =============================================================================
// example-gpxInfo - GPX without a map.
//
// Loads bin/data/palace-loop.gpx and shows the data side of tcxGeo: the
// track shape (locally-projected polyline), an elevation profile, and the
// stats (length via haversine, duration, point count). No network, no tiles.
// =============================================================================

#include <TrussC.h>
#include <tcxGeo.h>

using namespace tc;
using namespace tcx;

class tcApp : public App {
public:
    void setup() override;
    void draw() override;

private:
    GpxFile gpx_;

    void drawTrackShape(const GpxTrack& trk, Rect area);
    void drawElevationProfile(const GpxTrack& trk, Rect area);
};
