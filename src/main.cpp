#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "udp_client.h"
#include "strip_chart.h"

class AudioMonitor : public Fl_Double_Window {
public:
    AudioMonitor() : Fl_Double_Window(1220, 780, "Audio Performance Monitor") {
        begin();

        // --- Top Control Panel Bar ---
        top_panel = new Fl_Group(15, 10, 1190, 42);
        top_panel->box(FL_FLAT_BOX);

        ip_input = new Fl_Input(100, 18, 120, 26, "IP Address:");
        ip_input->value("127.0.0.1");
        ip_input->labelfont(FL_HELVETICA_BOLD);

        port_input = new Fl_Int_Input(295, 18, 60, 26, "UDP Port:");
        port_input->value("60440");
        port_input->labelfont(FL_HELVETICA_BOLD);

        refresh_input = new Fl_Int_Input(445, 18, 45, 26, "Refresh (s):");
        refresh_input->value("2");
        refresh_input->labelfont(FL_HELVETICA_BOLD);

        connect_btn = new Fl_Button(505, 18, 95, 26, "Connect");
        connect_btn->box(FL_BORDER_BOX);
        connect_btn->labelfont(FL_HELVETICA_BOLD);
        connect_btn->callback(connect_cb, this);

        status_box = new Fl_Box(610, 18, 115, 26, "Disconnected");
        status_box->box(FL_BORDER_BOX);
        status_box->labelfont(FL_HELVETICA_BOLD);
        status_box->align(FL_ALIGN_CENTER);

        sep_box = new Fl_Box(735, 15, 2, 32);
        sep_box->box(FL_FLAT_BOX);

        // Telemetry outputs
        refresh_count_out = new Fl_Output(845, 18, 60, 26, "Refresh count:");
        refresh_count_out->value("0");
        refresh_count_out->labelfont(FL_HELVETICA_BOLD);

        last_response_out = new Fl_Output(1015, 18, 80, 26, "Last response:");
        last_response_out->value("never");
        last_response_out->labelfont(FL_HELVETICA_BOLD);

        top_panel->end();

        // --- Metric Lines Toolbar (Row 2, above strip chart lines) ---
        toolbar_panel = new Fl_Group(15, 58, 1190, 40);
        toolbar_panel->box(FL_FLAT_BOX);

        toolbar_label = new Fl_Box(25, 65, 90, 26, "Metric Lines:");
        toolbar_label->labelfont(FL_HELVETICA_BOLD);

        metric_btn_group = new Fl_Group(120, 65, 730, 26);
        metric_btn_group->end();

        mode_btn = new Fl_Button(860, 65, 110, 26, "Mode: Stacked");
        mode_btn->box(FL_BORDER_BOX);
        mode_btn->labelfont(FL_HELVETICA_BOLD);
        mode_btn->callback(mode_cb, this);

        theme_btn = new Fl_Button(980, 65, 100, 26, "Theme: Dark");
        theme_btn->box(FL_BORDER_BOX);
        theme_btn->labelfont(FL_HELVETICA_BOLD);
        theme_btn->callback(theme_cb, this);

        clear_btn = new Fl_Button(1090, 65, 60, 26, "Clear");
        clear_btn->box(FL_BORDER_BOX);
        clear_btn->labelfont(FL_HELVETICA_BOLD);
        clear_btn->callback(clear_cb, this);

        toolbar_panel->end();

        // --- Strip Chart ---
        chart = new StripChart(15, 105, 1190, 660);

        end();

        // Initialize default metric keys
        std::vector<std::string> default_metrics = {
            "overruns", "cb_ms_last", "cb_ms_avg", "load_last",
            "load_avg", "late_starts", "discont", "clipped"
        };
        chart->initDefaultKeys(default_metrics);

        applyTheme();
        updateMetricButtons();

        resizable(chart);
        size_range(1050, 600);
    }

