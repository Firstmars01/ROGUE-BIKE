# Configuration CMake du projet Rogue Bike (basée sur le modèle du cours C++ Moderne esimed)
#
# Cibles générées :
#   - rogueCore : bibliothèque de logique métier, SANS dépendance graphique
#   - rogueUi   : bibliothèque d'affichage (projection isométrique, rendu raylib)
#   - RogueBike : exécutable du jeu (rogueCore + rogueUi + raylib)
#   - tests     : tests unitaires Catch2 (logique métier et mathématiques de projection)
cmake_minimum_required(VERSION 3.29)

# Nom du projet et de l'exécutable généré
project(RogueBike)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
# Forcer l'utilisation de C++ 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Tout doit être parfait : avertissements traités comme des erreurs (sauf dépendances externes)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
# Ne pas utiliser les extensions du compilateur
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY OFF)
endif()

# Pour "import std"
include(cmake/std.cmake)
include(cmake/compile_options.cmake)
# Dépendances externes téléchargées par FetchContent
include(cmake/raylib.cmake)
include(cmake/catch2.cmake)

# Répertoires des sources
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# --- Bibliothèque core : logique métier pure (grille, génération, combats, pathfinding) ---
# Découverte automatique des sources (CMake se relance à l'ajout ou la suppression d'un fichier)
file(GLOB CORE_SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/core/*.cpp")
file(GLOB CORE_MODULE_FILES CONFIGURE_DEPENDS "${SRC_DIR}/core/*.ixx")

add_library(rogueCore STATIC ${CORE_SRC_FILES})
target_sources(rogueCore
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        ${CORE_MODULE_FILES}
)
compile_options(rogueCore)

# --- Bibliothèque ui : affichage, seule partie (avec main.cpp) autorisée à utiliser raylib ---
file(GLOB UI_SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/ui/*.cpp")
file(GLOB UI_MODULE_FILES CONFIGURE_DEPENDS "${SRC_DIR}/ui/*.ixx")

add_library(rogueUi STATIC ${UI_SRC_FILES})
target_sources(rogueUi
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        ${UI_MODULE_FILES}
)
compile_options(rogueUi)
target_link_libraries(rogueUi rogueCore raylib)

# --- Exécutable principal ---
add_executable(${PROJECT_NAME} ${SRC_DIR}/main.cpp)
compile_options(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} rogueCore rogueUi raylib)

# Copie des ressources (sprites) à côté de l'exécutable à chaque build :
# le jeu les charge depuis <dossier de l'exécutable>/assets
add_custom_target(copyAssets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets
        COMMENT "Copie des ressources du jeu"
)
add_dependencies(${PROJECT_NAME} copyAssets)

# --- Tests unitaires : logique métier + fonctions pures de rogueUi (ui.isoMath, sans appel raylib) ---
file(GLOB TEST_FILES CONFIGURE_DEPENDS "${TESTS_DIR}/*.cpp")
add_executable(tests ${TEST_FILES})
target_link_libraries(tests PRIVATE rogueCore rogueUi Catch2::Catch2WithMain std-cxx-modules)
include(CTest)
include(Catch)
catch_discover_tests(tests)
