#pragma once
#include "rsa_cipher.hpp"

namespace nkg {

    class patch_solution {
    public:
        [[nodiscard]]
        virtual bool find_patch() = 0;

        [[nodiscard]]
        virtual bool check_rsa_privkey(const rsa_cipher& cipher) = 0;

        virtual void make_patch(const rsa_cipher& cipher) = 0;

        virtual ~patch_solution() = default;
    };

}
