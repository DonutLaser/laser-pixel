#if !defined (GUI_MATH_H)
#define GUI_MATH_H

#define ARRAY_SIZE(x) (sizeof (x) / (sizeof ((x)[0])))
#define ABS(x) ((x < 0) ? (-x) : (x))
#define MAX(x, y) ((x > y) ? (x) : (y))
#define MIN(x, y) ((x > y) ? (y) : (x))
#define CLAMP(x, min, max) ((x < min) ? (min) : ((x > max) ? (max) : (x)))
#define BETWEEN(x, min, max) ((x >= min) && (x <= max))

union v2 {
	struct {
		float x, y;
	};
	struct {
		float u, v;
	};
	float f[2];
};

union v3 {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	float f[3];
};

union v4 {
	struct {
		float x, y, z, w; // Position
	};
	struct {
		float r, g, b, a; // Color
	};
	struct {
		float top, right, bottom, left; // Margin
	};
	float f[4];
};

struct m4 {
	float value[4][4];
};

struct rect {
	float x, y, width, height;
};

int powi (int x, int y);

v2 make_v2 (float x, float y);
v2 make_v2 (int x, int y);
v2 make_v2 (unsigned x, unsigned y);

v3 make_v3 (float x, float y, float z);
v3 make_v3 (int x, int y, int z);
v3 make_v3 (unsigned x, unsigned y, unsigned z);
v3 make_v3 (v2 xy, float z);

v4 make_v4 (float x, float y, float z, float w);
v4 make_v4 (int x, int y, int z, int w);
v4 make_v4 (unsigned x, unsigned y, unsigned z, unsigned w);
v4 make_v4 (v3 xyz, float w);
v4 make_v4 (v2 xy, float z, float w);

v4 make_color (unsigned x, unsigned y, unsigned z, unsigned w);

m4 make_identity ();
m4 transpose (m4 matrix);
m4 make_ortho (float left, float right, float top, float bottom, float near, float far);
m4 translate (m4 matrix, v3 position);
m4 scale (m4 matrix, v3 scale);

rect make_rect (float x, float y, float width, float height);
rect make_rect (int x, int y, int width, int height);
rect make_rect (unsigned x, unsigned y, unsigned width, unsigned height);
rect make_rect (v2 pos, float width, float height);
rect make_rect (v2 pos, v2 size);
bool is_point_in_rect (rect r, v2 point);

unsigned digit_count (int value);
bool is_int_in_range (long long int value); 

bool operator== (v4 lhs, v4 rhs);
v4 operator+ (v2 lhs, v2 rhs);
v4 operator- (v2 lhs, v2 rhs);

#endif