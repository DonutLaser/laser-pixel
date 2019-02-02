#include "gui_math.h"

#include <limits.h>

int powi (int x, int y) {
	int result = 1;
	while (y > 0) {
		result *= x;
		--y;
	}

	return result;
}

v2 make_v2 (float x, float y) {
	return { x, y };
}

v2 make_v2 (int x, int y) {
	return { (float)x, (float)y };
}

v2 make_v2 (unsigned x, unsigned y) {
	return { (float)x, (float)y };
}

v3 make_v3 (float x, float y, float z) {
	return { x, y, z };
}

v3 make_v3 (v2 xy, float z) {
	return { xy.x, xy.y, z };
}

v3 make_v3 (int x, int y, int z) {
	return { (float)x, (float)y, (float)z };
}

v3 make_v3 (unsigned x, unsigned y, unsigned z) {
	return { (float)x, (float)y, (float)z };
}

v4 make_v4 (float x, float y, float z, float w) {
	return { x, y, z, w };
}

v4 make_v4 (int x, int y, int z, int w) {
	return { (float)x, (float)y, (float)z, (float)w };
}

v4 make_v4 (unsigned x, unsigned y, unsigned z, unsigned w) {
	return { (float)x, (float)y, (float)z, (float)w };
}

v4 make_v4 (v3 xyz, float w) {
	return { xyz.x, xyz.y, xyz.z, w };
}

v4 make_v4 (v2 xy, float z, float w) {
	return { xy.x, xy.y, z, w };
}

v4 make_color (unsigned x, unsigned y, unsigned z, unsigned w) {
	return { (float)x / 255, (float)y / 255, (float)z / 255, (float)w / 255 };
}

m4 make_identity () {
	m4 result = { };
	for (unsigned i = 0; i < 4; ++i)
		result.value[i][i] = 1.0f;
	
	return result;
}

m4 transpose (m4 matrix) {
	m4 result;
	for (int i = 0; i <= 3; ++i) {
		for (int j = 0; j <= 3; ++j)
			result.value[i][j] = matrix.value[j][i];
	}

	return result;
}

m4 make_ortho (float left, float right, float top, float bottom, float near, float far) {
	m4 result = { };

	result.value[0][0] = 2 / (right - left);
	result.value[1][1] = 2 / (top - bottom);
	result.value[2][2] = 2 / (far - near);
	result.value[3][0] = -((right + left) / (right - left));
	result.value[3][1] = -((top + bottom) / (top - bottom));
	result.value[3][2] = -((far + near) / (far - near));
	result.value[3][3] = 1;

	return result;
}

m4 translate (m4 matrix, v3 position) {
	matrix.value[0][3] += position.x;
	matrix.value[1][3] += position.y;
	matrix.value[2][3] += position.z;

	return matrix;
}

m4 scale (m4 matrix, v3 scale) {
	matrix.value[0][0] *= scale.x;
	matrix.value[1][1] *= scale.y;
	matrix.value[2][2] *= scale.z;

	return matrix;
}

rect make_rect (float x, float y, float width, float height) {
	return { x, y, width, height };
}

rect make_rect (int x, int y, int width, int height) {
	return { (float)x, (float)y, (float)width, (float)height };
}

rect make_rect (unsigned x, unsigned y, unsigned width, unsigned height) {
	return { (float)x, (float)y, (float)width, (float)height };
}

rect make_rect (v2 pos, float width, float height) {
	return { pos.x, pos.y, width, height };
}

rect make_rect (v2 pos, v2 size) {
	return { pos.x, pos.y, size.x, size.y };
}

bool is_point_in_rect (rect r, v2 point) {
	return point.x > r.x && point.x < r.x + r.width &&
		   point.y > r.y && point.y < r.y + r.height;
}

unsigned digit_count (int value) {
	unsigned result = 0;
	do {
		++result;
		value /= 10;
	} while (value != 0);

	return result;
}

bool is_int_in_range (long long int value) {
	return value <= INT_MAX && value >= INT_MIN;
}

bool operator== (v4 lhs, v4 rhs) {
		return lhs.x == rhs.x && lhs.y == rhs.y &&
			   lhs.z == rhs.z && lhs.w == rhs.w;
}

v4 operator+ (v2 lhs, v2 rhs) {
	return { lhs.x + rhs.x, lhs.y + rhs.y };
}

v4 operator- (v2 lhs, v2 rhs) {
	return { lhs.x - rhs.x, lhs.y - rhs.y };
}
