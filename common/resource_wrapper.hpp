#pragma once
#include <type_traits>
#include <utility>

namespace nkg {

    template<typename resource_traits_t, typename releaser_t = void>
    class resource_wrapper {
    public:
        using handle_t = typename resource_traits_t::handle_t; 
        static_assert(std::is_trivial_v<handle_t> && std::is_standard_layout_v<handle_t>, "`resource_wrapper` requires a handle with POD type.");

    private:
        handle_t m_handle;
        releaser_t m_releaser;

    public:
        template<typename releaser_arg_t>
        resource_wrapper(releaser_arg_t&& releaser) noexcept :
            m_handle(resource_traits_t::invalid_value),
            m_releaser(std::forward<releaser_arg_t>(releaser)) {}

        template<typename releaser_arg_t>
        resource_wrapper(const handle_t& handle, releaser_arg_t&& releaser) noexcept :
            m_handle(handle),
            m_releaser(std::forward<releaser_arg_t>(releaser)) {}

        template<typename releaser_arg_t>
        resource_wrapper(resource_traits_t, releaser_arg_t&& releaser) noexcept :
            m_handle(resource_traits_t::invalid_value),
            m_releaser(std::forward<releaser_arg_t>(releaser)) {}

        template<typename releaser_arg_t>
        resource_wrapper(resource_traits_t, const handle_t& handle, releaser_arg_t&& releaser) noexcept :
            m_handle(handle),
            m_releaser(std::forward<releaser_t>(releaser)) {}

        // 
        // `resource_wrapper` does not allow copy-construct
        // 
        resource_wrapper(const resource_wrapper& other) = delete;

        //
        // `resource_wrapper` allows move-construct.
        //
        resource_wrapper(resource_wrapper&& other) noexcept :
            m_handle(other.m_handle),
            m_releaser(std::move(other.m_releaser)) 
        {
            other.m_handle = resource_traits_t::invalid_value;
        }

        //
        // `resource_wrapper` does not allow to copy.
        //
        resource_wrapper& operator=(const resource_wrapper& other) = delete;

        //
        // `resource_wrapper` allows to move.
        //
        resource_wrapper& operator=(resource_wrapper&& other) noexcept {
            if (this != std::addressof(other)) {
                m_handle = other.m_handle;
                m_releaser = std::move(other.m_releaser);
                other.m_handle = resource_traits_t::invalid_value;
            }
            return *this;
        }

        template<typename = std::enable_if_t<std::is_pointer_v<handle_t>>>
        [[nodiscard]]
        handle_t operator->() const noexcept {
            return m_handle;
        }

        template<typename as_t, typename = std::enable_if_t<std::is_pointer_v<handle_t>>>
        [[nodiscard]]
        as_t as() const noexcept {
            return reinterpret_cast<as_t>(m_handle);
        }

        [[nodiscard]]
        bool is_valid() const noexcept {
            return resource_traits_t::is_valid(m_handle);
        }

        [[nodiscard]]
        const handle_t& get() const noexcept {
            return m_handle;
        }

        template<typename as_t = handle_t>
        [[nodiscard]]
        as_t* unsafe_addressof() noexcept {
            return reinterpret_cast<as_t*>(std::addressof(m_handle));
        }

        void set(const handle_t& handle) {
            if (is_valid()) {
                m_releaser(m_handle);
            }
            m_handle = handle;
        }

        void discard() noexcept {
            m_handle = resource_traits_t::invalid_value;
        }

        [[nodiscard]]
        handle_t transfer() noexcept {
            handle_t t = m_handle;
            m_handle = resource_traits_t::invalid_value;
            return t;
        }

        void release() {
            if (is_valid()) {
                m_releaser(m_handle);
                m_handle = resource_traits_t::invalid_value;
            }
        }

        ~resource_wrapper() {
            release();
        }
    };

    template<typename resource_traits_t>
    class resource_wrapper<resource_traits_t, void> {
    public:
        using handle_t = typename resource_traits_t::handle_t;
        static_assert(std::is_trivial_v<handle_t>&& std::is_standard_layout_v<handle_t>, "`resource_wrapper` requires a handle with POD type.");

    private:
        handle_t m_handle;

    public:
        resource_wrapper() noexcept :
            m_handle(resource_traits_t::invalid_value) {}

        resource_wrapper(const handle_t& handle) noexcept :
            m_handle(handle) {}

        resource_wrapper(resource_traits_t) noexcept :
            m_handle(resource_traits_t::invalid_value) {}

        resource_wrapper(resource_traits_t, const handle_t& handle) noexcept :
            m_handle(handle) {}

        resource_wrapper(const resource_wrapper& other) = delete;

        resource_wrapper(resource_wrapper&& other) noexcept :
            m_handle(other.m_handle) 
        {
            other.m_handle = resource_traits_t::invalid_value;
        }

        resource_wrapper& operator=(const resource_wrapper& other) = delete;

        resource_wrapper& operator=(resource_wrapper&& other) noexcept {
            if (this != std::addressof(other)) {
                m_handle = other.m_handle;
                other.m_handle = resource_traits_t::invalid_value;
            }
            return *this;
        }

        template<typename = std::enable_if_t<std::is_pointer_v<handle_t>>>
        [[nodiscard]]
        handle_t operator->() const noexcept {
            return m_handle;
        }

        template<typename as_t, typename = std::enable_if_t<std::is_pointer_v<handle_t>>>
        [[nodiscard]]
        as_t as() const noexcept {
            return reinterpret_cast<as_t>(m_handle);
        }

        [[nodiscard]]
        bool is_valid() const noexcept {
            return resource_traits_t::is_valid(m_handle);
        }

        [[nodiscard]]
        const handle_t& get() const noexcept {
            return m_handle;
        }

        template<typename as_t = handle_t>
        [[nodiscard]]
        as_t* unsafe_addressof() noexcept {
            return reinterpret_cast<as_t*>(std::addressof(m_handle));
        }

        void set(const handle_t& handle) {
            if (is_valid()) {
                resource_traits_t::release(m_handle);
            }
            m_handle = handle;
        }

        void discard() noexcept {
            m_handle = resource_traits_t::invalid_value;
        }

        [[nodiscard]]
        handle_t transfer() noexcept {
            handle_t t = m_handle;
            m_handle = resource_traits_t::invalid_value;
            return t;
        }

        void release() {
            if (is_valid()) {
                resource_traits_t::release(m_handle);
                m_handle = resource_traits_t::invalid_value;
            }
        }

        ~resource_wrapper() {
            release();
        }
    };

    template<typename resource_traits_t>
    resource_wrapper(resource_traits_t) ->
        resource_wrapper<resource_traits_t, void>;

    template<typename resource_traits_t, typename arg_t>
    resource_wrapper(resource_traits_t, arg_t&&) ->
        resource_wrapper<
            resource_traits_t,
            std::conditional_t<
                std::is_same_v<std::remove_cv_t<std::remove_reference_t<arg_t>>, typename resource_traits_t::handle_t> == false,
                std::remove_reference_t<arg_t>,
                void
            >
        >;

    template<typename resource_traits_t, typename releaser_t, typename handle_t = typename resource_traits_t::handle_t>
    resource_wrapper(resource_traits_t, const handle_t&, releaser_t&&) ->
        resource_wrapper<resource_traits_t, std::remove_reference_t<releaser_t>>;

}
