// Module core.exceptions (interface) : exception du jeu.
// Implémentation : Exceptions.cpp
module;
import std;

export module core.exceptions;

export namespace core {

    // Exception levée quand une valeur est invalide (dimension, statistique, génération)
    class GameException : public std::exception {
    public:
        // message : description de l'erreur
        explicit GameException(const std::string& message);

        // Renvoie la description de l'erreur
        const char* what() const noexcept override;

    private:
        std::string message;
    };

}
