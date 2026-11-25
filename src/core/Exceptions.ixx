// Module core.exceptions (interface) : classes d'exception propres au jeu.
// Implémentation : Exceptions.cpp
module;
// Import rapide de la bibliothèque standard
import std;

// Nommage et exportation de ce module
export module core.exceptions;

export namespace core {

    // Exception levée lorsqu'une dimension (grille, carte, salle) est invalide
    class InvalidDimensionException : public std::exception {
    public:
        // Crée l'exception. message : description de l'erreur
        explicit InvalidDimensionException(const std::string& message);

        // Renvoie la description de l'erreur
        const char* what() const noexcept override;

    private:
        std::string message;
    };

}
