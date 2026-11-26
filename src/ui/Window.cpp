// Module ui.window (implémentation).
module;
#include <raylib.h>
import std;

module ui.window;

namespace ui {

    // Canevas virtuel : texture de rendu en RAII et taille fixe
    struct Window::Canvas {
        RenderTexture2D target;
        float width;
        float height;

        // Crée la texture de rendu. Lève std::runtime_error en cas d'échec.
        Canvas(const int width, const int height)
            : target{LoadRenderTexture(width, height)},
              width{static_cast<float>(width)},
              height{static_cast<float>(height)} {
            if (target.id == 0) {
                throw std::runtime_error{"Impossible de creer le canevas"};
            }
        }

        // Libère la texture de rendu
        ~Canvas() {
            UnloadRenderTexture(target);
        }

        Canvas(const Canvas&) = delete;
        Canvas& operator=(const Canvas&) = delete;

        // Renvoie l'échelle d'affichage du canevas dans la fenêtre (proportions conservées)
        float Scale() const {
            return std::min(static_cast<float>(GetScreenWidth()) / width,
                            static_cast<float>(GetScreenHeight()) / height);
        }

        // Renvoie le décalage (bandes noires) du canevas centré dans la fenêtre
        Vector2 Offset() const {
            const auto scale = Scale();
            return Vector2{
                (static_cast<float>(GetScreenWidth()) - width * scale) / 2.0f,
                (static_cast<float>(GetScreenHeight()) - height * scale) / 2.0f
            };
        }
    };

    Window::Window(const int width, const int height, const std::string_view title, const int targetFps) {
        // std::string garantit un titre terminé par '\0'
        const auto titleText = std::string{title};
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(width, height, titleText.c_str());
        if (!IsWindowReady()) {
            throw std::runtime_error{"Impossible de creer la fenetre"};
        }
        // Plein écran sans bordure : aucune barre de titre ne réduit la zone d'affichage
        ToggleBorderlessWindowed();
        SetTargetFPS(targetFps);
        canvas = std::make_unique<Canvas>(width, height);
    }

    Window::~Window() {
        // Le canevas (texture) doit être libéré avant la fermeture du contexte graphique
        canvas.reset();
        CloseWindow();
    }

    bool Window::ShouldClose() const {
        return WindowShouldClose();
    }

    void Window::BeginFrame() const {
        BeginTextureMode(canvas->target);
    }

    void Window::EndFrame() const {
        EndTextureMode();
        BeginDrawing();
        ClearBackground(BLACK);
        const auto scale = canvas->Scale();
        const auto offset = canvas->Offset();
        // Hauteur négative : les textures de rendu OpenGL sont stockées à l'envers
        const auto source = Rectangle{0.0f, 0.0f, canvas->width, -canvas->height};
        const auto destination = Rectangle{offset.x, offset.y, canvas->width * scale, canvas->height * scale};
        DrawTexturePro(canvas->target.texture, source, destination, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        EndDrawing();
    }

    ScreenPos Window::MousePosition() const {
        // Transformation inverse de l'affichage : retrait du décalage puis de l'échelle
        const auto mouse = GetMousePosition();
        const auto scale = canvas->Scale();
        const auto offset = canvas->Offset();
        return ScreenPos{(mouse.x - offset.x) / scale, (mouse.y - offset.y) / scale};
    }

}
