#pragma once

#include <cstdint>
#include <iterator>
#include <cstddef>
#include <limits>
#include <cassert>
#include <cmath>

inline constexpr double gravity = 9.81; //m/s²
inline constexpr double joulesPerEnergyUnit = 0.00001;
inline constexpr double voxelEdgeLength = 0.01;
inline constexpr double sleepThreshold = 0.1; //K

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

inline double energyToTemperature(uint32_t energy, MaterialType material){
    return static_cast<double>(energy) * joulesPerEnergyUnit / (materialProperties[static_cast<size_t>(material)].density * 
    (voxelEdgeLength * voxelEdgeLength * voxelEdgeLength) * materialProperties[static_cast<size_t>(material)].specificHeatCapacity);
}

inline uint32_t temperatureToEnergy(double temperature, MaterialType material){
    assert(temperature >= 0 && "Temperature can't be negative.");
    const Material& properties = materialProperties[static_cast<size_t>(material)];
    double energy = std::round(temperature * properties.density * (voxelEdgeLength * voxelEdgeLength * voxelEdgeLength) * 
    properties.specificHeatCapacity / joulesPerEnergyUnit);
    assert(energy <= std::numeric_limits<uint32_t>::max() && "Energy is bigger than uint32_t.");  
    return static_cast<uint32_t>(energy);
}

inline int64_t heightEnergy(const MaterialType material, int heightInCells){
    const double mass = materialProperties[static_cast<size_t>(material)].density * (voxelEdgeLength * voxelEdgeLength * voxelEdgeLength);
    const double height = heightInCells * voxelEdgeLength;
    const double joules = mass * gravity * height;
    return std::llround(joules / joulesPerEnergyUnit);
}

static_assert(std::size(materialProperties) == static_cast<std::size_t>(MaterialType::COUNT),
"Materialtype is out of sync with materialProperties.");

