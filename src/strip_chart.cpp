#include "strip_chart.h"
#include <FL/fl_draw.H>
#include <algorithm>
#include <cmath>
#include <cstdio>

static Fl_Color blendColors(Fl_Color c1, Fl_Color c2, float weight) {
    uchar r1, g1, b1, r2, g2, b2;
    Fl::get_color(c1, r1, g1, b1);
    Fl::get_color(c2, r2, g2, b2);
    uchar r = static_cast<uchar>(r1 * weight + r2 * (1.0f - weight));
    uchar g = static_cast<uchar>(g1 * weight + g2 * (1.0f - weight));
    uchar b = static_cast<uchar>(b1 * weight + b2 * (1.0f - weight));
    return fl_rgb_color(r, g, b);
}

StripChart::StripChart(int x, int y, int w, int h, const char* label) 
    : Fl_Box(x, y, w, h, nullptr) {
    box(FL_BORDER_BOX);
    color(fl_rgb_color(20, 23, 30));
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

bool StripChart::isMetricEnabled(const std::string& key) const {
    return disabled_metrics.count(key) == 0;
}

void StripChart::initDefaultKeys(const std::vector<std::string>& keys) {
    for (const auto& k : keys) {
        if (std::find(metric_keys.begin(), metric_keys.end(), k) == metric_keys.end()) {
            metric_keys.push_back(k);
        }
    }
}

Fl_Color StripChart::getMetricColor(size_t index, bool is_dark) {
    static const Fl_Color dark_palette[] = {
        fl_rgb_color(255, 75, 75),   // 0: Crimson Red
        fl_rgb_color(0, 215, 255),   // 1: Bright Cyan
        fl_rgb_color(70, 160, 255),  // 2: Royal Blue
        fl_rgb_color(40, 220, 130),  // 3: Emerald Green
        fl_rgb_color(240, 220, 40),  // 4: Vivid Yellow
        fl_rgb_color(255, 140, 30),  // 5: Bright Orange
        fl_rgb_color(210, 80, 255),  // 6: Neon Purple
        fl_rgb_color(255, 90, 170)   // 7: Hot Pink
    };
    static const Fl_Color light_palette[] = {
        fl_rgb_color(210, 30, 30),   // 0: Dark Red
        fl_rgb_color(0, 135, 190),   // 1: Deep Cyan
        fl_rgb_color(30, 90, 210),   // 2: Royal Blue
        fl_rgb_color(20, 150, 70),   // 3: Deep Green
        fl_rgb_color(180, 140, 0),   // 4: Amber Yellow
        fl_rgb_color(215, 95, 10),   // 5: Deep Orange
        fl_rgb_color(150, 40, 190),  // 6: Deep Purple
        fl_rgb_color(200, 30, 120)   // 7: Deep Pink
    };
    if (is_dark) {
        return dark_palette[index % 8];
    } else {
        return light_palette[index % 8];
    }
}

Fl_Color StripChart::getMetricColor(const std::string& key) const {
    auto it = std::find(metric_keys.begin(), metric_keys.end(), key);
    if (it != metric_keys.end()) {
        size_t idx = std::distance(metric_keys.begin(), it);
        return getMetricColor(idx, dark_mode);
    }
    return dark_mode ? FL_WHITE : FL_BLACK;
}

void StripChart::setOverlayMode(bool overlay) {
    overlay_mode = overlay;
    redraw();
}

void StripChart::setDarkMode(bool dark) {
    dark_mode = dark;
    color(dark_mode ? fl_rgb_color(20, 23, 30) : fl_rgb_color(255, 255, 255));
    redraw();
}

void StripChart::draw() {
    Fl_Box::draw();
    if (history.empty()) {
        drawEmptyState("No telemetry data received yet. Click 'Connect' to stream metrics.");
        return;
    }

    std::vector<std::string> active_keys;
    for (const auto& k : metric_keys) {
        if (!disabled_metrics.count(k)) {
            active_keys.push_back(k);
        }
    }

    if (active_keys.empty()) {
        drawEmptyState("No metric lines selected. Toggle metric buttons above to display curves.");
        return;
    }

    if (overlay_mode) {
        drawOverlayPlot();
    } else {
        drawStackedPlots();
    }
}

void StripChart::drawEmptyState(const char* message) {
    Fl_Color bg_col     = dark_mode ? fl_rgb_color(20, 23, 30) : fl_rgb_color(255, 255, 255);
    Fl_Color border_col = dark_mode ? fl_rgb_color(45, 52, 66) : fl_rgb_color(205, 215, 228);
    Fl_Color grid_col   = dark_mode ? fl_rgb_color(30, 35, 45) : fl_rgb_color(230, 235, 245);
    Fl_Color text_col   = dark_mode ? fl_rgb_color(140, 155, 175) : fl_rgb_color(90, 105, 125);

    fl_color(bg_col);
    fl_rectf(x(), y(), w(), h());
    fl_color(border_col);
    fl_rect(x(), y(), w(), h());

    fl_line_style(FL_DOT, 1);
    fl_color(grid_col);
    for (int gy = y() + 40; gy < y() + h() - 20; gy += 40) {
        fl_line(x() + 20, gy, x() + w() - 20, gy);
    }
    for (int gx = x() + 40; gx < x() + w() - 20; gx += 60) {
        fl_line(gx, y() + 20, gx, y() + h() - 20);
    }
    fl_line_style(0);

    fl_font(FL_HELVETICA_BOLD, 14);
    fl_color(text_col);
    fl_draw(message, x(), y(), w(), h(), FL_ALIGN_CENTER);
}

void StripChart::drawStackedPlots() {
    std::vector<std::string> active_keys;
    for (const auto& k : metric_keys) {
        if (!disabled_metrics.count(k)) {
            active_keys.push_back(k);
        }
    }

    Fl_Color bg_col     = dark_mode ? fl_rgb_color(20, 23, 30) : fl_rgb_color(255, 255, 255);
    Fl_Color border_col = dark_mode ? fl_rgb_color(45, 52, 66) : fl_rgb_color(205, 215, 228);
    Fl_Color panel_bg   = dark_mode ? fl_rgb_color(14, 17, 22) : fl_rgb_color(246, 248, 252);
    Fl_Color panel_brd  = dark_mode ? fl_rgb_color(38, 44, 56) : fl_rgb_color(215, 222, 234);
    Fl_Color grid_col   = dark_mode ? fl_rgb_color(38, 45, 58) : fl_rgb_color(220, 226, 236);
    Fl_Color label_col  = dark_mode ? fl_rgb_color(130, 142, 160) : fl_rgb_color(90, 105, 125);
    Fl_Color val_col    = dark_mode ? FL_WHITE : fl_rgb_color(20, 25, 35);

    fl_color(bg_col);
    fl_rectf(x(), y(), w(), h());
    fl_color(border_col);
    fl_rect(x(), y(), w(), h());

    int margin_left = 65;
    int margin_right = 140;
    int margin_top = 10;
    int margin_bottom = 10;

    int plot_x = x() + margin_left;
    int plot_w = w() - margin_left - margin_right;
    if (plot_w < 50) plot_w = 50;

    int avail_h = h() - margin_top - margin_bottom;
    int num_plots = static_cast<int>(active_keys.size());
    int spacing = 10;
    int plot_h = (avail_h - (num_plots - 1) * spacing) / num_plots;
    if (plot_h < 25) plot_h = 25;

    for (int idx = 0; idx < num_plots; ++idx) {
        const std::string& key = active_keys[idx];
        int ry = y() + margin_top + idx * (plot_h + spacing);
        Fl_Color col = getMetricColor(key);

        // Sub-chart Plot Area
        fl_color(panel_bg);
        fl_rectf(plot_x, ry, plot_w, plot_h);
        fl_color(panel_brd);
        fl_rect(plot_x, ry, plot_w, plot_h);

        double val_min = 1e9, val_max = -1e9;
        double val_latest = 0.0;
        bool has_data = false;

        for (const auto& point : history) {
            auto it = point.find(key);
            if (it != point.end()) {
                val_min = std::min(val_min, it->second);
                val_max = std::max(val_max, it->second);
                val_latest = it->second;
                has_data = true;
            }
        }
        if (!has_data) { val_min = 0.0; val_max = 1.0; }
        if (std::abs(val_max - val_min) < 1e-6) {
            val_max = val_min + 1.0;
        }

        double range = val_max - val_min;
        double pad_min = val_min - range * 0.05;
        double pad_max = val_max + range * 0.05;
        double pad_range = pad_max - pad_min;

        fl_line_style(FL_DOT, 1);
        fl_color(grid_col);
        int y_top = ry + static_cast<int>(plot_h * 0.15);
        int y_mid = ry + plot_h / 2;
        int y_bot = ry + static_cast<int>(plot_h * 0.85);

        fl_line(plot_x, y_top, plot_x + plot_w, y_top);
        fl_line(plot_x, y_mid, plot_x + plot_w, y_mid);
        fl_line(plot_x, y_bot, plot_x + plot_w, y_bot);
        fl_line_style(0);

        fl_font(FL_HELVETICA, 10);
        fl_color(label_col);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", val_max);
        fl_draw(buf, x() + 5, y_top + 4);
        snprintf(buf, sizeof(buf), "%.2f", (val_min + val_max) / 2.0);
        fl_draw(buf, x() + 5, y_mid + 4);
        snprintf(buf, sizeof(buf), "%.2f", val_min);
        fl_draw(buf, x() + 5, y_bot + 4);

        fl_push_clip(plot_x + 1, ry + 1, plot_w - 2, plot_h - 2);

        int num_pts = static_cast<int>(history.size());
        double step = (max_points > 1) ? static_cast<double>(plot_w) / (max_points - 1) : 0.0;
        int start_x_offset = plot_w - static_cast<int>((num_pts - 1) * step);

        std::vector<int> px_vec, py_vec;
        for (int i = 0; i < num_pts; ++i) {
            auto it = history[i].find(key);
            if (it != history[i].end()) {
                int px = plot_x + start_x_offset + static_cast<int>(i * step);
                double norm = (it->second - pad_min) / pad_range;
                int py = ry + plot_h - static_cast<int>(norm * plot_h);
                px_vec.push_back(px);
                py_vec.push_back(py);
            }
        }

        if (px_vec.size() >= 2) {
            fl_color(blendColors(col, panel_bg, 0.25f));
            fl_begin_polygon();
            fl_vertex(px_vec.front(), ry + plot_h);
            for (size_t i = 0; i < px_vec.size(); ++i) {
                fl_vertex(px_vec[i], py_vec[i]);
            }
            fl_vertex(px_vec.back(), ry + plot_h);
            fl_end_polygon();

            fl_line_style(FL_SOLID, 2);
            fl_color(col);
            for (size_t i = 1; i < px_vec.size(); ++i) {
                fl_line(px_vec[i-1], py_vec[i-1], px_vec[i], py_vec[i]);
            }
            fl_line_style(0);

            int last_x = px_vec.back();
            int last_y = py_vec.back();
            fl_color(val_col);
            fl_pie(last_x - 3, last_y - 3, 7, 7, 0, 360);
            fl_color(col);
            fl_pie(last_x - 2, last_y - 2, 5, 5, 0, 360);
        }

        fl_pop_clip();

        // Right Sidebar Readout Panel (Metric stats)
        int sb_x = plot_x + plot_w + 6;
        int sb_w = margin_right - 10;
        fl_color(panel_bg);
        fl_rectf(sb_x, ry, sb_w, plot_h);
        fl_color(panel_brd);
        fl_rect(sb_x, ry, sb_w, plot_h);

        int rx = sb_x + 8;
        fl_font(FL_HELVETICA_BOLD, 11);
        fl_color(col);
        fl_draw(key.c_str(), rx, ry + 15);

        fl_font(FL_HELVETICA_BOLD, 12);
        fl_color(val_col);
        snprintf(buf, sizeof(buf), "%.2f", val_latest);
        fl_draw(buf, rx, ry + 32);

        fl_font(FL_HELVETICA, 9);
        fl_color(label_col);
        snprintf(buf, sizeof(buf), "min: %.2f", val_min);
        fl_draw(buf, rx, ry + 45);
        snprintf(buf, sizeof(buf), "max: %.2f", val_max);
        fl_draw(buf, rx, ry + 56);
    }
}

void StripChart::drawOverlayPlot() {
    std::vector<std::string> active_keys;
    for (const auto& k : metric_keys) {
        if (!disabled_metrics.count(k)) {
            active_keys.push_back(k);
        }
    }

    Fl_Color bg_col     = dark_mode ? fl_rgb_color(20, 23, 30) : fl_rgb_color(255, 255, 255);
    Fl_Color border_col = dark_mode ? fl_rgb_color(45, 52, 66) : fl_rgb_color(205, 215, 228);
    Fl_Color panel_bg   = dark_mode ? fl_rgb_color(14, 17, 22) : fl_rgb_color(246, 248, 252);
    Fl_Color panel_brd  = dark_mode ? fl_rgb_color(38, 44, 56) : fl_rgb_color(215, 222, 234);
    Fl_Color grid_col   = dark_mode ? fl_rgb_color(38, 45, 58) : fl_rgb_color(220, 226, 236);
    Fl_Color label_col  = dark_mode ? fl_rgb_color(130, 142, 160) : fl_rgb_color(90, 105, 125);
    Fl_Color val_col    = dark_mode ? FL_WHITE : fl_rgb_color(20, 25, 35);

    fl_color(bg_col);
    fl_rectf(x(), y(), w(), h());
    fl_color(border_col);
    fl_rect(x(), y(), w(), h());

    int margin_left = 65;
    int margin_right = 140;
    int margin_top = 12;
    int margin_bottom = 12;

    int plot_x = x() + margin_left;
    int plot_w = w() - margin_left - margin_right;
    int ry = y() + margin_top;
    int plot_h = h() - margin_top - margin_bottom;

    fl_color(panel_bg);
    fl_rectf(plot_x, ry, plot_w, plot_h);
    fl_color(panel_brd);
    fl_rect(plot_x, ry, plot_w, plot_h);

    fl_line_style(FL_DOT, 1);
    fl_color(grid_col);
    for (int i = 0; i <= 4; ++i) {
        int gy = ry + static_cast<int>(plot_h * (i / 4.0));
        fl_line(plot_x, gy, plot_x + plot_w, gy);

        fl_font(FL_HELVETICA, 10);
        fl_color(label_col);
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", 100 - i * 25);
        fl_draw(buf, x() + 10, gy + 4);
    }
    fl_line_style(0);

    fl_push_clip(plot_x + 1, ry + 1, plot_w - 2, plot_h - 2);

    int num_pts = static_cast<int>(history.size());
    double step = (max_points > 1) ? static_cast<double>(plot_w) / (max_points - 1) : 0.0;
    int start_x_offset = plot_w - static_cast<int>((num_pts - 1) * step);

    for (const auto& key : active_keys) {
        Fl_Color col = getMetricColor(key);

        double val_min = 1e9, val_max = -1e9;
        for (const auto& point : history) {
            auto it = point.find(key);
            if (it != point.end()) {
                val_min = std::min(val_min, it->second);
                val_max = std::max(val_max, it->second);
            }
        }
        if (val_max <= val_min) val_max = val_min + 1.0;
        double range = (val_max - val_min) * 1.1;
        val_min -= (val_max - val_min) * 0.05;

        std::vector<int> px_vec, py_vec;
        for (int i = 0; i < num_pts; ++i) {
            auto it = history[i].find(key);
            if (it != history[i].end()) {
                int px = plot_x + start_x_offset + static_cast<int>(i * step);
                double norm = (it->second - val_min) / range;
                int py = ry + plot_h - static_cast<int>(norm * plot_h);
                px_vec.push_back(px);
                py_vec.push_back(py);
            }
        }

        if (px_vec.size() >= 2) {
            fl_line_style(FL_SOLID, 2);
            fl_color(col);
            for (size_t i = 1; i < px_vec.size(); ++i) {
                fl_line(px_vec[i-1], py_vec[i-1], px_vec[i], py_vec[i]);
            }
            fl_line_style(0);

            fl_color(val_col);
            fl_pie(px_vec.back() - 3, py_vec.back() - 3, 7, 7, 0, 360);
            fl_color(col);
            fl_pie(px_vec.back() - 2, py_vec.back() - 2, 5, 5, 0, 360);
        }
    }

    fl_pop_clip();

    // Right Sidebar Legend Panel
    int sb_x = plot_x + plot_w + 6;
    int sb_w = margin_right - 10;
    fl_color(panel_bg);
    fl_rectf(sb_x, ry, sb_w, plot_h);
    fl_color(panel_brd);
    fl_rect(sb_x, ry, sb_w, plot_h);

    int rx = sb_x + 8;
    int ly = ry + 8;
    fl_font(FL_HELVETICA_BOLD, 12);
    fl_color(val_col);
    fl_draw("Legend", rx, ly + 10);
    ly += 22;

    for (const auto& key : active_keys) {
        Fl_Color col = getMetricColor(key);
        double val_latest = 0.0;
        if (!history.empty()) {
            auto it = history.back().find(key);
            if (it != history.back().end()) val_latest = it->second;
        }

        fl_color(col);
        fl_rectf(rx, ly + 2, 8, 8);

        fl_font(FL_HELVETICA, 10);
        fl_color(col);
        fl_draw(key.c_str(), rx + 13, ly + 9);

        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", val_latest);
        fl_font(FL_HELVETICA_BOLD, 10);
        fl_color(val_col);
        fl_draw(buf, rx + 13, ly + 20);

        ly += 26;
        if (ly > ry + plot_h - 20) break;
    }
}
