/* Copyright Matthew Reid
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <MapFeaturesConverter/FeaturesConverter.h>
#include <iostream>

//#define PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
#ifdef PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
#include "HeightmapLeveler.h"
#endif

#define PX_SCHED_IMPLEMENTATION 1
#include <px_sched/px_sched.h>

#include <SkyboltVis/Elevation/TilePlanetAltitudeProvider.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/CachedTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/MapboxElevationTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/SphericalMercatorToPlateCarreeTileSource.h>
#include <SkyboltVis/Renderable/Planet/Tile/TileSource/XyzTileSource.h>
#include <SkyboltVis/Image/SimpleImageFactory.h>
#include <SkyboltEngine/EngineCommandLineParser.h>
#include <SkyboltEngine/EngineSettings.h>
#include <SkyboltCommon/Json/JsonHelpers.h>
#include <SkyboltCommon/MapUtility.h>
#include <SkyboltCommon/ShaUtility.h>

#include <boost/program_options.hpp>

using namespace skybolt;
using namespace mapfeatures;
using namespace vis;

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
	try
	{
		// Default values
		int maxFeatureTileLod = 13;
		int maxHeightmapTileLod = 12;
		float minFeatureSizeFraction = 0.01;
		std::string outputDirectory = "Output";
		std::string tileCacheDirectory = "cache";
		std::string heightmapSourceDirectory = "SkyboltAssets/Seattle/Tiles/Earth/DEM";
		std::string heightmapDestinationDirectory = "SkyboltAssets/Assets/SeattleElevation/Tiles/Earth/Elevation";
		std::string inputPbfFilename = "washington-latest.osm.pbf";

		po::options_description desc;
		EngineCommandLineParser::addOptions(desc);
		desc.add_options()
			("inputPbfFile", po::value<std::string>(&inputPbfFilename)->default_value(inputPbfFilename), "input OSM PBF file")
			("maxFeatureTileLod", po::value<int>(&maxFeatureTileLod)->default_value(maxFeatureTileLod), "maximum feature tile level of detail")
			("maxHeightmapTileLod", po::value<int>(&maxHeightmapTileLod)->default_value(maxHeightmapTileLod), "maximum heightmap tile level of detail")
			("outputDirectory", po::value<std::string>(&outputDirectory)->default_value(outputDirectory), "output directory")
			("tileCacheDirectory", po::value<std::string>(&tileCacheDirectory)->default_value(tileCacheDirectory), "tile cache directory")
			("heightmapSourceDirectory", po::value<std::string>(&heightmapSourceDirectory)->default_value(heightmapSourceDirectory), "heightmap source directory")
			("heightmapDestinationDirectory", po::value<std::string>(&heightmapDestinationDirectory)->default_value(heightmapDestinationDirectory), "heightmap destination directory");

		auto params = EngineCommandLineParser::parse(argc, argv, desc);

		if (params.count("help"))
		{
			std::cout << desc << std::endl;
			return EXIT_SUCCESS;
		}

		nlohmann::json settings = readEngineSettings(params);
		auto tileApiKeys = readNameMap<std::string>(settings, "tileApiKeys");

#define USE_DEM
#ifdef USE_DEM
		XyzTileSourceConfig config;
		config.urlTemplate = heightmapSourceDirectory + "/{z}/{x}/{y}.png";
		config.elevationRerange = rerangeElevationFromUInt16WithElevationBounds(-32768, 32767);
		config.levelRange = {0, 10};
		config.imageFactory = std::make_shared<SimpleImageFactory>();

		auto tileSource = std::make_shared<XyzTileSource>(config);
		tileSource->validate();

#else // use mapbox
		auto mapboxKey = findOptional<std::string>(tileApiKeys, "mapbox");
		if (!mapboxKey)
		{
			throw std::runtime_error("Mapbox key not found in Skybolt settings file");
		}

		MapboxElevationTileSourceConfig config;
		config.urlTemplate = "http://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token={key}";
		config.apiKey = *mapboxKey;

		auto uncachedTileSource = std::make_shared<SphericalMercatorToPlateCarreeTileSource>(std::make_shared<MapboxElevationTileSource>(config));
		std::string tileSourceCacheDirectory = tileCacheDirectory  + "/" + calcSha1(config.urlTemplate);
		auto tileSource = std::make_shared<CachedTileSource>(uncachedTileSource, tileSourceCacheDirectory);
#endif
		BlockingTilePlanetAltitudeProvider altitudeProvider(tileSource, maxHeightmapTileLod);
		ReadPbfResult result = mapfeatures::readPbf(inputPbfFilename, altitudeProvider);
		{
			printf("Feature Conversion Stats:\n%s\n", mapfeatures::statsToString(result.features).c_str());

			mapfeatures::TreeCreatorParams treeCreatorParams;
			treeCreatorParams.minFeatureSizeFraction = minFeatureSizeFraction;
			treeCreatorParams.maxLodLevel = maxFeatureTileLod;
			mapfeatures::WorldFeatures worldFeatures = mapfeatures::createWorldFeatures(treeCreatorParams, result.features);

#ifdef PERFORM_HEIGHTMAP_LEVELING_UNDER_FEATURES
			double borderMeters = 100.0;
			std::vector<Feature*> airportFeatures;
			for (const auto& a : result.airports)
			{
				airportFeatures.push_back(a.second.get());
			}
			mapfeatures::levelHeightmapsUnderFeatures(heightmapSourceDirectory, heightmapDestinationDirectory, airportFeatures, borderMeters);

			for (const auto& airport : airportFeatures)
			{
				static_cast<Airport*>(airport)->altitude = mapfeatures::getAltitudeAtPosition(heightmapDestinationDirectory, airport->calcBounds().center());
			}
#endif
			mapfeatures::save(worldFeatures.tree, outputDirectory);
			mapfeatures::saveAirports(result.airports, outputDirectory + "/airports.apt");
		}
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception thrown: " << e.what() << std::endl;
	}
}