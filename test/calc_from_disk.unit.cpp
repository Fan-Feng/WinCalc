#include <memory>
#include <gtest/gtest.h>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <filesystem>

#include "wincalc/wincalc.h"

#include "paths.h"

class TestCalcFromDisk : public testing::Test
{
protected:
    virtual void SetUp()
    {}
};


TEST_F(TestCalcFromDisk, Test_NFRC_103_optics)
{
    SCOPED_TRACE("Begin Test: Single clear NFRC 103 using optics file as data source.");
    std::filesystem::path clear_3_path(test_dir);
    clear_3_path /= "products";
    clear_3_path /= "CLEAR_3.DAT";
    std::vector<OpticsParser::ProductData> products;
    OpticsParser::ProductData clear_3 = OpticsParser::parseFile(clear_3_path.string());
    products.push_back(clear_3);

    std::vector<Gap_Data> gaps;

    std::filesystem::path standard_path(test_dir);
    standard_path /= "standards";
    standard_path /= "W5_NFRC_2003.std";

    Standard standard = load_standard(standard_path);

    Thermal_Result u_result = calc_u(products, gaps, standard, 1.0, 1.0);
    EXPECT_NEAR(u_result.result, 5.9125145552954441, 1e-14);
    EXPECT_NEAR(u_result.t_sol, 0.83380702773635118, 1e-14);
    EXPECT_NEAR(u_result.layer_solar_absorptances[0], 0.091376375800192963, 1e-14);

    Thermal_Result shgc_result = calc_shgc(products, gaps, standard, 1.0, 1.0);
    EXPECT_NEAR(shgc_result.result, 0.86058891721415542, 1e-14);
    EXPECT_NEAR(shgc_result.t_sol, 0.83380702773635118, 1e-14);
    EXPECT_NEAR(shgc_result.layer_solar_absorptances[0], 0.091376375800192963, 1e-14);
}

TEST_F(TestCalcFromDisk, Test_NFRC_103_103_optics)
{
    SCOPED_TRACE("Begin Test: Double clear NFRC 103-103 using optics file as data source.");

    std::filesystem::path clear_3_path(test_dir);
    clear_3_path /= "products";
    clear_3_path /= "CLEAR_3.DAT";
    std::vector<OpticsParser::ProductData> products;
    OpticsParser::ProductData clear_3 = OpticsParser::parseFile(clear_3_path.string());
    products.push_back(clear_3);
    products.push_back(clear_3);

    Gap_Data air_gap{Gas_Type::AIR, 0.0127};
    std::vector<Gap_Data> gaps;
    gaps.push_back(air_gap);

    std::filesystem::path standard_path(test_dir);
    standard_path /= "standards";
    standard_path /= "W5_NFRC_2003.std";

    Standard standard = load_standard(standard_path);

    Thermal_Result u_result = calc_u(products, gaps, standard, 1.0, 1.0);
    EXPECT_NEAR(u_result.result, 2.7296194478984446, 1e-14);
    EXPECT_NEAR(u_result.t_sol, 0.70324342292094888, 1e-14);
    EXPECT_NEAR(u_result.layer_solar_absorptances[0], 0.096478606342105686, 1e-14);
    EXPECT_NEAR(u_result.layer_solar_absorptances[1], 0.072235301276579358, 1e-14);

    Thermal_Result shgc_result = calc_shgc(products, gaps, standard, 1.0, 1.0);
    EXPECT_NEAR(shgc_result.result, 0.76323563155897300, 1e-14);
    EXPECT_NEAR(shgc_result.t_sol, 0.70324342292094888, 1e-14);
    EXPECT_NEAR(shgc_result.layer_solar_absorptances[0], 0.096478606342105686, 1e-14);
    EXPECT_NEAR(shgc_result.layer_solar_absorptances[1], 0.072235301276579358, 1e-14);
}
