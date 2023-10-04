

#include "include/utils.h"
#include "include/line.h"




static Reading_t *segment_intersect(
    Reading_t *reading, 
    Vector2 A, Vector2 B, Vector2 C, Vector2 D
);

static Vector2 vec_rot(Vector2 v, Vector2 org, flt_t cos_theta, flt_t sin_theta);


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













bool Polygon_OrthogonalRect(const Polygon_t p)
{
    if (p.count != 4) return false;

    /* dot product of vec([0]->[1]) and vec([0]->[3])*/
    Vector2 a = {
        .x = p.points[1].x - p.points[0].x,
        .y = p.points[1].y - p.points[0].y,
    };
    Vector2 b = {
        .x = p.points[3].x - p.points[0].x,
        .y = p.points[3].y - p.points[0].y,
    };
    return flt_equ(0, a.x*b.x + a.y*b.y);
}





bool Polygon_TouchedRect(const Polygon_t a, const Polygon_t b)
{
    Vector2 _min = b.points[0];
    Vector2 _max = b.points[2];

    for (usize_t i = 0; i < a.count; i++)
    {
        Vector2 p = {.x = a.points[i].x, .y = a.points[i].y};
        /* point is inside b */
        if (!(p.x < _min.x || p.x > _max.x || p.y < _min.y || p.y > _max.y))
            return true;
    }
    return false;
}



bool Polygon_InsideRect(const Polygon_t a, const Polygon_t b)
{
    Vector2 _min = b.points[0];
    Vector2 _max = b.points[2];

    for (usize_t i = 0; i < a.count; i++)
    {
        Vector2 p = {.x = a.points[i].x, .y = a.points[i].y};
        /* point is NOT inside b */
        if ((p.x < _min.x || p.x > _max.x || p.y < _min.y || p.y > _max.y))
            return false;
    }
    return true;
}




void Polygon_RectFrom(Polygon_t *p, Vector2 center, flt_t w, flt_t h, flt_t angle)
{
    CAI_ASSERT(p->count >= 4, "polygon does not contain enough points for a rectangle");
    flt_t c = cos(angle), s = sin(angle);
    flt_t x = center.x - w/2, y = center.y - h/2;

    p->points[0] = (Vector2){.x = x, .y = y};
    p->points[1] = (Vector2){.x = x, .y = y + h};
    p->points[2] = (Vector2){.x = x + w, .y = y + h};
    p->points[3] = (Vector2){.x = x + w, .y = y};
    for (int i = 0; i < 4; i++)
    {
        p->points[i] = vec_rot(p->points[i], center, c, s);
    }
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


static Vector2 vec_rot(Vector2 v, Vector2 org, flt_t c, flt_t s)
{
    Vector2 d = {.x = v.x - org.x, .y = v.y - org.y};
    v.x = org.x + (c*d.x - s*d.y);
    v.y = org.y + (s*d.x + c*d.y);
    return v;
}

