#ifndef CAI_LINE_H
#define CAI_LINE_H



#include "common.h"
#include <raylib.h>

typedef struct Line_t
{
    Vector2 start, end;
} Line_t;
typedef struct Reading_t
{
    Vector2 at;
    flt_t dist;
} Reading_t;

/* returns NULL if a & b does not intersect, else return the pointer given */
Reading_t *Line_Intersect(Reading_t *at, const Line_t a, const Line_t b);
Line_t Line_From(flt_t x, flt_t y, flt_t offset_w, flt_t offset_h);
Line_t Line_Scale(Line_t line, double scale);
Line_t Line_Rotate(Line_t line, Vector2 origin, flt_t rad);
bool Line_PolyCollide(Line_t* poly1, usize_t count1, Line_t* poly2, usize_t count2);



typedef struct Polygon_t
{
    Vector2 *points;
    usize_t count;
} Polygon_t;


/* polygon does not own the points */
#define Polygon_Init(numlines, p_points) (Polygon_t){.count = numlines, .points = p_points}
#define Polygon_Deinit(p_polygon) (*(p_polygon) = ZEROALL(Polygon_t))

bool Poly_Collide(const Polygon_t a, const Polygon_t b);




#endif /* CAI_LINE_H */

