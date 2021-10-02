#pragma once
#include <map>
#include <unistd.h>
/* npan output routine */

/* internal API of printing, warning and assertion */

#define NPAN_COLOR_OUTPUT

#ifdef NPAN_COLOR_OUTPUT
#include <fmt/color.h>
#else
#include <fmt/format.h>
#endif

// #include <fmt/compile.h>
// Note: using compile time format string would cause potential code bloat, we decide not to use here.

#define NPAN_ASSERT(cond, args...) \
    (static_cast<bool>(cond) ? void(0) : (npan::detail::error(args), throw))
#define NPAN_WARNING(cond, args...) \
    (static_cast<bool>(cond) ? void(0) : npan::detail::error(args))

namespace npan
{
    namespace detail
    {
        // records device type when initializing
        struct Device_type
        {
            bool is_tty;
            operator bool()
            {
                return is_tty;
            }
            Device_type(int fd) : is_tty(isatty(fd)) {}
        };

        inline Device_type stdout_type{fileno(stdout)}, stderr_type{fileno(stderr)};

        template <typename... Args>
        void print(const fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void print(FILE *file, fmt::format_string<Args...> fmt, Args &&...args)
        {
            fmt::print(file, fmt, std::forward<Args>(args)...);
        }

#ifdef NPAN_COLOR_OUTPUT

        template <typename... Args>
        void warning(fmt::format_string<Args...> fmt, Args &&...args)
        {
            if (stdout_type)
                fmt::print(fg(fmt::color::red), fmt::string_view(fmt), std::forward<Args>(args)...);
            else
                fmt::print(fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void warning(FILE *file, fmt::format_string<Args...> fmt, Args &&...args)
        { // since we (should) rarely use warning, we can bear this cost
            Device_type dv(file->_fileno);
            if (dv)
                fmt::print(file, fg(fmt::color::red), fmt::string_view(fmt), std::forward<Args>(args)...);
            else
                fmt::print(file, fmt, std::forward<Args>(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args &&...args)
        {
            if (stderr_type)
                fmt::print(stderr, fg(fmt::color::red), fmt::string_view(fmt), std::forward<Args>(args)...);
            else
                fmt::print(stderr, fmt, std::forward<Args>(args)...);
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
