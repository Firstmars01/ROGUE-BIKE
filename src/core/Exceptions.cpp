// Module core.exceptions (implémentation) : code des classes d'exception.
// Interface : Exceptions.ixx
module;
// Import rapide de la bibliothèque standard
import std;

// Unité d'implémentation du module core.exceptions
module core.exceptions;

namespace core {

    // Mémorise le message d'erreur
    InvalidDimensionException::InvalidDimensionException(const std::string& message) : message{message} {}

    // Renvoie le message d'erreur
    const char* InvalidDimensionException::what() const noexcept {
        return message.c_str();
    }

}
