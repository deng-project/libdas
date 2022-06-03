// libdas: DENG asset handling management library
// licence: Apache, see LICENCE file
// file: DasValidator.cpp - DAS file validation class implementation
// author: Karl-Mihkel Ott


#define DAS_VALIDATOR_CPP
#include <DasValidator.h>

namespace Libdas {
    
    DasValidator::DasValidator(DasParser &_parser, bool _validate) : m_parser(_parser) {
        if(_validate) Validate();
    }


    DasValidator::DasValidator(DasValidator &&_val) noexcept :
        m_parser(_val.m_parser),
        m_warning_stack(_val.m_warning_stack),
        m_error_stack(_val.m_error_stack) {}
}
