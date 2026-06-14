#pragma once

// =============================================================================
// example-gpxInfo - GPX without a map.
//
// Loads bin/data/palace-loop.gpx and shows the data side of tcxGeo: the track
// shape (drawn from the point list), the waypoints with their elevation, and
// the stats (length via haversine, duration, point count). No network, no
// tiles — tcxGeo is just the data.
// =============================================================================

#include <TrussC.h>
#include <tcxGeo.h>

using namespace std;
using namespace tc;
using namespace tcx;

class tcApp : public App {
public:
    void setup() override;
    void draw() override;

private:
    GpxFile gpx_;

    // Projection from lat/lon to the on-screen plot, recomputed each frame so
    // it follows the window size. Aspect-correct (the loop stays round).
    Rect   plotArea_;
    LatLon center_;
    double lonScale_ = 1;   // cos(latitude): a degree of longitude is shorter
    double scale_    = 1;   // pixels per degree of latitude

    void computeProjection(const GpxTrack& track);
    Vec2 project(LatLon p);

    void drawTrack(const GpxTrack& track);
    void drawWaypoints();
    void drawStats(const GpxTrack& track);
};
