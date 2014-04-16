# This file will be configured to contain variables for CPack. These variables
# should be set in the CMake list file of the project before CPack module is
# included. The list of available CPACK_xxx variables and their associated
# documentation may be obtained using
#  cpack --help-variable-list
#
# Some variables are common to all generators (e.g. CPACK_PACKAGE_NAME)
# and some are specific to a generator
# (e.g. CPACK_NSIS_EXTRA_INSTALL_COMMANDS). The generator specific variables
# usually begin with CPACK_<GENNAME>_xxxx.


SET(CPACK_BINARY_BUNDLE "")
SET(CPACK_BINARY_CYGWIN "")
SET(CPACK_BINARY_DEB "OFF")
SET(CPACK_BINARY_DRAGNDROP "")
SET(CPACK_BINARY_NSIS "OFF")
SET(CPACK_BINARY_OSXX11 "")
SET(CPACK_BINARY_PACKAGEMAKER "")
SET(CPACK_BINARY_RPM "OFF")
SET(CPACK_BINARY_STGZ "ON")
SET(CPACK_BINARY_TBZ2 "OFF")
SET(CPACK_BINARY_TGZ "ON")
SET(CPACK_BINARY_TZ "ON")
SET(CPACK_BINARY_WIX "")
SET(CPACK_BINARY_ZIP "")
SET(CPACK_CMAKE_GENERATOR "Ninja")
SET(CPACK_COMPONENTS_ALL "development;library")
SET(CPACK_COMPONENT_UNSPECIFIED_HIDDEN "TRUE")
SET(CPACK_COMPONENT_UNSPECIFIED_REQUIRED "TRUE")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "build-essential, freeglut3-dev, libfreeimage-dev, libgl1-mesa-dev, libopenal-dev, libpango1.0-dev, libsdl-mixer1.2-dev, libsdl-ttf2.0-dev, libsndfile-dev, libxinerama-dev")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Oliver Schneider <mail@oli-obk.de>")
SET(CPACK_GENERATOR "STGZ;TGZ;TZ")
SET(CPACK_INSTALL_CMAKE_PROJECTS "/home/vileda/Projects/cpp/golGL/build;golGL;ALL;/")
SET(CPACK_INSTALL_PREFIX "/usr/local")
SET(CPACK_MODULE_PATH "")
SET(CPACK_NSIS_DISPLAY_NAME "Gosu0.7.50")
SET(CPACK_NSIS_INSTALLER_ICON_CODE "")
SET(CPACK_NSIS_INSTALLER_MUI_ICON_CODE "")
SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
SET(CPACK_NSIS_PACKAGE_NAME "Gosu0.7.50")
SET(CPACK_OUTPUT_CONFIG_FILE "/home/vileda/Projects/cpp/golGL/build/CPackConfig.cmake")
SET(CPACK_PACKAGE_CONTACT "Oliver Schneider <mail@oli-obk.de>")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "/home/vileda/Projects/cpp/golGL/external/gosu/cmake/../README.txt")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "a 2D game development library")
SET(CPACK_PACKAGE_FILE_NAME "libgosu-dev-0.7.50-Linux")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Gosu0.7.50")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Gosu0.7.50")
SET(CPACK_PACKAGE_NAME "libgosu-dev")
SET(CPACK_PACKAGE_RELOCATABLE "true")
SET(CPACK_PACKAGE_VENDOR "unknown")
SET(CPACK_PACKAGE_VERSION "0.7.50")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "7")
SET(CPACK_PACKAGE_VERSION_PATCH "50")
SET(CPACK_RESOURCE_FILE_LICENSE "/home/vileda/Projects/cpp/golGL/external/gosu/cmake/../COPYING")
SET(CPACK_RESOURCE_FILE_README "/usr/share/cmake-2.8/Templates/CPack.GenericDescription.txt")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake-2.8/Templates/CPack.GenericWelcome.txt")
SET(CPACK_SET_DESTDIR "OFF")
SET(CPACK_SOURCE_CYGWIN "")
SET(CPACK_SOURCE_GENERATOR "TGZ;TBZ2;TZ")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "/home/vileda/Projects/cpp/golGL/build/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_TBZ2 "ON")
SET(CPACK_SOURCE_TGZ "ON")
SET(CPACK_SOURCE_TZ "ON")
SET(CPACK_SOURCE_ZIP "OFF")
SET(CPACK_SYSTEM_NAME "Linux")
SET(CPACK_TOPLEVEL_TAG "Linux")
SET(CPACK_WIX_SIZEOF_VOID_P "8")

# Configuration for component "library"

SET(CPACK_COMPONENTS_ALL development library)
set(CPACK_COMPONENT_LIBRARY_DISPLAY_NAME "Library")
set(CPACK_COMPONENT_LIBRARY_DESCRIPTION "The runtime libraries")
set(CPACK_COMPONENT_LIBRARY_REQUIRED TRUE)

# Configuration for component "development"

SET(CPACK_COMPONENTS_ALL development library)
set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "Development")
set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Files required for developing with Gosu")
set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS library)

# Configuration for component group "bindings"
set(CPACK_COMPONENT_GROUP_BINDINGS_DISPLAY_NAME "Bindings")
set(CPACK_COMPONENT_GROUP_BINDINGS_DESCRIPTION "Language bindings")
