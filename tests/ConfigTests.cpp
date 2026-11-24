// Tests unitaires du module core.config (vérifie aussi que Catch2 + modules fonctionnent).
#include <catch2/catch_all.hpp>

import core.config;

// Vérifie que la configuration de la fenêtre est cohérente
TEST_CASE("Configuration de la fenetre valide", "[config]") {
    // Vérifications faites à la compilation (constexpr)
    STATIC_REQUIRE(core::windowWidth > 0);
    STATIC_REQUIRE(core::windowHeight > 0);
    STATIC_REQUIRE(core::targetFps > 0);

    // Le titre ne doit pas être vide
    REQUIRE_FALSE(core::windowTitle.empty());
    REQUIRE(core::windowTitle == "Rogue Bike");
}
