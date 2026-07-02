#include "tcxGpx.h"

#include "pugixml/pugixml.hpp"

#include <algorithm>
#include <cstdio>

namespace tcx::geo {

// ---------------------------------------------------------------- time

// "2024-12-15T10:30:45Z" / "2024-12-15T10:30:45.123+09:00" → UTC time_t.
// GPX times are UTC ("Z") or carry an explicit offset; honour the offset.
static std::time_t parseIso8601(const char* s) {
    if (!s || !*s) return 0;

    struct tm t{};
    if (std::sscanf(s, "%d-%d-%dT%d:%d:%d",
                    &t.tm_year, &t.tm_mon, &t.tm_mday,
                    &t.tm_hour, &t.tm_min, &t.tm_sec) < 6) return 0;
    t.tm_year -= 1900;
    t.tm_mon  -= 1;

#if defined(_WIN32)
    std::time_t utc = _mkgmtime(&t);
#else
    std::time_t utc = timegm(&t);
#endif
    if (utc == (std::time_t)-1) return 0;

    // Optional offset after the seconds (and optional fraction): +09:00 / -05:00.
    // Search past the date part so its '-' separators don't match.
    for (const char* p = s + 11; *p; p++) {
        if (*p == '+' || *p == '-') {
            int h = 0, m = 0;
            if (std::sscanf(p + 1, "%d:%d", &h, &m) >= 1) {
                int off = h * 3600 + m * 60;
                utc += (*p == '+') ? -off : off;
            }
            break;
        }
    }
    return utc;
}

// ---------------------------------------------------------------- parse

static GpxPoint parsePoint(const pugi::xml_node& pt) {
    GpxPoint p;
    p.pos.lat = pt.attribute("lat").as_double();
    p.pos.lon = pt.attribute("lon").as_double();
    p.ele     = pt.child("ele").text().as_double(0.0);
    p.time    = parseIso8601(pt.child_value("time"));
    return p;
}

static GpxFile parseDoc(const pugi::xml_document& doc) {
    GpxFile out;
    auto gpx = doc.child("gpx");
    if (!gpx) return out;
    out.valid = true;

    for (auto trk : gpx.children("trk")) {
        GpxTrack t;
        t.name = trk.child_value("name");
        for (auto seg : trk.children("trkseg")) {
            std::vector<GpxPoint> pts;
            for (auto pt : seg.children("trkpt")) pts.push_back(parsePoint(pt));
            if (!pts.empty()) t.segments.push_back(std::move(pts));
        }
        if (!t.segments.empty()) out.tracks.push_back(std::move(t));
    }

    // <rte> is a planned route rather than a recorded log. Expose it as a
    // single-segment track so callers draw both through the same path.
    for (auto rte : gpx.children("rte")) {
        GpxTrack t;
        t.name = rte.child_value("name");
        std::vector<GpxPoint> pts;
        for (auto pt : rte.children("rtept")) pts.push_back(parsePoint(pt));
        if (!pts.empty()) {
            t.segments.push_back(std::move(pts));
            out.tracks.push_back(std::move(t));
        }
    }

    for (auto wpt : gpx.children("wpt")) {
        GpxWaypoint w;
        w.pos.lat = wpt.attribute("lat").as_double();
        w.pos.lon = wpt.attribute("lon").as_double();
        w.ele     = wpt.child("ele").text().as_double(0.0);
        w.time    = parseIso8601(wpt.child_value("time"));
        w.name    = wpt.child_value("name");
        out.waypoints.push_back(std::move(w));
    }

    return out;
}

GpxFile loadGpx(const std::string& path) {
    pugi::xml_document doc;
    if (!doc.load_file(path.c_str())) return {};
    return parseDoc(doc);
}

GpxFile parseGpx(const std::string& xml) {
    pugi::xml_document doc;
    if (!doc.load_buffer(xml.data(), xml.size())) return {};
    return parseDoc(doc);
}

// ---------------------------------------------------------------- bounds

bool GpxTrack::bounds(LatLon* sw, LatLon* ne) const {
    bool any = false;
    LatLon lo{ 90.0,  180.0};
    LatLon hi{-90.0, -180.0};
    for (auto& seg : segments) {
        for (auto& p : seg) {
            lo.lat = std::min(lo.lat, p.pos.lat);
            lo.lon = std::min(lo.lon, p.pos.lon);
            hi.lat = std::max(hi.lat, p.pos.lat);
            hi.lon = std::max(hi.lon, p.pos.lon);
            any = true;
        }
    }
    if (!any) return false;
    if (sw) *sw = lo;
    if (ne) *ne = hi;
    return true;
}

// ---------------------------------------------------------------- stats

double GpxTrack::totalLength() const {
    double total = 0;
    for (auto& seg : segments) {
        for (size_t i = 1; i < seg.size(); i++) {
            total += distance(seg[i - 1].pos, seg[i].pos);
        }
    }
    return total;
}

double GpxTrack::duration() const {
    std::time_t first = 0, last = 0;
    for (auto& seg : segments) {
        for (auto& p : seg) {
            if (p.time == 0) continue;
            if (first == 0 || p.time < first) first = p.time;
            if (p.time > last) last = p.time;
        }
    }
    return (first == 0) ? 0.0 : (double)(last - first);
}

} // namespace tcx::geo
