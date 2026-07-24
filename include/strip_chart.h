#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <vector>
#include <string>
#include <map>

class StripChart : public Fl_Box {
public:
    StripChart(int x, int y, int w, int h, const char* label = 0);
    ~StripChart();

    void addDataPoint(const std::map<std::string, double>& metrics);
    void clear();

protected:
    void draw() override;

private:
    std::vector<std::map<std::string, double>> history;
    std::vector<std::string> metric_keys;
    int max_points = 500;
    double min_y = 0, max_y = 100;
};
