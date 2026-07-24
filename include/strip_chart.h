#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <vector>
#include <string>
#include <map>
#include <set>

class StripChart : public Fl_Box {
public:
    StripChart(int x, int y, int w, int h, const char* label = nullptr);
    ~StripChart();

    void addDataPoint(const std::map<std::string, double>& metrics);
    void clear();
    void enableMetric(const std::string& key);
    void disableMetric(const std::string& key);
    bool isMetricEnabled(const std::string& key) const;

    const std::vector<std::string>& getMetricKeys() const { return metric_keys; }
    void initDefaultKeys(const std::vector<std::string>& keys);

    static Fl_Color getMetricColor(size_t index, bool is_dark = true);
    Fl_Color getMetricColor(const std::string& key) const;

    void setOverlayMode(bool overlay);
    bool getOverlayMode() const { return overlay_mode; }

    void setDarkMode(bool dark);
    bool isDarkMode() const { return dark_mode; }

protected:
    void draw() override;

private:
    std::vector<std::map<std::string, double>> history;
    std::vector<std::string> metric_keys;
    std::set<std::string> disabled_metrics;
    int max_points = 300;
    bool overlay_mode = false;
    bool dark_mode = true;

    void drawStackedPlots();
    void drawOverlayPlot();
    void drawEmptyState(const char* message);
};
