# Télécharger et compiler raylib depuis GitHub (même compilateur et même runtime que le jeu)
include(FetchContent)

# Pas besoin des exemples de raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# CMake 4.x refuse les vieux cmake_minimum_required des dépendances (raylib / GLFW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# raylib est une dépendance externe : ses avertissements ne doivent pas bloquer notre build
set(rogueWarningAsError ${CMAKE_COMPILE_WARNING_AS_ERROR})
set(CMAKE_COMPILE_WARNING_AS_ERROR OFF)

FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.5
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(raylib)

# Rétablir le réglage pour nos propres cibles
set(CMAKE_COMPILE_WARNING_AS_ERROR ${rogueWarningAsError})
