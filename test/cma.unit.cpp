#include <memory>
#include <gtest/gtest.h>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "wincalc/wincalc.h"
#include "util.h"
#include "paths.h"


using namespace wincalc;
using namespace window_standards;

class TestCMA : public testing::Test
{
protected:
    thmxParser::ThmxFileContents top_frame;
    thmxParser::ThmxFileContents bottom_frame;
    thmxParser::ThmxFileContents jamb_frame;
    thmxParser::ThmxFileContents spacer;

    virtual void SetUp()
    {
        std::filesystem::path top_frame_path(test_dir);
        top_frame_path /= "products";
        top_frame_path /= "sample-head_CMA.thmx";

        std::filesystem::path bottom_frame_path(test_dir);
        bottom_frame_path /= "products";
        bottom_frame_path /= "sample-sill_CMA.thmx";

        std::filesystem::path jamb_frame_path(test_dir);
        jamb_frame_path /= "products";
        jamb_frame_path /= "sample-jamb_CMA.thmx";

        std::filesystem::path spacer_path(test_dir);
        spacer_path /= "products";
        spacer_path /= "Spacer_CMA.thmx";

        top_frame = thmxParser::parseFile(top_frame_path.string());
        bottom_frame = thmxParser::parseFile(bottom_frame_path.string());
        jamb_frame = thmxParser::parseFile(jamb_frame_path.string());

        spacer = thmxParser::parseFile(spacer_path.string());
    }
};

TEST_F(TestCMA, Test_CMA_Single_Vision)
{
    // These values are from
    // https://github.com/LBNL-ETA/Windows-CalcEngine/blob/Version_1.0.20/src/Tarcog/tst/units/CMASingleVisionWindow.unit.cpp
    double glazing_system_u = 1.25800;
    double glazing_system_shgc = 0.341;
    double tvis = 0.53500;   // direct-hemispheric visible front transmittance
    double width = 1.2;
    double height = 1.5;
    std::shared_ptr<CMA::CMAWindow> cma_window = wincalc::get_cma_window_single_vision(
      top_frame, bottom_frame, jamb_frame, jamb_frame, width, height);
    auto spacer_keff = wincalc::get_spacer_keff(spacer);

    auto results =
      wincalc::calc_cma(cma_window, glazing_system_u, glazing_system_shgc, tvis, spacer_keff);
    EXPECT_NEAR(results.u, 1.451714, 1e-6);
    EXPECT_NEAR(results.shgc, 0.299620, 1e-6);
    EXPECT_NEAR(results.vt, 0.468371, 1e-6);
}

TEST_F(TestCMA, Test_CMA_Double_Vision_Vertical)
{
    // These values are from
    // https://github.com/LBNL-ETA/Windows-CalcEngine/blob/Version_1.0.20/src/Tarcog/tst/units/CMADoubleVisionVerticalWindow.unit.cpp
    double glazing_system_u = 1.25800;
    double glazing_system_shgc = 0.341;
    double tvis = 0.53500;   // direct-hemispheric visible front transmittance
    double width = 1.2;
    double height = 1.5;

    auto spacer_keff = wincalc::get_spacer_keff(spacer);

    std::shared_ptr<CMA::CMAWindow> cma_window =
      wincalc::get_cma_window_double_vision_vertical(top_frame,
                                                     bottom_frame,
                                                     jamb_frame,
                                                     jamb_frame,
                                                     jamb_frame,
                                                     jamb_frame,
                                                     jamb_frame,
                                                     width,
                                                     height);

    auto results =
      wincalc::calc_cma(cma_window, glazing_system_u, glazing_system_shgc, tvis, spacer_keff);

    EXPECT_NEAR(results.u, 1.511768, 1e-6);
    EXPECT_NEAR(results.shgc, 0.290800, 1e-6);
    EXPECT_NEAR(results.vt, 0.454171, 1e-6);
}

TEST_F(TestCMA, Test_CMA_Double_Vision_Horizontal)
{
    // These values are from
    // https://github.com/LBNL-ETA/Windows-CalcEngine/blob/Version_1.0.20/src/Tarcog/tst/units/CMADoubleVisionHorizontalWindow.unit.cpp
    double glazing_system_u = 1.25800;
    double glazing_system_shgc = 0.341;
    double tvis = 0.535;   // direct-hemispheric visible front transmittance
    double width = 1.5;
    double height = 1.2;

    auto spacer_keff = wincalc::get_spacer_keff(spacer);

    std::shared_ptr<CMA::CMAWindow> cma_window =
      wincalc::get_cma_window_double_vision_horizontal(top_frame,
                                                       top_frame,
                                                       bottom_frame,
                                                       bottom_frame,
                                                       jamb_frame,
                                                       jamb_frame,
                                                       jamb_frame,
                                                       width,
                                                       height);

    auto results =
      wincalc::calc_cma(cma_window, glazing_system_u, glazing_system_shgc, tvis, spacer_keff);

    EXPECT_NEAR(results.u, 1.512250, 1e-6);
    EXPECT_NEAR(results.shgc, 0.290802, 1e-6);
    EXPECT_NEAR(results.vt, 0.454171, 1e-6);
}
