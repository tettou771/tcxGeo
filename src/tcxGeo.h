#pragma once

// =============================================================================
// tcxGeo - geographic primitives + geo-format I/O
// =============================================================================
// The shared foundation of the tcx geo addons:
//
//   LatLon            — WGS84 coordinate (double degrees) + distance() /
//                       bearing() spherical-earth helpers.
//   GpxFile, loadGpx  — GPX (GPS Exchange Format) tracks / routes / waypoints.
//
// No network, no rendering, no external dependencies (XML parsing uses the
// pugixml bundled with TrussC core). tcxOpenStreetMap builds on this addon
// for its map rendering — use tcxGeo alone when you only need the data side:
// elevation profiles, track statistics, matching photos to a GPS log, ...
//
// Other geo formats (GeoJSON, KML, ...) are intended to live here too, which
// keeps a single LatLon type across them and makes cross-format conversion
// natural.
// =============================================================================

#include "tcxLatLon.h"   // LatLon, distance, bearing
#include "tcxGpx.h"      // GpxFile, GpxTrack, loadGpx, parseGpx
