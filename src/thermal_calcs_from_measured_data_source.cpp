#include <algorithm>
#include <locale>
#include <windows_standards/windows_standard.h>
#include <OpticsParser.hpp>

#include "thermal_calcs_from_measured_data_source.h"
#include "thermal_calcs.h"

namespace wincalc
{
    Thermal_Result calc_u(std::vector<OpticsParser::ProductData> const & products,
                          std::vector<Gap_Data> const & gap_values,
                          window_standards::Optical_Standard const & standard,
                          double width,
                          double height,
                          Environments const & environments)
    {
        std::vector<Engine_Gap_Info> gaps = convert(gap_values);
        return calc_u_iso15099(products, gaps, width, height, standard, environments);
    }

    Thermal_Result calc_shgc(std::vector<OpticsParser::ProductData> const & products,
                             std::vector<Gap_Data> const & gap_values,
                             window_standards::Optical_Standard const & standard,
                             double width,
                             double height,
                             Environments const & environments)
    {
        std::vector<Engine_Gap_Info> gaps = convert(gap_values);
        return calc_shgc_iso15099(products, gaps, width, height, standard, environments);
    }
}   // namespace wincalc
