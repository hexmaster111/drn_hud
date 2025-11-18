#include <raylib.h>
#include <raymath.h>

#define ARENA_IMPL
#include "arena.h"

#define DRN_IMPL
#include "drn.h"

int main(int argc, char *argv[])
{

    DrnLoadRes lr = LoadDrnScriptFromFile("sample_drn/helloworld.drn");
    
    if (!lr.isOk)
    {
        fprintf(stderr, "%s\n", lr.error);
        free(lr.error);
        return 1;
    }

    

    return 0;
}