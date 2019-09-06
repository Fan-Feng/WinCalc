
#include <algorithm>
#include <iostream>
#include <sstream>

#include <WCEGases.hpp>
#include <WCETarcog.hpp>

#include "create_wce_objects.h"
#include "thermal_calcs.h"
#include "environmental_conditions.h"

namespace wincalc
{

    Tarcog::ISO15099::CSystem create_system(Tarcog::ISO15099::CIGU & igu,
                                            Environments const & environments)
    {
        auto indoor = create_indoor_environment(environments.inside);
        auto outdoor = create_outdoor_environment(environments.outside);
        auto system = Tarcog::ISO15099::CSystem(igu, indoor, outdoor);
        return system;
    }

    double calc_u_iso15099(Tarcog::ISO15099::CIGU & igu, Environments const & environments)
    {
        Tarcog::ISO15099::CSystem system = create_system(igu, environments);
        double u = system.getUValue();
        return u;
    }

    double calc_shgc_iso15099(Tarcog::ISO15099::CIGU & igu,
                              double t_sol,
                              Environments const & environments)
    {
        Tarcog::ISO15099::CSystem system = create_system(igu, environments);
        return system.getSHGC(t_sol);
    }

    Tarcog::ISO15099::CIGU create_igu(
      std::vector<std::shared_ptr<Tarcog::ISO15099::CIGUSolidLayer>> const & solid_layers,
      std::vector<std::shared_ptr<Tarcog::ISO15099::CIGUGapLayer>> const & gaps,
      double width,
      double height)
    {
        Tarcog::ISO15099::CIGU igu(width, height);
        igu.addLayer(solid_layers[0]);
        if(!gaps.empty())
        {
            for(size_t i = 0; i < gaps.size(); ++i)
            {
                // if there are gaps then there should be one more solid layer
                // than gap layer.  So we can just add a gap then a solid layer
                // at the next index until the end of the gaps
                igu.addLayer(gaps[i]);
                igu.addLayer(solid_layers[i + 1]);
            }
        }
        return igu;
    }

    struct IGU_Info
    {
        Tarcog::ISO15099::CIGU igu;
        double t_sol;
    };

    IGU_Info create_igu(std::vector<OpticsParser::ProductData> const & layers,
                        std::vector<Engine_Gap_Info> const & gaps,
                        double width,
                        double height,
                        window_standards::Optical_Standard const & standard)
    {
        auto solar_method = standard.methods.at(window_standards::Optical_Standard_Method_Type::SOLAR);
        auto multi_pane_specular = create_multi_pane_specular(layers, solar_method);

        double t_sol =
          multi_pane_specular->getPropertySimple(FenestrationCommon::PropertySimple::T,
                                                 FenestrationCommon::Side::Front,
                                                 FenestrationCommon::Scattering::DirectDirect,
                                                 0,
                                                 0);

        std::vector<std::shared_ptr<Tarcog::ISO15099::CIGUSolidLayer>> tarcog_solid_layers;

        for(size_t i = 0; i < layers.size(); ++i)
        {
            double absorbtance =
              multi_pane_specular->getAbsorptanceLayer(i + 1,
                                                       FenestrationCommon::Side::Front,
                                                       FenestrationCommon::ScatteringSimple::Direct,
                                                       0,
                                                       0);
            double thickness = layers[i].thickness / 1000.0;
            double conductivity =
              layers[i].conductivity.value();   // Conductivity must be set somewhere before here,
                                                // hopefully by whatever is loading the product data
            auto layer = Tarcog::ISO15099::Layers::solid(thickness,
                                                         conductivity,
                                                         layers[i].frontEmissivity,
                                                         layers[i].IRTransmittance,
                                                         layers[i].backEmissivity,
                                                         layers[i].IRTransmittance);
            layer->setSolarAbsorptance(absorbtance);
            tarcog_solid_layers.push_back(layer);
        }

        std::vector<std::shared_ptr<Tarcog::ISO15099::CIGUGapLayer>> tarcog_gaps;
        for(Engine_Gap_Info Engine_Gap_Info : gaps)
        {
            tarcog_gaps.push_back(Tarcog::ISO15099::Layers::gap(
              Engine_Gap_Info.thickness, Gases::CGas({{1.0, Engine_Gap_Info.gas}})));
        }

        return IGU_Info{create_igu(tarcog_solid_layers, tarcog_gaps, width, height), t_sol};
    }

    Thermal_Result assemble_thermal_result(double result, IGU_Info const & igu_info)
    {
        double t_sol = igu_info.t_sol;
        std::vector<double> layer_solar_absorptances;
        std::vector<std::shared_ptr<Tarcog::ISO15099::CIGUSolidLayer>> solid_layers =
          igu_info.igu.getSolidLayers();
        for(auto const & layer : solid_layers)
        {
            layer_solar_absorptances.push_back(layer->getSolarAbsorptance());
        }
        return Thermal_Result{result, t_sol, layer_solar_absorptances};
    }

    Thermal_Result calc_u_iso15099(std::vector<OpticsParser::ProductData> const & layers,
                                   std::vector<Engine_Gap_Info> const & gaps,
                                   double width,
                                   double height,
                                   window_standards::Optical_Standard const & standard,
                                   Environments const & environments)
    {
        IGU_Info igu_info = create_igu(layers, gaps, width, height, standard);
        double u = calc_u_iso15099(igu_info.igu, environments);
        return assemble_thermal_result(u, igu_info);
    }

    Thermal_Result calc_shgc_iso15099(std::vector<OpticsParser::ProductData> const & layers,
                                      std::vector<Engine_Gap_Info> const & gaps,
                                      double width,
                                      double height,
                                      window_standards::Optical_Standard const & standard,
                                      Environments const & environments)
    {
        IGU_Info igu_info = create_igu(layers, gaps, width, height, standard);
        double shgc = calc_shgc_iso15099(igu_info.igu, igu_info.t_sol, environments);
        return assemble_thermal_result(shgc, igu_info);
    }
}   // namespace wincalc
