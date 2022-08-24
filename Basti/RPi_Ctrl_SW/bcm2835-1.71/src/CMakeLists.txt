
set(BCM2835_SOURCES
        ./bcm2835.c)

add_library(bcm2835 ${BCM2835_SOURCES})

# Name of package
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE="bcm2835")
# Define to the address where bug reports for this package should be sent.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_BUGREPORT="mikem@airspayce.com")
# Define to the full name of this package.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_NAME="bcm2835")
# Define to the full name and version of this package.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_STRING="bcm2835 1.71")
# Define to the one symbol short name of this package.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_TARNAME="bcm2835")
# Define to the home page for this package.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_URL="")
# Define to the version of this package.
target_compile_definitions(bcm2835 PRIVATE -DPACKAGE_VERSION="${CMAKE_PROJECT_VERSION}")
# Version number of package
target_compile_definitions(bcm2835 PRIVATE -DVERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(bcm2835 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
