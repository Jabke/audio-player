//
// Created by galkin on 15.08.24.
//

#ifndef AUDIO_PALYER_SRC_PULSE_CLIENT_EXCEPTIONS_H_
#define AUDIO_PALYER_SRC_PULSE_CLIENT_EXCEPTIONS_H_

#include <stdexcept>
#include <fmt/core.h>

namespace pulse_client {
namespace pulse_exceptions {

//---BaseException------------------------------------------------------------------------------------------------------

class BaseException : public std::runtime_error {
 public:
  explicit BaseException(std::string message) : std::runtime_error(message) {};
};

//---ExceptionWhenCreatingPAFacade------------------------------------------------------------------------------------------------------

class ExceptionWhenCreatingPAFacade : public BaseException {
 public:
  explicit ExceptionWhenCreatingPAFacade(std::string name_of_entity) :
    BaseException(fmt::format("Error of creating pa entity:{}", name_of_entity)) {};
};

};  // pulse_exceptions
};  // pulse_client
#endif //AUDIO_PALYER_SRC_PULSE_CLIENT_EXCEPTIONS_H_
