#include <raylib.h>
#include <raymath.h>

#define DRN_IMPL
#include "drn.h"

int main(int argc, char *argv[])
{

    SliceStack s = {0};
    Slice s0 = {.base = "hello world", .len = sizeof("hello world")};

    SliceStack_Push(&s, s0);

    Slice sp = SliceStack_Pop(&s);

    if (sp.base == 0)
    {
        printf("sp.base==0\n");
        abort(); // error bad code
    }

    printf("%.*s\n", sp.len, sp.base);

    return 0;
}