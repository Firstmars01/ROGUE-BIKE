// Module ui.renderer (implémentation) : dessin des tuiles avec raylib.
// Interface : Renderer.ixx
module;
// raylib est incluse uniquement ici (fragment de module global) : elle reste invisible
// pour les modules qui importent ui.renderer
#include <raylib.h>
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module ui.renderer
module ui.renderer;

namespace ui {

    // Fonctions et constantes internes à ce fichier (non exportées)
    namespace {

        // Couleurs des tuiles
        constexpr Color floorColor{92, 84, 78, 255};
        constexpr Color floorOutlineColor{60, 54, 50, 255};
        constexpr Color wallTopColor{150, 140, 128, 255};
        constexpr Color wallLeftColor{105, 96, 88, 255};
        constexpr Color wallRightColor{78, 70, 64, 255};
        constexpr Color stairsColor{214, 170, 60, 255};
        constexpr Color stairsInnerColor{120, 86, 30, 255};

        // Les 4 sommets d'un losange isométrique
        struct Diamond {
            Vector2 top{};
            Vector2 left{};
            Vector2 bottom{};
            Vector2 right{};
        };

        // Calcule les sommets du losange de centre center, réduit d'un facteur scale (1 = taille d'une tuile)
        Diamond MakeDiamond(const ScreenPos center, const float scale) {
            const auto halfWidth = halfTileWidth * scale;
            const auto halfHeight = halfTileHeight * scale;
            return Diamond{
                Vector2{center.x, center.y - halfHeight},
                Vector2{center.x - halfWidth, center.y},
                Vector2{center.x, center.y + halfHeight},
                Vector2{center.x + halfWidth, center.y}
            };
        }

        // Décale un losange verticalement de offsetY pixels (négatif = vers le haut)
        Diamond Raise(const Diamond& diamond, const float offsetY) {
            const auto shift = [offsetY](const Vector2 point) { return Vector2{point.x, point.y + offsetY}; };
            return Diamond{shift(diamond.top), shift(diamond.left), shift(diamond.bottom), shift(diamond.right)};
        }

        // Dessine un triangle plein quel que soit l'ordre des sommets :
        // raylib n'affiche que les triangles donnés dans le sens inverse des aiguilles d'une montre.
        void FillTriangle(const Vector2 a, const Vector2 b, const Vector2 c, const Color color) {
            const auto cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
            if (cross < 0.0f) {
                DrawTriangle(a, b, c, color);
            } else if (cross > 0.0f) {
                DrawTriangle(a, c, b, color);
            }
            // cross == 0 : triangle plat, rien à dessiner
        }

        // Dessine un quadrilatère convexe plein (sommets donnés dans l'ordre du contour)
        void FillQuad(const Vector2 a, const Vector2 b, const Vector2 c, const Vector2 d, const Color color) {
            FillTriangle(a, b, c, color);
            FillTriangle(a, c, d, color);
        }

        // Dessine un losange plein
        void FillDiamond(const Diamond& diamond, const Color color) {
            FillQuad(diamond.top, diamond.left, diamond.bottom, diamond.right, color);
        }

        // Dessine le contour d'un losange
        void OutlineDiamond(const Diamond& diamond, const Color color) {
            DrawLineV(diamond.top, diamond.left, color);
            DrawLineV(diamond.left, diamond.bottom, color);
            DrawLineV(diamond.bottom, diamond.right, color);
            DrawLineV(diamond.right, diamond.top, color);
        }

        // Dessine une case de sol : losange plein et contour
        void DrawFloor(const ScreenPos center) {
            const auto diamond = MakeDiamond(center, 1.0f);
            FillDiamond(diamond, floorColor);
            OutlineDiamond(diamond, floorOutlineColor);
        }

        // Dessine l'escalier : losange doré avec un losange intérieur plus sombre
        void DrawStairs(const ScreenPos center) {
            const auto diamond = MakeDiamond(center, 1.0f);
            FillDiamond(diamond, stairsColor);
            FillDiamond(MakeDiamond(center, 0.5f), stairsInnerColor);
            OutlineDiamond(diamond, floorOutlineColor);
        }

        // Dessine un mur en relief : face gauche, face droite, puis dessus surélevé de wallHeight
        void DrawWall(const ScreenPos center) {
            const auto base = MakeDiamond(center, 1.0f);
            const auto top = Raise(base, -wallHeight);
            FillQuad(top.left, top.bottom, base.bottom, base.left, wallLeftColor);
            FillQuad(top.bottom, top.right, base.right, base.bottom, wallRightColor);
            FillDiamond(top, wallTopColor);
            OutlineDiamond(top, wallRightColor);
        }

    }

    // Calcule une fois l'origine qui centre la carte dans la fenêtre
    Renderer::Renderer(const int mapWidth, const int mapHeight, const int screenWidth, const int screenHeight)
        : origin{CenteredOrigin(mapWidth, mapHeight, screenWidth, screenHeight)} {}

    // Origine de la projection
    ScreenPos Renderer::GetOrigin() const noexcept {
        return origin;
    }

    // Parcours par diagonales : sum = x + y croissant (ordre du peintre)
    void Renderer::DrawGrid(const core::Grid& grid) const {
        const auto width = grid.GetWidth();
        const auto height = grid.GetHeight();
        for (auto sum = 0; sum <= width + height - 2; ++sum) {
            // Sur une diagonale, x varie de façon à garder y = sum - x dans la grille
            const auto firstX = std::max(0, sum - (height - 1));
            const auto lastX = std::min(sum, width - 1);
            for (auto x = firstX; x <= lastX; ++x) {
                const auto pos = core::GridPos{x, sum - x};
                if (const auto tile = grid.GetTile(pos)) {
                    DrawTile(pos, *tile);
                }
            }
        }
    }

    // Choisit le dessin selon le type de la case
    void Renderer::DrawTile(const core::GridPos pos, const core::TileType type) const {
        const auto center = GridToScreen(pos, origin);
        switch (type) {
            case core::TileType::Floor:
                DrawFloor(center);
                break;
            case core::TileType::Wall:
                DrawWall(center);
                break;
            case core::TileType::Stairs:
                DrawStairs(center);
                break;
            case core::TileType::Empty:
                // Hors du donjon : rien à dessiner
                break;
        }
    }

}
