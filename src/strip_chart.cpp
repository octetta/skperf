#include "strip_chart.h"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>

StripChart::StripChart(int x, int y, int w, int h, const char* label) 
    : Fl_Box(x, y, w, h, nullptr) {
    box(FL_DOWN_BOX);
    color(FL_BLACK);
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
    }
    redraw();
}

void StripChart::clear() {
    history.clear();
    metric_keys.clear();
    disabled_metrics.clear();
    redraw();
}

void StripChart::enableMetric(const std::string& key) {
    disabled_metrics.erase(key);
    redraw();
}

void StripChart::disableMetric(const std::string& key) {
    disabled_metrics.insert(key);
    redraw();
}

void StripChart::draw() {
    Fl_Box::draw();
    if (history.empty() || metric_keys.empty()) return;

    int num_metrics = 0;
    for (const auto& k : metric_keys) {
        if (!disabled_metrics.count(k)) num_metrics++;
    }
    if (num_metrics == 0) return;

    int row_h = (h() - 40) / num_metrics;
    int base_y = y() + 20;

    int visible_idx = 0;
    for (const auto& key : metric_keys) {
        if (disabled_metrics.count(key)) continue;

        int row_y = base_y + visible_idx * row_h;
        int row_graph_h = row_h - 30;

        // Find min/max for this metric only
        double row_min = 1e9, row_max = -1e9;
        for (const auto& point : history) {
            auto it = point.find(key);
            if (it != point.end()) {
                row_min = std::min(row_min, it->second);
                row_max = std::max(row_max, it->second);
            }
        }
        if (row_max == row_min) row_max = row_min + 1;

        // Grid for this row
        fl_color(FL_GRAY);
        fl_line(x() + 40, row_y + 10, x() + w() - 200, row_y + 10);
        fl_line(x() + 40, row_y + row_graph_h + 10, x() + w() - 200, row_y + row_graph_h + 10);

        // Line for this metric
        fl_color(static_cast<Fl_Color>(FL_RED + (visible_idx % 8)));
        int prev_px = -1, prev_py = -1;
        int num_points = static_cast<int>(history.size());
        int step = (w() - 280) / std::max(1, num_points - 1);

        for (int i = 0; i < num_points; ++i) {
            auto it = history[i].find(key);
            if (it == history[i].end()) continue;

            double val = it->second;
            int px = x() + 40 + i * step;
            int py = row_y + 10 + static_cast<int>(row_graph_h - ((val - row_min) / (row_max - row_min) * row_graph_h));

            if (prev_px != -1) {
                fl_line(prev_px, prev_py, px, py);
            }
            prev_px = px;
            prev_py = py;
        }

        // Label for this row
        fl_color(FL_WHITE);
        fl_font(FL_HELVETICA, 12);
        fl_draw(key.c_str(), x() + w() - 190, row_y + row_h / 2 + 5);

        visible_idx++;
    }
}