    void applyTheme() {
        bool dark = is_dark_mode;
        chart->setDarkMode(dark);

        Fl_Color win_bg      = dark ? fl_rgb_color(24, 27, 34)   : fl_rgb_color(242, 244, 248);
        Fl_Color panel_bg    = dark ? fl_rgb_color(32, 36, 45)   : fl_rgb_color(228, 233, 242);
        Fl_Color toolbar_bg  = dark ? fl_rgb_color(28, 31, 38)   : fl_rgb_color(220, 225, 234);
        Fl_Color input_bg    = dark ? fl_rgb_color(18, 21, 26)   : fl_rgb_color(255, 255, 255);
        Fl_Color input_txt   = dark ? FL_WHITE                    : fl_rgb_color(20, 25, 35);
        Fl_Color cursor_col  = dark ? fl_rgb_color(0, 220, 255)   : fl_rgb_color(0, 100, 220);
        Fl_Color label_txt   = dark ? fl_rgb_color(200, 210, 225) : fl_rgb_color(50, 60, 75);
        Fl_Color out_bg      = dark ? fl_rgb_color(14, 16, 20)   : fl_rgb_color(238, 242, 248);
        Fl_Color out_txt     = dark ? fl_rgb_color(100, 220, 255) : fl_rgb_color(0, 105, 210);
        Fl_Color btn_bg      = dark ? fl_rgb_color(45, 52, 66)   : fl_rgb_color(205, 214, 228);
        Fl_Color btn_txt     = dark ? FL_WHITE                    : fl_rgb_color(30, 35, 45);
        Fl_Color sep_color   = dark ? fl_rgb_color(55, 62, 75)   : fl_rgb_color(190, 200, 215);

        color(win_bg);
        top_panel->color(panel_bg);
        toolbar_panel->color(toolbar_bg);
        sep_box->color(sep_color);
        toolbar_label->labelcolor(label_txt);

        // Style Inputs and explicitly set bright visible cursor_color
        auto style_input = [&](Fl_Input* in) {
            in->color(input_bg);
            in->textcolor(input_txt);
            in->labelcolor(label_txt);
            in->cursor_color(cursor_col);
            in->box(FL_BORDER_BOX);
        };
        style_input(ip_input);
        style_input(port_input);
        style_input(refresh_input);

        // Style Outputs
        auto style_output = [&](Fl_Output* out) {
            out->color(out_bg);
            out->textcolor(out_txt);
            out->labelcolor(label_txt);
            out->box(FL_BORDER_BOX);
        };
        style_output(refresh_count_out);
        style_output(last_response_out);

        // Buttons (Flat)
        mode_btn->color(btn_bg);
        mode_btn->labelcolor(btn_txt);

        theme_btn->color(btn_bg);
        theme_btn->labelcolor(btn_txt);
        theme_btn->label(dark ? "Theme: Dark" : "Theme: Light");

        clear_btn->color(btn_bg);
        clear_btn->labelcolor(btn_txt);

        updateConnectColors();
        updateMetricButtons();
        redraw();
    }

    void updateConnectColors() {
        bool dark = is_dark_mode;
        if (udp.isConnected()) {
            status_box->color(dark ? fl_rgb_color(30, 90, 50) : fl_rgb_color(210, 245, 220));
            status_box->labelcolor(dark ? fl_rgb_color(120, 240, 150) : fl_rgb_color(15, 110, 45));
            connect_btn->color(dark ? fl_rgb_color(180, 45, 45) : fl_rgb_color(225, 60, 60));
            connect_btn->labelcolor(FL_WHITE);
        } else {
            status_box->color(dark ? fl_rgb_color(50, 55, 68) : fl_rgb_color(205, 212, 224));
            status_box->labelcolor(dark ? fl_rgb_color(180, 190, 205) : fl_rgb_color(60, 70, 85));
            connect_btn->color(dark ? fl_rgb_color(36, 140, 76) : fl_rgb_color(30, 150, 80));
            connect_btn->labelcolor(FL_WHITE);
        }
    }

    void updateMetricButtons() {
        const auto& keys = chart->getMetricKeys();
        bool dark = is_dark_mode;

        metric_btn_group->clear();
        metric_btn_group->begin();

        int bx = metric_btn_group->x();
        int by = metric_btn_group->y();
        int bh = 26;

        for (size_t i = 0; i < keys.size(); ++i) {
            const std::string& key = keys[i];
            Fl_Color col = chart->getMetricColor(i, dark);

            int bw = std::max(75, static_cast<int>(key.length() * 8 + 14));
            auto* btn = new Fl_Button(bx, by, bw, bh);
            btn->copy_label(key.c_str());
            btn->type(FL_TOGGLE_BUTTON);
            bool is_on = chart->isMetricEnabled(key);
            btn->value(is_on ? 1 : 0);
            btn->box(FL_BORDER_BOX);

            if (is_on) {
                btn->color(dark ? fl_rgb_color(34, 40, 52) : fl_rgb_color(240, 244, 252));
                btn->labelcolor(col);
            } else {
                btn->color(dark ? fl_rgb_color(22, 25, 32) : fl_rgb_color(210, 216, 226));
                btn->labelcolor(dark ? fl_rgb_color(90, 100, 115) : fl_rgb_color(120, 130, 145));
            }
            btn->labelfont(FL_HELVETICA_BOLD);
            btn->callback(metric_toggle_cb, this);

            bx += bw + 5;
        }

        metric_btn_group->end();
        toolbar_panel->redraw();
    }

    static void theme_cb(Fl_Widget*, void* data) {
        auto* self = static_cast<AudioMonitor*>(data);
        self->is_dark_mode = !self->is_dark_mode;
        self->applyTheme();
    }

