#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <algorithm>

struct RainfallData {
    int year;
    double pa;      // Station A rainfall
    double avg_p10; // Average of 10 stations
    double cum_pa;  // Cumulative Station A
    double cum_avg; // Cumulative average
};

class DoubleMassCurve {
private:
    std::vector<RainfallData> data;
    int inconsistency_year = -1;
    double slope_before = 0.0;
    double slope_after = 0.0;
    double correction_factor = 1.0;
    
public:
    void inputData() {
        // Input the rainfall data from the table
        std::vector<std::tuple<int, double, double>> rainfall_data = {
            {1990, 677, 780}, {1991, 579, 660}, {1992, 96, 110}, {1993, 463, 520},
            {1994, 473, 540}, {1995, 700, 800}, {1996, 480, 540}, {1997, 432, 490},
            {1998, 494, 560}, {1999, 504, 575}, {2000, 416, 480}, {2001, 532, 600},
            {2002, 505, 580}, {2003, 829, 950}, {2004, 680, 770}, {2005, 1243, 1400},
            {2006, 998, 1140}, {2007, 572, 650}, {2008, 595, 646}, {2009, 374, 350},
            {2010, 634, 590}, {2011, 496, 490}, {2012, 385, 400}, {2013, 437, 390},
            {2014, 567, 570}, {2015, 355, 377}, {2016, 684, 653}, {2017, 824, 787},
            {2018, 425, 410}, {2019, 611, 588}
        };
        
        data.clear();
        double cum_pa = 0.0, cum_avg = 0.0;
        
        for (const auto& entry : rainfall_data) {
            RainfallData rd;
            rd.year = std::get<0>(entry);
            rd.pa = std::get<1>(entry);
            rd.avg_p10 = std::get<2>(entry);
            
            cum_pa += rd.pa;
            cum_avg += rd.avg_p10;
            
            rd.cum_pa = cum_pa;
            rd.cum_avg = cum_avg;
            
            data.push_back(rd);
        }
    }
    
    // Linear regression to calculate slope
    double calculateSlope(int start_idx, int end_idx) {
        if (start_idx >= end_idx || end_idx >= data.size()) return 0.0;
        
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
        int n = end_idx - start_idx + 1;
        
        for (int i = start_idx; i <= end_idx; i++) {
            double x = data[i].cum_avg;
            double y = data[i].cum_pa;
            
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
        }
        
        double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
        return slope;
    }
    
