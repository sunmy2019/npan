#pragma once

/* npan output routine */

/* internal API of printing, warning and assertion */

#define NPAN_COLOR_OUTPUT

#ifdef NPAN_COLOR_OUTPUT
#include <fmt/color.h>
#endif

// #include <fmt/compile.h>
// Note: using compile time format string would cause potential code bloat, we decide not to use here.

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
        void print(fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void print(FILE *file, fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(file, fmt, std::forward<Args>(args)...);
        }

#ifdef NPAN_COLOR_OUTPUT

        template <typename S, typename... Args>
        void warning(const S &fmt, Args &&...args)
        {
            fmt::print(fg(fmt::color::red), fmt, std::forward<Args>(args)...);
        }

        template <typename S, typename... Args>
        void warning(FILE *file, const S &fmt, Args &&...args)
        {
            fmt::print(file, fg(fmt::color::red), fmt, std::forward<Args>(args)...);
        }

        template <typename S, typename... Args>
        void error(const S &fmt, Args &&...args)
        {
            fmt::print(stderr, fg(fmt::color::red), fmt, std::forward<Args>(args)...);
        }

#else
        template <typename... Args>
        void warning(fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warning(FILE *file, fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(file, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(stderr, fmt, std::forward<Args>(args)...);
        }
#endif
    }
};