    static void metric_toggle_cb(Fl_Widget* w, void* data) {
        auto* btn = static_cast<Fl_Button*>(w);
        auto* self = static_cast<AudioMonitor*>(data);
        std::string key = btn->label();

        if (btn->value()) {
            self->chart->enableMetric(key);
        } else {
            self->chart->disableMetric(key);
        }
        self->updateMetricButtons();
    }

    static void mode_cb(Fl_Widget*, void* data) {
        auto* self = static_cast<AudioMonitor*>(data);
        bool next_overlay = !self->chart->getOverlayMode();
        self->chart->setOverlayMode(next_overlay);
        self->mode_btn->label(next_overlay ? "Mode: Overlay" : "Mode: Stacked");
    }

    static void clear_cb(Fl_Widget*, void* data) {
        auto* self = static_cast<AudioMonitor*>(data);
        self->chart->clear();
        self->refresh_count = 0;
        self->updateStatus();
    }

    static void connect_cb(Fl_Widget*, void* data) {
        static_cast<AudioMonitor*>(data)->toggleConnect();
    }

    void toggleConnect() {
        if (!udp.isConnected()) {
            std::string ip = ip_input->value() ? ip_input->value() : "";
            int port = 0;
            if (port_input->value() && port_input->value()[0] != '\0') {
                try {
                    port = std::stoi(port_input->value());
                } catch (...) {
                    port = 0;
                }
            }
            if (port <= 0 || ip.empty()) return;

            if (udp.connect(ip, port)) {
                udp.send("log 1");
                status_box->label("Connected");
                connect_btn->label("Disconnect");
                updateConnectColors();

                refresh_count = 0;
                updateStatus();
                Fl::add_timeout(0.5, refresh_cb, this);
            }
        } else {
            udp.disconnect();
            status_box->label("Disconnected");
            connect_btn->label("Connect");
            updateConnectColors();

            chart->clear();
            Fl::remove_timeout(refresh_cb, this);
        }
    }

    static void refresh_cb(void* data) {
        auto* self = static_cast<AudioMonitor*>(data);
        if (self->udp.isConnected()) {
            self->udp.send("/a?");
            std::string resp = self->udp.receive(800);
            if (!resp.empty()) {
                auto metrics = self->parseMetrics(resp);
                if (!metrics.empty()) {
                    self->chart->addDataPoint(metrics);
                    self->updateMetricButtons();
                    self->refresh_count++;
                    self->last_response_time = std::chrono::system_clock::now();
                    self->updateStatus();
                }
            }

            double period = 2.0;
            if (self->refresh_input->value() && self->refresh_input->value()[0] != '\0') {
                try {
                    period = std::stod(self->refresh_input->value());
                } catch (...) {
                    period = 2.0;
                }
            }
            if (period < 0.2) period = 0.5;
            Fl::add_timeout(period, refresh_cb, self);
        }
    }

    void updateStatus() {
        refresh_count_out->value(std::to_string(refresh_count).c_str());

        if (last_response_time.time_since_epoch().count() == 0) {
            last_response_out->value("never");
        } else {
            auto t = std::chrono::system_clock::to_time_t(last_response_time);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&t), "%H:%M:%S");
            last_response_out->value(oss.str().c_str());
        }
    }

    std::map<std::string, double> parseMetrics(const std::string& data) {
        std::map<std::string, double> metrics;
        auto extract = [&](const std::string& prefix) -> double {
            size_t pos = data.find(prefix);
            if (pos == std::string::npos) return 0.0;
            pos = data.find_first_of("0123456789.-", pos);
            if (pos == std::string::npos) return 0.0;
            try { return std::stod(data.substr(pos)); } catch (...) { return 0.0; }
        };

        metrics["overruns"]   = extract("overruns:");
        metrics["cb_ms_last"] = extract("last");
        metrics["cb_ms_avg"]  = extract("avg");
        metrics["load_last"]  = extract("last");
        metrics["load_avg"]   = extract("avg");
        metrics["late_starts"]= extract("late-starts");
        metrics["discont"]    = extract("discontinuities");
        metrics["clipped"]    = extract("clipped-samples");

        return metrics;
    }

private:
    Fl_Group* top_panel;
    Fl_Input* ip_input;
    Fl_Int_Input* port_input;
    Fl_Int_Input* refresh_input;
    Fl_Button* connect_btn;
    Fl_Box* status_box;
    Fl_Box* sep_box;
    Fl_Output* refresh_count_out;
    Fl_Output* last_response_out;

    Fl_Group* toolbar_panel;
    Fl_Box* toolbar_label;
    Fl_Group* metric_btn_group;
    Fl_Button* mode_btn;
    Fl_Button* theme_btn;
    Fl_Button* clear_btn;

    StripChart* chart;
    UdpClient udp;
    int refresh_count = 0;
    bool is_dark_mode = true;
    std::chrono::system_clock::time_point last_response_time;
};

int main(int argc, char** argv) {
    AudioMonitor win;
    win.show(argc, argv);
    return Fl::run();
}
