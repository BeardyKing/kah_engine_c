#include <kah_math/mat4.h>
#include <stdio.h>

void mat4f_print(const mat4f* m){
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
