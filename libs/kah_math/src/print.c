//===INCLUDES==================================================================
#include <kah_math/print.h>

#include <kah_math/mat4.h>
#include <stdio.h>
//=============================================================================

//===API=======================================================================
void print_mat4f(const mat4f* m){
   printf(
        "%.2f\t%.2f\t%.2f\t%.2f\n"
        "%.2f\t%.2f\t%.2f\t%.2f\n"
        "%.2f\t%.2f\t%.2f\t%.2f\n"
        "%.2f\t%.2f\t%.2f\t%.2f\n",
        m->col[0].x, m->col[1].x, m->col[2].x, m->col[3].x,
        m->col[0].y, m->col[1].y, m->col[2].y, m->col[3].y,
        m->col[0].z, m->col[1].z, m->col[2].z, m->col[3].z,
        m->col[0].w, m->col[1].w, m->col[2].w, m->col[3].w
    );
}

void print_transform(const Transform* t){
    printf(
        "P:%.2f\t%.2f\t%.2f\n"
        "R:%.2f\t%.2f\t%.2f\n"
        "S:%.2f\t%.2f\t%.2f\n",
        t->position.x, t->position.y, t->position.z,
        t->rotation.x, t->rotation.y, t->rotation.z,
        t->scale.x, t->scale.y, t->scale.z
    );
}
//=============================================================================
