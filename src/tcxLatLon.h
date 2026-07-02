#pragma once

// =============================================================================
// tcxLatLon.h - geographic coordinate + spherical-earth helpers
// =============================================================================
// LatLon is the shared coordinate type of the tcx geo addons (tcxGeo,
// tcxOpenStreetMap, ...). Kept deliberately dumb — no arithmetic operators,
// because degree arithmetic is almost always a bug (distances aren't
// Euclidean, midpoints break across the antimeridian). Use the named helpers
// instead, or a projection (e.g. Web Mercator in tcxOpenStreetMap) when you
// need pixel math.
//
// double, not float, on purpose: float resolution at Tokyo's longitude is
// ~1.4 m on the ground, which visibly jitters at street-level zoom.
//
// The helpers use the spherical-earth approximation (±0.5% vs the ellipsoid)
// — right for creative coding, not for surveying.
// =============================================================================

#include <algorithm>
#include <cmath>

namespace tcx::geo {

// WGS84 degrees. lat in [-90, 90] (+north), lon in [-180, 180] (+east).
struct LatLon {
    double lat = 0;
    double lon = 0;

    bool operator==(const LatLon& o) const { return lat == o.lat && lon == o.lon; }
    bool operator!=(const LatLon& o) const { return !(*this == o); }
};

inline constexpr double EARTH_RADIUS_M = 6371008.8;   // IUGG mean radius

// Great-circle distance in metres (haversine).
inline double distance(LatLon a, LatLon b) {
    double lat1 = a.lat * M_PI / 180.0;
    double lat2 = b.lat * M_PI / 180.0;
    double dLat = (b.lat - a.lat) * M_PI / 180.0;
    double dLon = (b.lon - a.lon) * M_PI / 180.0;
    double s = std::sin(dLat * 0.5);
    double t = std::sin(dLon * 0.5);
    double h = s * s + std::cos(lat1) * std::cos(lat2) * t * t;
    return 2.0 * EARTH_RADIUS_M * std::asin(std::min(1.0, std::sqrt(h)));
}

// Initial bearing from a towards b, radians clockwise from true north,
// in [0, TAU). (The bearing drifts along a great circle — this is the
// value at the start point.)
inline double bearing(LatLon a, LatLon b) {
    double lat1 = a.lat * M_PI / 180.0;
    double lat2 = b.lat * M_PI / 180.0;
    double dLon = (b.lon - a.lon) * M_PI / 180.0;
    double y = std::sin(dLon) * std::cos(lat2);
    double x = std::cos(lat1) * std::sin(lat2)
             - std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
    double br = std::atan2(y, x);
    if (br < 0) br += 2.0 * M_PI;
    return br;
}

} // namespace tcx::geo

// -----------------------------------------------------------------------------
// Backward compatibility. The canonical namespace is now `tcx::geo`. These
// silent aliases keep older code compiling: flat `tcx::LatLon` and legacy
// `trussc::LatLon`. DEPRECATED — removed in v1.0.0.
// (No [[deprecated]] attribute: under the usual `using namespace tc;` it would
//  warn on idiomatic unqualified use too. See tcxGeo README for migration.)
// -----------------------------------------------------------------------------
namespace tcx    { using geo::LatLon; }          // deprecated: remove at v1.0.0
namespace tcx    { using geo::EARTH_RADIUS_M; }  // deprecated: remove at v1.0.0
namespace tcx    { using geo::distance; }         // deprecated: remove at v1.0.0
namespace tcx    { using geo::bearing; }          // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::LatLon; }         // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::EARTH_RADIUS_M; } // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::distance; }       // deprecated: remove at v1.0.0
namespace trussc { using tcx::geo::bearing; }        // deprecated: remove at v1.0.0
