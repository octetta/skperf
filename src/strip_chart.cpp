#include "strip_chart.h"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>

StripChart::StripChart(int x, int y, int w, int h, const char* label) 
    : Fl_Box(x, y, w, h, 0), max_points(500), min_y(0), max_y(100) {  // No label to avoid garbage
    box(FL_DOWN_BOX);
    color(FL_BLACK);
    labelcolor(FL_WHITE);
}

StripChart::~StripChart() {}

void StripChart::addDataPoint(const std::map<std::string, double>& metrics) {
    history.push_back(metrics);
    if (history.size() > static_cast<size_t>(max_points)) {
        history.erase(history.begin());
    }

    for (const auto& m : metrics) {
        if (std::find(metric_keys.begin(), metric_keys.end(), m.first) == metric_keys.end()) {
            metric_keys.push_back(m.first);
        }
        min_y = std::min(min_y, m.second);
        max_y = std::max(max_y, m.second * 1.1);
    }
    redraw();
}

void StripChart::clear() {
    history.clear();
    metric_keys.clear();
    min_y = 0;
    max_y = 100;
    redraw();
}

void StripChart::draw() {
    Fl_Box::draw();
    if (history.empty() || metric_keys.empty()) return;

    int graph_w = w() - 170;
    int graph_h = h() - 50;
    int base_x = x() + 20;
    int base_y = y() + 30;

    // Grid lines
    fl_color(FL_GRAY);
    for (int i = 0; i <= 5; ++i) {
        int yy = base_y + (graph_h * i / 5);
        fl_line(base_x, yy, base_x + graph_w, yy);
    }

    double range = max_y - min_y;
    if (range <= 0) range = 1.0;

    int num_points = static_cast<int>(history.size());
    if (num_points < 2) return;
    int step = graph_w / (num_points - 1);

    // Plot lines
    for (size_t k = 0; k < metric_keys.size(); ++k) {
        fl_color(static_cast<Fl_Color>(FL_RED + (k % 8)));
        std::string key = metric_keys[k];
        int prev_px = -1, prev_py = -1;

        for (int i = 0; i < num_points; ++i) {
            auto it = history[i].find(key);
            if (it == history[i].end()) continue;

            double val = it->second;
            int px = base_x + i * step;
            int py = base_y + static_cast<int>(graph_h - ((val - min_y) / range * graph_h));

            if (prev_px != -1) {
                fl_line(prev_px, prev_py, px, py);
            }
            prev_px = px;
            prev_py = py;
        }
    }

    // Legend aligned to grid lines on the right
    fl_color(FL_WHITE);
    fl_font(FL_HELVETICA, 12);
    int legend_x = x() + w() - 155;
    int legend_y_start = base_y + 4;  // align with first grid

    for (size_t k = 0; k < metric_keys.size() && k < 6; ++k) {  // limit to 6 for space
        int yy = legend_y_start + (graph_h * k / 5);
        fl_draw(metric_keys[k].c_str(), legend_x, yy);
    }
}
