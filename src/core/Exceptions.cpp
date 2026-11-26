// Module core.exceptions (implémentation).
module;
import std;

module core.exceptions;

namespace core {

    GameException::GameException(const std::string& message) : message{message} {}

    const char* GameException::what() const noexcept {
        return message.c_str();
    }

}
