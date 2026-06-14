#include "tcApp.h"

void tcApp::setup() {
    gpx_ = loadGpx(getDataPath("palace-loop.gpx"));
    if (!gpx_.valid || gpx_.tracks.empty()) {
        logError("gpxInfo") << "could not load palace-loop.gpx";
        return;
    }
    auto& trk = gpx_.tracks.front();
    logNotice("gpxInfo") << trk.name << ": " << trk.pointCount() << " points, "
                         << trk.totalLength() / 1000.0 << " km, "
                         << trk.duration() / 60.0 << " min";
}

void tcApp::draw() {
    clear(0.10f, 0.10f, 0.12f);
    if (gpx_.tracks.empty()) {
        setColor(1.0f, 0.4f, 0.4f);
        drawBitmapString("palace-loop.gpx not found", 20, 30);
        return;
    }

    auto& trk = gpx_.tracks.front();
    float w = getWidth(), h = getHeight();

    drawTrackShape(trk, {20, 50, w - 40, h - 260});
    drawElevationProfile(trk, {20, h - 180, w - 40, 140});

    // Stats line.
    char buf[256];
    snprintf(buf, sizeof(buf), "%s  |  %zu points  |  %.2f km  |  %.0f min  |  %zu waypoints",
             trk.name.c_str(), trk.pointCount(),
             trk.totalLength() / 1000.0, trk.duration() / 60.0,
             gpx_.waypoints.size());
    setColor(0.9f, 0.9f, 0.9f);
    drawBitmapString(buf, 20, 30);
}

// Locally-projected track shape: equirectangular with cos(midLat) correction
// is plenty for a track-sized area — this demo's whole point is that you
// don't need a map (or Mercator) to make use of a GPX file.
void tcApp::drawTrackShape(const GpxTrack& trk, Rect area) {
    LatLon sw, ne;
    if (!trk.bounds(&sw, &ne)) return;

    double midLat   = (sw.lat + ne.lat) * 0.5;
    double lonScale = std::cos(midLat * M_PI / 180.0);
    double spanX = (ne.lon - sw.lon) * lonScale;
    double spanY = (ne.lat - sw.lat);
    double scale = 0.9 * std::min(area.width  / std::max(spanX, 1e-9),
                                  area.height / std::max(spanY, 1e-9));

    auto project = [&](LatLon p) -> Vec2 {
        double x = (p.lon - (sw.lon + ne.lon) * 0.5) * lonScale * scale;
        double y = ((sw.lat + ne.lat) * 0.5 - p.lat) * scale;   // +lat = up
        return {(float)(area.x + area.width * 0.5 + x),
                (float)(area.y + area.height * 0.5 + y)};
    };

    setColor(0.90f, 0.25f, 0.20f);
    for (auto& seg : trk.segments) {
        for (size_t i = 1; i < seg.size(); i++) {
            drawLine(project(seg[i - 1].pos), project(seg[i].pos));
        }
    }

    setColor(0.20f, 0.45f, 0.95f);
    for (auto& wp : gpx_.waypoints) {
        Vec2 px = project(wp.pos);
        fill();
        drawCircle(px.x, px.y, 4);
        drawBitmapString(wp.name, px.x + 8, px.y + 4);
    }
}

void tcApp::drawElevationProfile(const GpxTrack& trk, Rect area) {
    // Elevation vs cumulative distance, over all segments.
    std::vector<Vec2> pts;     // x = metres from start, y = ele
    double dist = 0;
    double eleMin = 1e9, eleMax = -1e9;
    for (auto& seg : trk.segments) {
        for (size_t i = 0; i < seg.size(); i++) {
            if (i > 0) dist += distance(seg[i - 1].pos, seg[i].pos);
            pts.push_back({(float)dist, (float)seg[i].ele});
            eleMin = std::min(eleMin, seg[i].ele);
            eleMax = std::max(eleMax, seg[i].ele);
        }
    }
    if (pts.size() < 2 || dist <= 0) return;
    double eleSpan = std::max(eleMax - eleMin, 1.0);

    setColor(0.25f, 0.25f, 0.30f);
    noFill();
    drawRect(area.x, area.y, area.width, area.height);

    auto project = [&](Vec2 p) -> Vec2 {
        return {area.x + p.x / (float)dist * area.width,
                area.y + area.height * (1.0f - (float)((p.y - eleMin) / eleSpan))};
    };

    setColor(0.35f, 0.80f, 0.45f);
    for (size_t i = 1; i < pts.size(); i++) {
        drawLine(project(pts[i - 1]), project(pts[i]));
    }

    char buf[128];
    snprintf(buf, sizeof(buf), "elevation %.0f - %.0f m over %.2f km",
             eleMin, eleMax, dist / 1000.0);
    setColor(0.7f, 0.7f, 0.7f);
    drawBitmapString(buf, area.x + 8, area.y + 16);
}
