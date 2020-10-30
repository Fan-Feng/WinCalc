#include <memory>
#include <gtest/gtest.h>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "wincalc/wincalc.h"
#include "optical_calcs.h"
#include "util.h"
#include "convert_optics_parser.h"
#include "paths.h"


using namespace wincalc;
using namespace window_standards;

class TestUserWovenNFRC102 : public testing::Test
{
protected:
    std::shared_ptr<Glazing_System> glazing_system_u;
    std::shared_ptr<Glazing_System> glazing_system_shgc;

    virtual void SetUp()
    {
        std::filesystem::path clear_3_path(test_dir);
        clear_3_path /= "products";
        clear_3_path /= "CLEAR_3.json";

        // Use the same material as in the venetian test above for consistancy with WCE tests
        std::filesystem::path venetian_material_path(test_dir);
        venetian_material_path /= "products";
        venetian_material_path /= "igsdb_12852.json";

        std::vector<std::shared_ptr<OpticsParser::ProductData>> parsed_products;
        OpticsParser::Parser parser;
        auto clear_3 = parser.parseJSONFile(clear_3_path.string());
        auto shade_material = parser.parseJSONFile(venetian_material_path.string());
        auto shade_geometry = std::make_shared<OpticsParser::WovenGeometry>(0.002, 0.003, 0.002);
        auto shade_composition_info = std::shared_ptr<OpticsParser::CompositionInformation>(
          new OpticsParser::CompositionInformation{shade_material, shade_geometry});
        OpticsParser::ProductData shade_layer_info(
          "User Woven Shade", "shading", "User Manufacturer");
        auto woven_shade = std::make_shared<OpticsParser::ComposedProductData>(
          shade_layer_info, shade_composition_info);

        parsed_products.push_back(woven_shade);
        parsed_products.push_back(clear_3);

        auto converted_products = convert_to_solid_layers(parsed_products);

        Engine_Gap_Info air_gap(Gases::GasDef::Air, 0.0127);
        std::vector<Engine_Gap_Info> gaps;
        gaps.push_back(air_gap);

        std::filesystem::path standard_path(test_dir);
        standard_path /= "standards";
        standard_path /= "W5_NFRC_2003.std";
        Optical_Standard standard = load_optical_standard(standard_path.string());

        auto bsdf_hemisphere =
          SingleLayerOptics::CBSDFHemisphere::create(SingleLayerOptics::BSDFBasis::Quarter);

        glazing_system_u = std::make_shared<Glazing_System>(
          converted_products, gaps, standard, 1.0, 1.0, nfrc_u_environments(), bsdf_hemisphere);
        glazing_system_shgc = std::make_shared<Glazing_System>(
          converted_products, gaps, standard, 1.0, 1.0, nfrc_shgc_environments(), bsdf_hemisphere);
    }
};

TEST_F(TestUserWovenNFRC102, Test_U)
{
    auto u_result = glazing_system_u->u();
    EXPECT_NEAR(u_result, 2.9148823179439018, 1e-6);

    auto u_layer_effective_conductivity_solid_layers =
      glazing_system_u->solid_layers_effective_conductivities(Tarcog::ISO15099::System::Uvalue);
    EXPECT_NEAR(u_layer_effective_conductivity_solid_layers[0], 146.74330655757495, 1e-6);
    EXPECT_NEAR(u_layer_effective_conductivity_solid_layers[1], 1, 1e-6);

    auto u_layer_effective_conductivity_gap_layers =
      glazing_system_u->gap_layers_effective_conductivities(Tarcog::ISO15099::System::Uvalue);
    EXPECT_NEAR(u_layer_effective_conductivity_gap_layers[0], 0.070327899269384583, 1e-6);

    auto u_system_effective_conductivity =
      glazing_system_u->system_effective_conductivity(Tarcog::ISO15099::System::Uvalue);
    EXPECT_NEAR(u_system_effective_conductivity, 0.10040701238466257, 1e-6);
}

TEST_F(TestUserWovenNFRC102, Test_SHGC)
{
    auto shgc_result = glazing_system_shgc->shgc();
    EXPECT_NEAR(shgc_result, 0.14718304092197346, 1e-6);
}

