#include "tcApp.h"

int main() {
    tc::WindowSettings settings;
    settings.setSize(800, 600);
    settings.setTitle("tcxGeo - example-gpxInfo");

    return TC_RUN_APP(tcApp, settings);
}
