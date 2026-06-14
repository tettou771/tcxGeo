#include "tcApp.h"

void tcApp::setup() {
    gpx_ = loadGpx(getDataPath("palace-loop.gpx"));
    if (!gpx_.valid || gpx_.tracks.empty()) {
        logError("gpxInfo") << "could not load palace-loop.gpx";
        return;
    }
    const GpxTrack& track = gpx_.tracks.front();
    logNotice("gpxInfo") << track.name << ": " << track.pointCount() << " points, "
                         << track.totalLength() / 1000.0 << " km, "
                         << track.duration() / 60.0 << " min";
}

void tcApp::draw() {
    clear(0.10f, 0.10f, 0.12f);

    if (gpx_.tracks.empty()) {
        setColor(1.0f, 0.4f, 0.4f);
        drawBitmapString("palace-loop.gpx not found", 20, 30);
        return;
    }

    const GpxTrack& track = gpx_.tracks.front();
    computeProjection(track);
    drawTrack(track);
    drawWaypoints();
    drawStats(track);
}

// -----------------------------------------------------------------------------
// Projection
// -----------------------------------------------------------------------------

// Fit the track's bounding box into the plot area, keeping latitude and
// longitude at the same scale so the shape isn't stretched. A degree of
// longitude is cos(latitude) as wide as a degree of latitude, so we correct
// for that.
void tcApp::computeProjection(const GpxTrack& track) {
    float margin = 60;
    plotArea_ = Rect(margin, margin, getWidth() - 2 * margin, getHeight() - 2 * margin);

    LatLon sw, ne;
    track.bounds(&sw, &ne);
    center_ = { (sw.lat + ne.lat) * 0.5, (sw.lon + ne.lon) * 0.5 };
    lonScale_ = cos(center_.lat / 360.0 * TAU);

    double spanLon = (ne.lon - sw.lon) * lonScale_;
    double spanLat = (ne.lat - sw.lat);
    double fitX = plotArea_.width  / max(spanLon, 1e-9);
    double fitY = plotArea_.height / max(spanLat, 1e-9);
    scale_ = 0.9 * min(fitX, fitY);
}

Vec2 tcApp::project(LatLon p) {
    float x = plotArea_.getCenterX() + (p.lon - center_.lon) * lonScale_ * scale_;
    float y = plotArea_.getCenterY() - (p.lat - center_.lat) * scale_;   // +lat = up
    return Vec2(x, y);
}

// -----------------------------------------------------------------------------
// Drawing
// -----------------------------------------------------------------------------

void tcApp::drawTrack(const GpxTrack& track) {
    setColor(0.90f, 0.25f, 0.20f);
    for (const auto& segment : track.segments) {
        for (size_t i = 1; i < segment.size(); i++) {
            drawLine(project(segment[i - 1].pos), project(segment[i].pos));
        }
    }
}

void tcApp::drawWaypoints() {
    for (const auto& wp : gpx_.waypoints) {
        Vec2 px = project(wp.pos);

        setColor(0.20f, 0.45f, 0.95f);
        fill();
        drawCircle(px.x, px.y, 4);

        // Name + elevation, so it's clear each point carries an elevation too.
        string label = wp.name + " (elev. " + to_string((int)wp.ele) + " m)";
        drawBitmapString(label, px.x + 8, px.y + 4);
    }
}

void tcApp::drawStats(const GpxTrack& track) {
    double eleMin = 1e9, eleMax = -1e9;
    for (const auto& segment : track.segments) {
        for (const auto& p : segment) {
            eleMin = min(eleMin, p.ele);
            eleMax = max(eleMax, p.ele);
        }
    }

    char buf[256];
    snprintf(buf, sizeof(buf),
             "%s  |  %zu points  |  %.2f km  |  %.0f min  |  elevation %.0f-%.0f m  |  %zu waypoints",
             track.name.c_str(), track.pointCount(),
             track.totalLength() / 1000.0, track.duration() / 60.0,
             eleMin, eleMax, gpx_.waypoints.size());

    setColor(0.9f, 0.9f, 0.9f);
    drawBitmapString(buf, 20, 30);
}
