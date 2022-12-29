from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class SkyboltConan(ConanFile):
    name = "skybolt"
    version = "1.5.0"
    settings = "os", "compiler", "arch", "build_type"
    options = {
		"shared": [True, False],
		"enable_fft_ocean": [True, False],
		"shared_plugins": [True, False], # Build plugins as shared libraries
		"fixed_osg": [True, False] # Use patched OSG (TODO: Link to details)
	}
    default_options = {
        "shared": True,
        "enable_fft_ocean": True,
		"shared_plugins": True,
        "fixed_osg": True,
        # Force all dependencies to be shared libraries
#        "*:shared": True
    }
    generators = ["virtualrunenv"]
    exports_sources = "*"
    no_copy_source = True

    requires = [
		"boost/1.75.0",
		"catch2/2.13.8",
		"cpp-httplib/0.10.1",
		"earcut/2.2.3",
		"glm/0.9.9.8",
		"nlohmann_json/3.10.5",
		"zlib/1.2.12", # Indirect dependency. Specified to resolve version clash between boost and freetype.
	]

    def layout(self):
        cmake_layout(self)
        
    def requirements(self):
        if self.options.fixed_osg:
            self.requires("openscenegraph-mr/3.7.0@prograda/stable")
            self.osg_dependency_name = "openscenegraph-mr"
        else:
            self.requires("openscenegraph/3.6.5")
            self.osg_dependency_name = "openscenegraph"
            
        self.options[self.osg_dependency_name].with_curl = True # Required for loading terrain tiles from http sources

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["Skybolt_LinkStaticOpenSceneGraphPlugins"] = str(not self.options[self.osg_dependency_name].shared)
        tc.variables["SKYBOLT_PLUGINS_STATIC_BUILD"] = str(not self.options.shared_plugins)

        if self.options.enable_fft_ocean == True:
            tc.variables["BUILD_FFT_OCEAN_PLUGIN"] = "true"
		
        tc.generate()
        
        deps = CMakeDeps(self)
        deps.generate()
        
    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
		
    def package(self):
        cmake = CMake(self)
        cmake.install()
		
    def package_info(self):
        self.cpp_info.includedirs = ["include"]
        self.cpp_info.names["cmake_find_package"] = "Skybolt"
        self.cpp_info.libs = ["AircraftHud", "MapAttributesConverter", "SkyboltCommon", "SkyboltEngine", "SkyboltSim", "SkyboltVis"]
		
        if self.options.enable_fft_ocean and not self.options.shared_plugins:
            self.cpp_info.libs.append("FftOcean")