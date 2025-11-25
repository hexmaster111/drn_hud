#include <raylib.h>
#include <raymath.h>

#define ARENA_IMPL
#include "arena.h"

#define DRN_IMPL
#include "drn.h"

int main(int argc, char *argv[])
{

    DrnNode *join_task = &(DrnNode){
        .kind = DNK_TASK,
        .code = SLICE_CLIT("TASK: Tell admin \"Task Complete\""),
    };

    DrnScript s = {
        .start = &(DrnNode){
            .kind = DNK_TASK,
            .code = SLICE_CLIT("TASK: Fill dishwasher with dirty items from sink."),
            .next = &(DrnNode){
                .kind = DNK_CONDITION,
                .code = SLICE_CLIT("CONDITION: Dishwasher is 75%% or greater."),
                .inner = &(DrnNode){
                    .kind = DNK_TASK,
                    .code = SLICE_CLIT("TASK: Start Washer."),
                    .next = join_task},
                .next = join_task},
        }};

    DrnLoadRes lr = LoadDrnScriptFromFile("sample_drn/condition.drn");

    if (!lr.isOk)
    {
        fprintf(stderr, "%s\n", lr.error);
        free(lr.error);
        return 1;
    }

    return 0;
}