TEST_F(TestUserWovenNFRC102, Test_Solar)
{
	WCE_Optical_Results solar_results =
		glazing_system_u->optical_method_results(Optical_Standard_Method_Type::SOLAR);

	EXPECT_NEAR(solar_results.system_results.front.transmittance.direct_hemispherical,
		0.098171536948485016,
		1e-6);
	EXPECT_NEAR(solar_results.system_results.back.transmittance.direct_hemispherical, 0.7867,
		1e-6);
	EXPECT_NEAR(solar_results.system_results.front.reflectance.direct_hemispherical,
		0.6065899265825383,
		1e-6);
	EXPECT_NEAR(solar_results.system_results.back.reflectance.direct_hemispherical,
		0.50146963524036026,
		1e-6);
	EXPECT_NEAR(solar_results.system_results.front.transmittance.direct_diffuse, 0.786517, 1e-6);
	EXPECT_NEAR(solar_results.system_results.back.transmittance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(solar_results.system_results.front.reflectance.direct_diffuse, 0.073785, 1e-6);
	EXPECT_NEAR(solar_results.system_results.back.reflectance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(solar_results.system_results.front.transmittance.diffuse_diffuse, 0.708676,
		1e-6); EXPECT_NEAR(solar_results.system_results.back.transmittance.diffuse_diffuse,
			0.63794508795388927, 1e-6);
	EXPECT_NEAR(solar_results.system_results.front.reflectance.diffuse_diffuse, 0.138815, 1e-6);
	EXPECT_NEAR(solar_results.system_results.back.reflectance.diffuse_diffuse,
		0.14709734859263215, 1e-6);
	EXPECT_NEAR(solar_results.layer_results[0].front.absorptance.direct, 0.28441079217097104, 1e-6);
	EXPECT_NEAR(
		solar_results.layer_results[1].front.absorptance.direct, 0.010827744298005591, 1e-6);
}

TEST_F(TestUserWovenNFRC102, Test_Visible)
{
	WCE_Optical_Results photopic_results =
		glazing_system_u->optical_method_results(Optical_Standard_Method_Type::PHOTOPIC);

	EXPECT_NEAR(photopic_results.system_results.front.transmittance.direct_hemispherical,
		0.10723787565441972,
		1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.transmittance.direct_direct, 0.8502, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.front.reflectance.direct_hemispherical,
		0.66231650848248969,
		1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.reflectance.direct_hemispherical,
		0.63235320261454742,
		1e-6);
	EXPECT_NEAR(photopic_results.system_results.front.transmittance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.transmittance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.front.reflectance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.reflectance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.front.transmittance.diffuse_diffuse,
		0.78935221564455493, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.transmittance.diffuse_diffuse,
		0.78935221564455493, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.front.reflectance.diffuse_diffuse,
		0.13525667684831866, 1e-6);
	EXPECT_NEAR(photopic_results.system_results.back.reflectance.diffuse_diffuse,
		0.13525667684831866, 1e-6);
}

TEST_F(TestUserWovenNFRC102, Test_TUV)
{
	WCE_Optical_Results tuv_results =
		glazing_system_u->optical_method_results(Optical_Standard_Method_Type::TUV);
	EXPECT_NEAR(tuv_results.system_results.front.transmittance.direct_hemispherical,
		0.079820547642344905,
		1e-6);

	EXPECT_NEAR(tuv_results.system_results.front.transmittance.direct_direct, 0.58733483792234731, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.transmittance.direct_direct, 0.58733483792234731, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.front.reflectance.direct_direct, 0.062256898186217748, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.reflectance.direct_direct, 0.062256898186217748, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.front.transmittance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.transmittance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.front.reflectance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.reflectance.direct_diffuse, 0.0, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.front.transmittance.diffuse_diffuse, 0.51679983061465506, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.transmittance.diffuse_diffuse, 0.51679983061465506, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.front.reflectance.diffuse_diffuse, 0.11475903289790138, 1e-6);
	EXPECT_NEAR(tuv_results.system_results.back.reflectance.diffuse_diffuse, 0.11475903289790138, 1e-6);
}

TEST_F(TestUserWovenNFRC102, Test_Color)
{
    WCE_Color_Results color_results = glazing_system_u->color();

    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.lab.L, 94.666217263591847, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.lab.a, 1.0038394903462700, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.lab.b, 2.3170364131629428, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.rgb.R, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.rgb.G, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.rgb.B, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.trichromatic.X, 83.028312591606294, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.trichromatic.Y, 86.830285384922604, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_direct.trichromatic.Z, 91.092529581183740, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.lab.L, 94.666217263591847, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.lab.a, 1.0038394903462700, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.lab.b, 2.3170364131629428, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.rgb.R, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.rgb.G, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.rgb.B, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.trichromatic.X, 83.028312591606294, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.trichromatic.Y, 86.830285384922604, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_direct.trichromatic.Z, 91.092529581183740, 1e-6);

    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.lab.L, 32.685672100834950, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.lab.a, -13.456067709025742, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.lab.b, -9.4328475174762723, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.rgb.R, 38, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.rgb.G, 92, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.rgb.B, 101, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.trichromatic.X, 5.7584248621303411, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.trichromatic.Y, 7.3931504074726693, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_direct.trichromatic.Z, 11.074221343637136, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.lab.L, 32.685672100834950, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.lab.a, -13.456067709025742, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.lab.b, -9.4328475174762723, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.rgb.R, 38, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.rgb.G, 92, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.rgb.B, 101, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.trichromatic.X, 5.7584248621303411, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.trichromatic.Y, 7.3931504074726693, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_direct.trichromatic.Z, 11.074221343637136, 1e-6);

    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.lab.a, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.lab.b, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.lab.L, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.rgb.R, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.rgb.G, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.rgb.B, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.trichromatic.X, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.trichromatic.Y, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.direct_diffuse.trichromatic.Z, 0.0, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.lab.a, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.lab.b, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.lab.L, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.rgb.R, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.rgb.G, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.rgb.B, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.trichromatic.X, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.trichromatic.Y, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.direct_diffuse.trichromatic.Z, 0.0, 1e-6);

    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.lab.a, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.lab.b, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.lab.L, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.rgb.R, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.rgb.G, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.rgb.B, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.trichromatic.X, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.trichromatic.Y, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.direct_diffuse.trichromatic.Z, 0.0, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.lab.a, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.lab.b, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.lab.L, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.rgb.R, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.rgb.G, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.rgb.B, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.trichromatic.X, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.trichromatic.Y, 0.0, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.direct_diffuse.trichromatic.Z, 0.0, 1e-6);

    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.lab.L, 91.205381649557779, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.lab.a, 0.81757398116044655, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.lab.b, 2.2108499191388242, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.rgb.R, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.rgb.G, 253, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.rgb.B, 249, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.trichromatic.X, 75.404098662194983, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.trichromatic.Y, 78.936123385628918, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.transmittance.diffuse_diffuse.trichromatic.Z, 82.856762497727459, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.lab.L, 91.205381649557779, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.lab.a, 0.81757398116044655, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.lab.b, 2.2108499191388242, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.rgb.R, 255, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.rgb.G, 253, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.rgb.B, 249, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.trichromatic.X, 75.404098662194983, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.trichromatic.Y, 78.936123385628918, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.transmittance.diffuse_diffuse.trichromatic.Z, 82.856762497727459, 1e-6);

    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.lab.L, 43.540458616337347, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.lab.a, -9.1523474616345695, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.lab.b, -6.2165296319391494, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.rgb.R, 87, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.rgb.G, 119, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.rgb.B, 125, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.trichromatic.X, 11.523015097588450, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.trichromatic.Y, 13.522673394145196, 1e-6);
    EXPECT_NEAR(color_results.system_results.front.reflectance.diffuse_diffuse.trichromatic.Z, 17.554745273890433, 1e-6);

    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.lab.L, 43.540458616337347, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.lab.a, -9.1523474616345695, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.lab.b, -6.2165296319391494, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.rgb.R, 87, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.rgb.G, 119, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.rgb.B, 125, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.trichromatic.X, 11.523015097588450, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.trichromatic.Y, 13.522673394145196, 1e-6);
    EXPECT_NEAR(color_results.system_results.back.reflectance.diffuse_diffuse.trichromatic.Z, 17.554745273890433, 1e-6);
}