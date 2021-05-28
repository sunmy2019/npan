void inline npan_assert(bool condition)
{
    if (__builtin_expect(!condition, 0))
        throw;
}