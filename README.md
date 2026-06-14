# tcxGeo

Geographic primitives and geo-format I/O for TrussC. The shared foundation
of the tcx geo addons — `tcxOpenStreetMap` builds its map rendering on top of
this; use tcxGeo alone when you only need the data side.

```
addons.make
  tcxGeo
```

No network, no rendering, no external dependencies (XML parsing uses the
pugixml bundled with TrussC core).

---

## What's in it

| Thing | Use for |
|---|---|
| `tcx::LatLon` | WGS84 coordinate, `double` degrees. The shared coordinate type across geo addons. |
| `tcx::distance(a, b)` | Great-circle distance in metres (haversine). |
| `tcx::bearing(a, b)` | Initial bearing a → b, radians clockwise from north, `[0, TAU)`. |
| `tcx::GpxFile` / `loadGpx()` / `parseGpx()` | GPX tracks, routes, and waypoints with `<ele>` / `<time>`. |

## LatLon

```cpp
#include <tcxGeo.h>
using namespace tcx;

LatLon tokyoTower {35.6586, 139.7454};
LatLon skytree    {35.7101, 139.8107};

double m  = distance(tokyoTower, skytree);   // ≈ 8200 m
double br = bearing(tokyoTower, skytree);    // radians, clockwise from north
```

`LatLon` is deliberately dumb — no `+`/`*` operators, because degree
arithmetic is almost always a bug (distances aren't Euclidean, midpoints
break across the antimeridian). It's `double` on purpose: `float` resolution
at Tokyo's longitude is ~1.4 m on the ground, which visibly jitters at
street-level zoom. The helpers use the spherical-earth approximation
(±0.5% vs the ellipsoid) — right for creative coding, not for surveying.

## GPX

```cpp
GpxFile gpx = loadGpx(getDataPath("hike.gpx"));   // or parseGpx(xmlString)
if (!gpx.valid) { /* unreadable / not GPX */ }

for (auto& trk : gpx.tracks) {
    logNotice() << trk.name << ": "
                << trk.pointCount() << " pts, "
                << trk.totalLength() / 1000.0 << " km in "
                << trk.duration() / 60.0 << " min";

    for (auto& seg : trk.segments)        // trkseg boundaries are preserved
        for (auto& p : seg)               // p.pos (LatLon), p.ele, p.time
            ...;
}

for (auto& w : gpx.waypoints)             // w.pos, w.name, w.ele, w.time
    ...;
```

Covers what GPS loggers and export tools actually emit: `<trk>/<trkseg>/<trkpt>`,
`<rte>` (exposed as a single-segment track), `<wpt>`, per-point `<ele>` and
`<time>` (ISO 8601, `Z` or explicit offset → UTC `time_t`). Extensions
(heart rate etc.) are ignored. Segment boundaries are preserved — a logger
that loses fix starts a new `<trkseg>`, and bridging that gap would invent a
straight line.

`GpxTrack` helpers: `pointCount()`, `bounds(&sw, &ne)`, `totalLength()`
(metres), `duration()` (seconds).

## Examples

- `example-gpxInfo/` — loads a GPX file and draws the track shape, an
  elevation profile, and the stats. No map, no network: the data side only.
  For drawing a track *on a map*, see tcxOpenStreetMap's `example-gpx`.

## Roadmap

Other geo formats (GeoJSON, KML, ...) are intended to live here too — one
shared `LatLon` across formats is what makes cross-format conversion natural.

## License

MIT (see `LICENSES.md`).
