#pragma once
#include "patch_solution_since.hpp"
#include "image_interpreter.hpp"
#include <any>
#include <string>
#include <map>

namespace nkg {

    template<>
    class patch_solution_since<16, 0, 7, 0> final : public patch_solution {
    private:
        static inline std::string_view official_encoded_key = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw1dqF3SkCaAAmMzs889IqdW9M2dIdh3jG9yPcmLnmJiGpBF4E9VHSMGe8oPAy2kJDmdNt4BcEygvssEfginva5t5jm352UAoDosUJkTXGQhpAWMF4fBmBpO3EedG62rOsqMBgmSdAyxCSPBRJIOFR0QgZFbRnU0frj34fiVmgYiLuZSAmIbs8ZxiHPdp1oD4tUpvsFci4QJtYNjNnGU2WPH6rvChGl1IRKrxMtqLielsvajUjyrgOC6NmymYMvZNER3htFEtL1eQbCyTfDmtYyQ1Wt4Ot12lxf0wVIR5mcGN7XCXJRHOFHSf1gzXWabRSvmt1nrl7sW6cjxljuuQawIDAQAB";
        
        image_interpreter& m_libcc_interpreter;
        image_interpreter::va_t m_va_CSRegistrationInfoFetcher_WIN_vtable;
        image_interpreter::va_t m_va_CSRegistrationInfoFetcher_WIN_GenerateRegistrationKey;
        image_interpreter::va_t m_va_iat_entry_malloc;

        std::string _build_encoded_key(const rsa_cipher& cipher);

    public:
        patch_solution_since(image_interpreter& libcc_interpreter);

        [[nodiscard]]
        virtual bool find_patch() override;

        [[nodiscard]]
        virtual bool check_rsa_privkey(const rsa_cipher& cipher) override;

        virtual void make_patch(const rsa_cipher& cipher) override;

    };

}
