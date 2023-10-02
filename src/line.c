

#include "include/utils.h"
#include "include/line.h"




static Reading_t *segment_intersect(
    Reading_t *reading, 
    Vector2 A, Vector2 B, Vector2 C, Vector2 D
);


flt_t utils_randflt(flt_t lower_bound, flt_t upper_bound)
{
    return LERP(lower_bound, upper_bound, rand() / (double)RAND_MAX);
}

flt_t f32lerp(flt_t start, flt_t end, flt_t percentage)
{
    return start + (end - start) * percentage;
}


Reading_t *Line_Intersect(Reading_t *reading, const Line_t a, const Line_t b)
{
    return segment_intersect(reading, a.start, a.end, b.start, b.end);
}




Line_t Line_From(flt_t x, flt_t y, flt_t offset_w, flt_t offset_h)
{
    return (Line_t){
        .start = {
            .x = x,
            .y = y,
        },
        .end = {
            .x = x + offset_w,
            .y = y + offset_h,
        },
    };
}


Line_t Line_Scale(Line_t line, double scale)
{
    return (Line_t){
        .start.x = line.start.x * scale, 
        .start.y = line.start.y * scale,
        .end.x = line.end.x * scale,
        .end.y = line.end.y * scale,
    };
}



Line_t Line_Rotate(Line_t line, Vector2 org, flt_t rad)
{
    flt_t c = cos(rad), s = sin(rad);
    Vector2 sdiff = {.x = line.start.x - org.x, .y = line.start.y - org.y};
    Vector2 ediff = {.x = line.end.x - org.x, .y = line.end.y - org.y };
    Line_t ret;
    ret.start = (Vector2){
        .x = org.x + (c * sdiff.x - s * sdiff.y),
        .y = org.y + (s * sdiff.x + c * sdiff.y),
    };
    ret.end = (Vector2){
        .x = org.x + (c * ediff.x - s * ediff.y),
        .y = org.y + (s * ediff.x + c * ediff.y),
    };
    return ret;
}


bool Line_PolyCollide(Line_t* poly1, usize_t count1, Line_t* poly2, usize_t count2)
{
    for (usize_t i = 0; i < count1; i++)
    {
        for (usize_t k = 0; k < count2; k++)
        {
            Reading_t reading;
            if (Line_Intersect(&reading, poly1[i], poly2[k]))
                return true;
        }
    }
    return false;
}







bool Poly_Collide(const Polygon_t a, const Polygon_t b)
{

}





static Reading_t *segment_intersect(Reading_t *reading, Vector2 A, Vector2 B, Vector2 C, Vector2 D)
{
    flt_t t_top = (D.x-C.x)*(A.y-C.y)-(D.y-C.y)*(A.x-C.x);
    flt_t u_top = (C.y-A.y)*(A.x-B.x)-(C.x-A.x)*(A.y-B.y);
    flt_t bottom = (D.y-C.y)*(B.x-A.x)-(D.x-C.x)*(B.y-A.y);

    if (bottom)
    {
        flt_t t = t_top / bottom;
        flt_t u = u_top / bottom;
        if ((0 <= t && t <= 1) && (0 <= u && u <= 1))
        {
            reading->dist = t;
            reading->at.x = LERP(A.x, B.x, t);
            reading->at.y = LERP(A.y, B.y, t);
            return reading;
        }
    }
    return NULL;
}



