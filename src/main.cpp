#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "udp_client.h"
#include "strip_chart.h"

class AudioMonitor : public Fl_Window {
public:
    AudioMonitor() : Fl_Window(960, 680, "Audio Performance Monitor") {
        begin();

        // Controls row
        ip_input = new Fl_Input(120, 20, 160, 25, "IP Address:");
        ip_input->value("127.0.0.1");

        port_input = new Fl_Int_Input(370, 20, 80, 25, "UDP Port:");
        port_input->value("60440");

        refresh_input = new Fl_Int_Input(530, 20, 60, 25, "Refresh (s):");
        refresh_input->value("2");

        connect_btn = new Fl_Button(670, 18, 110, 30, "Connect");
        connect_btn->callback(connect_cb, this);

        status_box = new Fl_Box(810, 22, 130, 25, "Disconnected");
        status_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        // Status row
        refresh_count_box = new Fl_Box(20, 65, 200, 25, "Refresh count: 0");
        last_response_box = new Fl_Box(280, 65, 280, 25, "Last response: never");

        // Chart
        chart = new StripChart(20, 120, 920, 530, "Performance Metrics");

        end();
        resizable(this);
        size_range(700, 500);
    }

    static void connect_cb(Fl_Widget*, void* data) {
        static_cast<AudioMonitor*>(data)->toggleConnect();
    }

    void toggleConnect() {
        if (!udp.isConnected()) {
            std::string ip = ip_input->value();
            int port = std::stoi(port_input->value() ? port_input->value() : "0");
            if (port <= 0) return;

            if (udp.connect(ip, port)) {
                udp.send("log 1");
                status_box->label("Connected");
                connect_btn->label("Disconnect");
                refresh_count = 0;
                updateStatus();
                Fl::add_timeout(0.5, refresh_cb, this);
            }
        } else {
            udp.disconnect();
            status_box->label("Disconnected");
            connect_btn->label("Connect");
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
                    self->refresh_count++;
                    self->last_response_time = std::chrono::system_clock::now();
                    self->updateStatus();
                }
            }
            double period = std::stod(self->refresh_input->value() ? self->refresh_input->value() : "2");
            if (period < 0.2) period = 0.5;
            Fl::add_timeout(period, refresh_cb, self);
        }
    }

    void updateStatus() {
        std::string count_str = "Refresh count: " + std::to_string(refresh_count);
        refresh_count_box->label(count_str.c_str());

        if (last_response_time.time_since_epoch().count() == 0) {
            last_response_box->label("Last response: never");
        } else {
            auto t = std::chrono::system_clock::to_time_t(last_response_time);
            std::ostringstream oss;
            oss << "Last response: " << std::put_time(std::localtime(&t), "%H:%M:%S");
            last_response_box->label(oss.str().c_str());
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

        metrics["callbacks"] = extract("callbacks:");
        metrics["frames"] = extract("frames:");
        metrics["samples"] = extract("samples:");
        metrics["overruns"] = extract("overruns:");
        metrics["cb_ms_last"] = extract("last");
        metrics["cb_ms_avg"]  = extract("avg");
        metrics["load_last"]  = extract("last");
        metrics["load_avg"]   = extract("avg");
        metrics["late_starts"] = extract("late-starts");
        metrics["discont"] = extract("discontinuities");
        metrics["clipped"] = extract("clipped-samples");

        return metrics;
    }

private:
    Fl_Input* ip_input;
    Fl_Int_Input* port_input;
    Fl_Int_Input* refresh_input;
    Fl_Button* connect_btn;
    Fl_Box* status_box;
    Fl_Box* refresh_count_box;
    Fl_Box* last_response_box;
    StripChart* chart;
    UdpClient udp;
    int refresh_count = 0;
    std::chrono::system_clock::time_point last_response_time;
};

int main(int argc, char** argv) {
    AudioMonitor win;
    win.show(argc, argv);
    return Fl::run();
}
