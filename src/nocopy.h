#ifndef NOCOPY_H
#define NOCOPY_H

class non_copyable
{
protected:
    non_copyable() = default;
    ~non_copyable() = default;

    non_copyable(non_copyable const &) = delete;
    void operator=(non_copyable const &x) = delete;
};

#endif
