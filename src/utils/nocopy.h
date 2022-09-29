#ifndef _NOCOPY_H_
#define _NOCOPY_H_

class Nocopy
{
public:
    Nocopy() {}
    ~Nocopy() {}

    Nocopy(const Nocopy &) = delete;
    Nocopy &operator=(const Nocopy &) = delete;
};

#endif // _NOCOPY_H_