    // Calculate correlation coefficient
    double calculateCorrelation(int start_idx, int end_idx) {
        if (start_idx >= end_idx || end_idx >= data.size()) return 0.0;
        
        double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
        int n = end_idx - start_idx + 1;
        
        for (int i = start_idx; i <= end_idx; i++) {
            double x = data[i].cum_avg;
            double y = data[i].cum_pa;
            
            sum_x += x;
            sum_y += y;
            sum_xy += x * y;
            sum_x2 += x * x;
            sum_y2 += y * y;
        }
        
        double numerator = n * sum_xy - sum_x * sum_y;
        double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));
        
        return (denominator != 0.0) ? numerator / denominator : 0.0;
    }
    
    void detectInconsistency() {
        double max_slope_diff = 0.0;
        int best_break_point = -1;
        
        std::cout << "\nTesting break points for inconsistency:\n";
        std::cout << "Year\tSlope Before\tSlope After\tSlope Diff\tCorr1\tCorr2\n";
        std::cout << "----\t------------\t-----------\t----------\t-----\t-----\n";
        
        // Test different break points (minimum 5 years on each side)
        for (int i = 5; i < data.size() - 5; i++) {
            double slope1 = calculateSlope(0, i - 1);
            double slope2 = calculateSlope(i, data.size() - 1);
            double corr1 = calculateCorrelation(0, i - 1);
            double corr2 = calculateCorrelation(i, data.size() - 1);
            
            double slope_diff = fabs(slope2 - slope1);
            
            std::cout << data[i].year << "\t" << std::fixed << std::setprecision(4) 
                     << slope1 << "\t\t" << slope2 << "\t\t" << slope_diff 
                     << "\t\t" << corr1 << "\t" << corr2 << "\n";
            
            // Look for significant change in slope with good correlations
            // Lowered threshold and correlation requirements based on real data analysis
            if (slope_diff > max_slope_diff && corr1 > 0.85 && corr2 > 0.85) {
                max_slope_diff = slope_diff;
                best_break_point = i;
                slope_before = slope1;
                slope_after = slope2;
            }
        }
        
        // Consider data inconsistent if slope difference is significant
        // Lowered threshold based on actual data analysis
        if (max_slope_diff > 0.05) {  // More sensitive threshold
            inconsistency_year = data[best_break_point].year;
            correction_factor = slope_after / slope_before;
        }
        
        std::cout << "\nMaximum slope difference: " << std::fixed << std::setprecision(4) << max_slope_diff << "\n";
    }
    
    void correctData() {
        if (inconsistency_year == -1) return;
        
        // Find the index of inconsistency year
        int break_idx = -1;
        for (int i = 0; i < data.size(); i++) {
            if (data[i].year == inconsistency_year) {
                break_idx = i;
                break;
            }
        }
        
        if (break_idx == -1) return;
        
        std::cout << "\n**CORRECTED DATA:**\n";
        std::cout << "Correction Factor: " << std::fixed << std::setprecision(4) << correction_factor << "\n";
        std::cout << "\nYear\tOriginal PA\tCorrected PA\tAvg P10\tStatus\n";
        std::cout << "----\t-----------\t------------\t-------\t------\n";
        
        for (int i = 0; i < data.size(); i++) {
            if (i < break_idx) {
                double corrected_pa = data[i].pa * correction_factor;
                std::cout << data[i].year << "\t" << std::fixed << std::setprecision(1) 
                         << data[i].pa << "\t\t" << corrected_pa << "\t\t" 
                         << data[i].avg_p10 << "\tCorrected\n";
            } else {
                std::cout << data[i].year << "\t" << std::fixed << std::setprecision(1) 
                         << data[i].pa << "\t\t" << data[i].pa << "\t\t" 
                         << data[i].avg_p10 << "\tOriginal\n";
            }
        }
    }
    
    void displayResults() {
        std::cout << "=== DOUBLE MASS CURVE ANALYSIS RESULTS ===\n\n";
        
        // Display cumulative data (first 10 and last 10 for brevity)
        std::cout << "CUMULATIVE DATA (Sample):\n";
        std::cout << "Year\tPA\tAvg P10\tCum PA\t\tCum Avg P10\n";
        std::cout << "----\t---\t-------\t------\t\t-----------\n";
        
        for (int i = 0; i < std::min(10, (int)data.size()); i++) {
            const auto& entry = data[i];
            std::cout << entry.year << "\t" << std::fixed << std::setprecision(0) 
                     << entry.pa << "\t" << entry.avg_p10 << "\t" 
                     << std::setprecision(1) << entry.cum_pa << "\t\t" 
                     << entry.cum_avg << "\n";
        }
        
        std::cout << "...\n";
        
        for (int i = std::max(10, (int)data.size()-5); i < data.size(); i++) {
            const auto& entry = data[i];
            std::cout << entry.year << "\t" << std::fixed << std::setprecision(0) 
                     << entry.pa << "\t" << entry.avg_p10 << "\t" 
                     << std::setprecision(1) << entry.cum_pa << "\t\t" 
                     << entry.cum_avg << "\n";
        }
        
        if (inconsistency_year != -1) {
            std::cout << "\n**INCONSISTENCY DETECTED!**\n";
            std::cout << "Inconsistency starts from year: " << inconsistency_year << "\n";
            std::cout << "Slope before inconsistency: " << std::fixed << std::setprecision(4) << slope_before << "\n";
            std::cout << "Slope after inconsistency: " << std::fixed << std::setprecision(4) << slope_after << "\n";
            std::cout << "Slope change: " << std::fixed << std::setprecision(4) << fabs(slope_after - slope_before) << "\n";
            std::cout << "Correction factor: " << std::fixed << std::setprecision(4) << correction_factor << "\n";
        } else {
            std::cout << "\n**DATA IS CONSISTENT**\n";
            std::cout << "No significant inconsistency detected in the rainfall data.\n";
        }
    }
    
    void performAnalysis() {
        inputData();
        detectInconsistency();
        displayResults();
        
        if (inconsistency_year != -1) {
            correctData();
        }
    }
};

int main() {
    std::cout << "RAINFALL DATA CONSISTENCY ANALYSIS\n";
    std::cout << "Using Double Mass Curve Technique\n";
    std::cout << "==================================\n\n";
    
    DoubleMassCurve analysis;
    analysis.performAnalysis();
    
    return 0;
}
