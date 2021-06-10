#include <cassert> // for __assert_fail
#define NPAN_COLOR_OUTPUT

#ifdef NPAN_COLOR_OUTPUT
#include <fmt/color.h>
#endif

// #include <fmt/compile.h>
// Note: using compile time format string would cause code bloat, we decide not to use here.

#ifdef NDEBUG
#define NPAN_ASSERT(cond, args...) void(0)
#define NPAN_WARNING(cond, args...) void(0)
#else
#define NPAN_ASSERT(cond, args...) \
    (static_cast<bool>(cond) ? void(0) : (npan::detail::error(args), throw))
#define NPAN_WARNING(cond, args...) \
    (static_cast<bool>(cond) ? void(0) : npan::detail::error(args))
#endif

namespace npan
{
    namespace detail
    {
        template <typename... Args>
        void inline print(Args &&...args)
        {
            fmt::print(std::forward<Args>(args)...);
        }
        template <typename... Args>
        void inline print(FILE *file, Args &&...args)
        {
            fmt::print(file, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void inline warning(Args &&...args)
        {
#ifdef NPAN_COLOR_OUTPUT
            fmt::print(fg(fmt::color::red), std::forward<Args>(args)...);
#else
            fmt::print(std::forward<Args>(args)...);
#endif
        }

        template <typename... Args>
        void inline warning(FILE *file, Args &&...args)
        {
#ifdef NPAN_COLOR_OUTPUT
            fmt::print(file, fg(fmt::color::red), std::forward<Args>(args)...);
#else
            fmt::print(file, std::forward<Args>(args)...);
#endif
        }

        template <typename... Args>
        void inline error(Args &&...args)
        {
#ifdef NPAN_COLOR_OUTPUT
            fmt::print(stderr, fg(fmt::color::red), std::forward<Args>(args)...);
#else
            fmt::print(stderr, std::forward<Args>(args)...);
#endif
        }

    }
};
