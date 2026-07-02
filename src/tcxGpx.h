#pragma once

// =============================================================================
// tcxGpx.h - GPX (GPS Exchange Format) track / waypoint parsing
// =============================================================================
// Pure data + parser — no rendering, no map dependency. Pair with
// tcxOpenStreetMap's TrackLayer to draw a parsed track over a map, or consume
// the points yourself (elevation profiles, photo-by-time matching, ...).
//
//   GpxFile gpx = loadGpx(getDataPath("hike.gpx"));
//   for (auto& trk : gpx.tracks)
//       logNotice() << trk.name << ": " << trk.pointCount() << " points";
//
// Covers the GPX 1.0/1.1 elements that GPS loggers and export tools actually
// emit: <trk>/<trkseg>/<trkpt>, <rte>/<rtept>, <wpt>, with per-point <ele>
// and <time>. Extensions (heart rate etc.) are ignored.
// =============================================================================

#include "tcxLatLon.h"

#include <ctime>
#include <string>
#include <vector>

namespace tcx::geo {

struct GpxPoint {
    LatLon      pos{};
    double      ele  = 0;    // metres above sea level; 0 when the file has none
    std::time_t time = 0;    // UTC; 0 when the file has none
};

struct GpxWaypoint {
    LatLon      pos{};
    double      ele  = 0;
    std::time_t time = 0;
    std::string name;
};

struct GpxTrack {
    std::string name;
    // trkseg boundaries are preserved: a logger that loses GPS fix starts a
    // new segment, and drawing across that gap would invent a straight line.
    std::vector<std::vector<GpxPoint>> segments;

    size_t pointCount() const {
        size_t n = 0;
        for (auto& s : segments) n += s.size();
        return n;
    }

    // Bounding box over every point. False when the track is empty.
    bool bounds(LatLon* sw, LatLon* ne) const;

    // Sum of great-circle distances between consecutive points, in metres.
    // Segment gaps (GPS dropouts) are not bridged.
    double totalLength() const;

    // Last timestamp minus first, in seconds. 0 when the file has no <time>.
    double duration() const;
};

struct GpxFile {
    std::vector<GpxTrack>    tracks;     // <trk>, plus <rte> as 1-segment tracks
    std::vector<GpxWaypoint> waypoints;  // <wpt>
    bool valid = false;                  // false = unreadable / not a GPX document

    bool empty() const { return tracks.empty() && waypoints.empty(); }
};

// Parse a .gpx file on disk / a GPX document already in memory.
GpxFile loadGpx (const std::string& path);
GpxFile parseGpx(const std::string& xml);

} // namespace tcx::geo

// -----------------------------------------------------------------------------
// Backward compatibility. The canonical namespace is now `tcx::geo`. These
// silent aliases keep older code compiling: flat `tcx::GpxFile` and legacy
// `trussc::GpxFile`. DEPRECATED — removed in v1.0.0.
// (No [[deprecated]] attribute: under the usual `using namespace tc;` it would
//  warn on idiomatic unqualified use too. See tcxGeo README for migration.)
// -----------------------------------------------------------------------------
namespace tcx    { using geo::GpxPoint; }    // deprecated: remove at v1.0.0
namespace tcx    { using geo::GpxWaypoint; } // deprecated: remove at v1.0.0
namespace tcx    { using geo::GpxTrack; }    // deprecated: remove at v1.0.0
namespace tcx    { using geo::GpxFile; }     // deprecated: remove at v1.0.0
namespace tcx    { using geo::loadGpx; }     // deprecated: remove at v1.0.0
namespace tcx    { using geo::parseGpx; }    // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::GpxPoint; }    // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::GpxWaypoint; } // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::GpxTrack; }    // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::GpxFile; }     // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::loadGpx; }     // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::parseGpx; }    // deprecated: remove at v1.0.0
