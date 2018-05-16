/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

push: push a new origin matrix onto the origin stack

pop: remove the top matrix on the origin stack

move/scale/rotate: create a transformation matrix
based on the provided values, then
multiply the current top of the
origins stack by it.

box/sphere/torus: create a solid object based on the
provided values. Store that in a
temporary matrix, multiply it by the
current top of the origins stack, then
call draw_polygons.

line: create a line based on the provided values. Store
that in a temporary matrix, multiply it by the
current top of the origins stack, then call draw_lines.

save: call save_extension with the provided filename

display: view the screen
=========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {

    int i;
    struct matrix *tmp;
    struct stack *systems;
    screen t;
    zbuffer zb;
    color g;
    double step_3d = 20;
    double theta;

    //Lighting values here for easy access
    color ambient;
    double light[2][3];
    double view[3];
    double areflect[3];
    double dreflect[3];
    double sreflect[3];

    ambient.red = 50;
    ambient.green = 50;
    ambient.blue = 50;

    light[LOCATION][0] = 0.5;
    light[LOCATION][1] = 0.75;
    light[LOCATION][2] = 1;

    light[COLOR][RED] = 0;
    light[COLOR][GREEN] = 255;
    light[COLOR][BLUE] = 255;

    view[0] = 0;
    view[1] = 0;
    view[2] = 1;

    areflect[RED] = 0.1;
    areflect[GREEN] = 0.1;
    areflect[BLUE] = 0.1;

    dreflect[RED] = 0.5;
    dreflect[GREEN] = 0.5;
    dreflect[BLUE] = 0.5;

    sreflect[RED] = 0.5;
    sreflect[GREEN] = 0.5;
    sreflect[BLUE] = 0.5;

    systems = new_stack();
    tmp = new_matrix(4, 1000);
    clear_screen( t );
    clear_zbuffer(zb);
    g.red = 0;
    g.green = 0;
    g.blue = 0;

    for (i=0;i<lastop;i++) {
        printf("%d: ",i);
        switch (op[i].opcode) {
            case SPHERE:
                printf("Sphere: %6.2f %6.2f %6.2f r=%6.2f",
                        op[i].op.sphere.d[0],op[i].op.sphere.d[1],
                        op[i].op.sphere.d[2],
                        op[i].op.sphere.r);
                add_sphere(tmp, op[i].op.sphere.d[0],op[i].op.sphere.d[1],
                        op[i].op.sphere.d[2],
                        op[i].op.sphere.r, step_3d);
                matrix_mult(peek(systems), tmp);
                draw_polygons(tmp, t, zb,
                        view, light, ambient, areflect, dreflect, sreflect);
                tmp->lastcol = 0;
                if (op[i].op.sphere.constants != NULL) {
                    printf("\tconstants: %s",op[i].op.sphere.constants->name);
                }
                if (op[i].op.sphere.cs != NULL) {
                    printf("\tcs: %s",op[i].op.sphere.cs->name);
                }

                break;
            case TORUS:
                printf("Torus: %6.2f %6.2f %6.2f r0=%6.2f r1=%6.2f",
                        op[i].op.torus.d[0],op[i].op.torus.d[1],
                        op[i].op.torus.d[2],
                        op[i].op.torus.r0,op[i].op.torus.r1);
                add_torus(tmp, op[i].op.torus.d[0],op[i].op.torus.d[1],
                        op[i].op.torus.d[2],
                        op[i].op.torus.r0,op[i].op.torus.r1, step_3d);
                matrix_mult(peek(systems), tmp);
                draw_polygons(tmp, t, zb,
                        view, light, ambient, areflect, dreflect, sreflect);
                if (op[i].op.torus.constants != NULL)
                {
                    printf("\tconstants: %s",op[i].op.torus.constants->name);
                }
                if (op[i].op.torus.cs != NULL)
                {
                    printf("\tcs: %s",op[i].op.torus.cs->name);
                }

                break;
            case BOX:
                printf("Box: d0: %6.2f %6.2f %6.2f d1: %6.2f %6.2f %6.2f",
                        op[i].op.box.d0[0],op[i].op.box.d0[1],
                        op[i].op.box.d0[2],
                        op[i].op.box.d1[0],op[i].op.box.d1[1],
                        op[i].op.box.d1[2]);
                add_box(tmp, op[i].op.box.d0[0],op[i].op.box.d0[1],
                        op[i].op.box.d0[2],
                        op[i].op.box.d1[0],op[i].op.box.d1[1],
                        op[i].op.box.d1[2]);
                matrix_mult(peek(systems), tmp);
                draw_polygons(tmp, t, zb,
                        view, light, ambient, areflect, dreflect, sreflect);
                if (op[i].op.box.constants != NULL)
                {
                    printf("\tconstants: %s",op[i].op.box.constants->name);
                }
                if (op[i].op.box.cs != NULL)
                {
                    printf("\tcs: %s",op[i].op.box.cs->name);
                }

                break;
            case LINE:
                printf("Line: from: %6.2f %6.2f %6.2f to: %6.2f %6.2f %6.2f",
                        op[i].op.line.p0[0],op[i].op.line.p0[1],
                        op[i].op.line.p0[1],
                        op[i].op.line.p1[0],op[i].op.line.p1[1],
                        op[i].op.line.p1[1]);
                add_edge(tmp, op[i].op.line.p0[0],op[i].op.line.p0[1],
                        op[i].op.line.p0[1],
                        op[i].op.line.p1[0],op[i].op.line.p1[1],
                        op[i].op.line.p1[1]);
                matrix_mult(peek(systems), tmp);
                draw_lines(tmp, t, zb, g);
                if (op[i].op.line.constants != NULL)
                {
                    printf("\n\tConstants: %s",op[i].op.line.constants->name);
                }
                if (op[i].op.line.cs0 != NULL)
                {
                    printf("\n\tCS0: %s",op[i].op.line.cs0->name);
                }
                if (op[i].op.line.cs1 != NULL)
                {
                    printf("\n\tCS1: %s",op[i].op.line.cs1->name);
                }
                break;
            case MOVE:
                printf("Move: %6.2f %6.2f %6.2f",
                        op[i].op.move.d[0],op[i].op.move.d[1],
                        op[i].op.move.d[2]);
                tmp = make_translate(op[i].op.move.d[0],op[i].op.move.d[1],
                        op[i].op.move.d[2]);
                matrix_mult(peek(systems), tmp);
                copy_matrix(tmp, peek(systems));
                tmp->lastcol = 0;
                if (op[i].op.move.p != NULL)
                {
                    printf("\tknob: %s",op[i].op.move.p->name);
                }
                break;
            case SCALE:
                printf("Scale: %6.2f %6.2f %6.2f",
                        op[i].op.scale.d[0],op[i].op.scale.d[1],
                        op[i].op.scale.d[2]);
                tmp = make_scale(op[i].op.scale.d[0],op[i].op.scale.d[1],
                        op[i].op.scale.d[2]);
                matrix_mult(peek(systems), tmp);
                copy_matrix(tmp, peek(systems));
                tmp->lastcol = 0;
                if (op[i].op.scale.p != NULL)
                {
                    printf("\tknob: %s",op[i].op.scale.p->name);
                }
                break;
            case ROTATE:
                printf("Rotate: axis: %6.2f degrees: %6.2f",
                        op[i].op.rotate.axis,
                        op[i].op.rotate.degrees);
                theta = op[i].op.rotate.degrees * (M_PI / 180);
                if (op[i].op.rotate.axis == 0) {
                    tmp = make_rotX( theta );
                }
                else if (op[i].op.rotate.axis == 1) {
                    tmp = make_rotY( theta );
                }
                else {
                    tmp = make_rotZ( theta );
                }
                matrix_mult(peek(systems), tmp);
                copy_matrix(tmp, peek(systems));
                tmp->lastcol = 0;
                if (op[i].op.rotate.p != NULL)
                {
                    printf("\tknob: %s",op[i].op.rotate.p->name);
                }
                break;
            case PUSH:
                push(systems);
                break;
            case POP:
                pop(systems);
                break;
            case SAVE:
                printf("Save: %s",op[i].op.save.p->name);
                save_extension(t, op[i].op.save.p->name);
                break;
            case DISPLAY:
                printf("Display");
                display(t);
                break;
        }
    }
}
