#pragma once

#include <cstdint>
#include <iterator>
#include <cstddef>

enum class MaterialType:uint8_t {
    AIR,
    STONE,

    COUNT
};

struct Material {
    double density; //kg/m³
    double specificHeatCapacity; //J/kg·K
    double conductivity; //W/m·K
};

inline constexpr Material materialProperties[] = {
    {1.2, 1005, 0.026}, //Air
    {2700, 800, 2.0} //Stone 
};

const double joulesPerEnergyUnit = 0.00001;
const double voxelEdgeLength = 0.01;

inline double energyToTemperature(uint32_t energy, MaterialType material){
    return static_cast<double>(energy) * joulesPerEnergyUnit / (materialProperties[static_cast<size_t>(material)].density * 
    (voxelEdgeLength * voxelEdgeLength * voxelEdgeLength) * materialProperties[static_cast<size_t>(material)].specificHeatCapacity);
}

static_assert(std::size(materialProperties) == static_cast<std::size_t>(MaterialType::COUNT),
"Materialtype is out of sync with materialProperties.");

