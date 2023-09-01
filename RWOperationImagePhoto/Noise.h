
#pragma once

// Perlin Noise functions

class CNoise
{
	enum
	{
		B = 0x100,
		BM = 0xff,
		N = 0x1000,

		// random number generator
		MODULUS = 2147483647,
		MULTIPLIER = 48271,
		CHECK = 399268537,
		A256 = 22925,
		DEFAULT = 123456789,
	};

	int p[B + B + 2];
	float g3[B + B + 2][3];
	float g2[B + B + 2][2];
	float g1[B + B + 2];

	long seed;
	long m_nOrigSeed;


	void normalize2(float* v)
	{
		float s = sqrtf(v[0] * v[0] + v[1] * v[1]);
		v[0] = v[0] / s;
		v[1] = v[1] / s;
	}

	void normalize3(float* v)
	{
		float s = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
		v[0] = v[0] / s;
		v[1] = v[1] / s;
		v[2] = v[2] / s;
	}

	long random()
	{
		static long const Q = MODULUS / MULTIPLIER;
		static long const R = MODULUS % MULTIPLIER;
        long t = MULTIPLIER * (seed % Q) - R * (seed / Q);
		seed = t > 0 ? t : (t + MODULUS);
		return seed;
	}

public:
	CNoise() : m_nOrigSeed(DEFAULT-1)
	{
		SetSeed(DEFAULT);
	}
	void SetSeed(long a_nSeed)
	{
		if (m_nOrigSeed == a_nSeed)
			return;

		m_nOrigSeed = seed = a_nSeed;
		int i, j, k;

		for (i = 0; i < B; i++)
		{
			p[i] = i;

			g1[i] = (float)((random() % (B + B)) - B) / B;

			for (j = 0; j < 2; j++)
				g2[i][j] = (float)((random() % (B + B)) - B) / B;
			normalize2(g2[i]);

			for (j = 0; j < 3; j++)
				g3[i][j] = (float)((random() % (B + B)) - B) / B;
			normalize3(g3[i]);
		}

		for (i = B-1; i >= 0; i--)
		{
			k = p[i];
			p[i] = p[j = random() % B];
			p[j] = k;
		}

		for (i = 0; i < B + 2; i++)
		{
			p[B + i] = p[i];
			g1[B + i] = g1[i];
			for (j = 0; j < 2; j++)
				g2[B + i][j] = g2[i][j];
			for (j = 0; j < 3; j++)
				g3[B + i][j] = g3[i][j];
		}
	}

	/**
	 * Compute turbulence using Perlin noise.
	 * @param x the x value
	 * @param y the y value
	 * @param octaves number of octaves of turbulence
	 * @return turbulence value at (x,y)
	 */
	float turbulence2(float x, float y, float octaves)
	{
		float t = 0.0f;

		for (float f = 1.0f; f <= octaves; f *= 2)
			t += fabsf(noise2(f * x, f * y)) / f;

		return t;
	}

	/**
	 * Compute turbulence using Perlin noise.
	 * @param x the x value
	 * @param y the y value
	 * @param octaves number of octaves of turbulence
	 * @return turbulence value at (x,y)
	 */
	float turbulence3(float x, float y, float z, float octaves)
	{
		float t = 0.0f;

		for (float f = 1.0f; f <= octaves; f *= 2)
			t += fabsf(noise3(f * x, f * y, f * z)) / f;

		return t;
	}

	float sCurve(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}
	
	/**
	 * Compute 1-dimensional Perlin noise.
	 * @param x the x value
	 * @return noise value at x in the range -1..1
	 */
	float noise1(float x)
	{
		int bx0, bx1;
		float rx0, rx1, sx, t, u, v;

		t = x + N;
		bx0 = ((int)t) & BM;
		bx1 = (bx0+1) & BM;
		rx0 = t - (int)t;
		rx1 = rx0 - 1.0f;

		sx = sCurve(rx0);

		u = rx0 * g1[p[bx0]];
		v = rx1 * g1[p[bx1]];
		return 2.3f*lerp(sx, u, v);
	}

	/**
	 * Compute 2-dimensional Perlin noise.
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @return noise value at (x,y)
	 */
	float noise2(float x, float y)
	{
		int bx0, bx1, by0, by1, b00, b10, b01, b11;
		float rx0, rx1, ry0, ry1, q[2], sx, sy, a, b, t, u, v;
		int i, j;

		t = x + N;
		bx0 = ((int)t) & BM;
		bx1 = (bx0+1) & BM;
		rx0 = t - (int)t;
		rx1 = rx0 - 1.0f;
	
		t = y + N;
		by0 = ((int)t) & BM;
		by1 = (by0+1) & BM;
		ry0 = t - (int)t;
		ry1 = ry0 - 1.0f;
	
		i = p[bx0];
		j = p[bx1];

		b00 = p[i + by0];
		b10 = p[j + by0];
		b01 = p[i + by1];
		b11 = p[j + by1];

		sx = sCurve(rx0);
		sy = sCurve(ry0);

		q[0] = g2[b00][0]; q[1] = g2[b00][1]; u = rx0 * q[0] + ry0 * q[1];
		q[0] = g2[b10][0]; q[1] = g2[b10][1]; v = rx1 * q[0] + ry0 * q[1];
		a = lerp(sx, u, v);

		q[0] = g2[b01][0]; q[1] = g2[b01][1]; u = rx0 * q[0] + ry1 * q[1];
		q[0] = g2[b11][0]; q[1] = g2[b11][1]; v = rx1 * q[0] + ry1 * q[1];
		b = lerp(sx, u, v);

		return 1.5f*lerp(sy, a, b);
	}

	/**
	 * Compute 3-dimensional Perlin noise.
	 * @param x the x coordinate
	 * @param y the y coordinate
	 * @param y the y coordinate
	 * @return noise value at (x,y,z)
	 */
	float noise3(float x, float y, float z)
	{
		int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
		float rx0, rx1, ry0, ry1, rz0, rz1, q[3], sy, sz, a, b, c, d, t, u, v;
		int i, j;

		t = x + N;
		bx0 = ((int)t) & BM;
		bx1 = (bx0+1) & BM;
		rx0 = t - (int)t;
		rx1 = rx0 - 1.0f;

		t = y + N;
		by0 = ((int)t) & BM;
		by1 = (by0+1) & BM;
		ry0 = t - (int)t;
		ry1 = ry0 - 1.0f;
	
		t = z + N;
		bz0 = ((int)t) & BM;
		bz1 = (bz0+1) & BM;
		rz0 = t - (int)t;
		rz1 = rz0 - 1.0f;
	
		i = p[bx0];
		j = p[bx1];

		b00 = p[i + by0];
		b10 = p[j + by0];
		b01 = p[i + by1];
		b11 = p[j + by1];

		t  = sCurve(rx0);
		sy = sCurve(ry0);
		sz = sCurve(rz0);

		for (int i = 0; i < 3; ++i) q[i] = g3[b00 + bz0][i]; u = rx0 * q[0] + ry0 * q[1] + rz0 * q[2];
		for (int i = 0; i < 3; ++i) q[i] = g3[b10 + bz0][i]; v = rx1 * q[0] + ry0 * q[1] + rz0 * q[2];
		a = lerp(t, u, v);

		for (int i = 0; i < 3; ++i) q[i] = g3[b01 + bz0][i]; u = rx0 * q[0] + ry1 * q[1] + rz0 * q[2];
		for (int i = 0; i < 3; ++i) q[i] = g3[b11 + bz0][i]; v = rx1 * q[0] + ry1 * q[1] + rz0 * q[2];
		b = lerp(t, u, v);

		c = lerp(sy, a, b);

		for (int i = 0; i < 3; ++i) q[i] = g3[b00 + bz1][i]; u = rx0 * q[0] + ry0 * q[1] + rz1 * q[2];
		for (int i = 0; i < 3; ++i) q[i] = g3[b10 + bz1][i]; v = rx1 * q[0] + ry0 * q[1] + rz1 * q[2];
		a = lerp(t, u, v);

		for (int i = 0; i < 3; ++i) q[i] = g3[b01 + bz1][i]; u = rx0 * q[0] + ry1 * q[1] + rz1 * q[2];
		for (int i = 0; i < 3; ++i) q[i] = g3[b11 + bz1][i]; v = rx1 * q[0] + ry1 * q[1] + rz1 * q[2];
		b = lerp(t, u, v);

		d = lerp(sy, a, b);

		return 1.5f*lerp(sz, c, d);
	}

	float lerp(float t, float a, float b)
	{
		return a + t * (b - a);
	}


	/**
	 * Returns the minimum and maximum of a number of random values
	 * of the given function. This is useful for making some stab at
	 * normalising the function.
	 */
	//float* findRange(Function1D f, float* minmax)
	//{
	//	if (minmax == null)
	//		minmax = new float[2];
	//	float min = 0, max = 0;
	//	// Some random numbers here...
	//	for (float x = -100; x < 100; x += 1.27139) {
	//		float n = f.evaluate(x);
	//		min = Math.min(min, n);
	//		max = Math.max(max, n);
	//	}
	//	minmax[0] = min;
	//	minmax[1] = max;
	//	return minmax;
	//}
	
	/**
	 * Returns the minimum and maximum of a number of random values
	 * of the given function. This is useful for making some stab at
	 * normalising the function.
	 */
	template<class TGenerator>
	static void findRange(TGenerator& f, float& min, float& max)
	{
		// Some random numbers here...
		for (float y = -100; y < 100; y += 10.35173)
		{
			for (float x = -100; x < 100; x += 10.77139)
			{
				float n = f.evaluate(x, y);
				if (n < min) min = n;
				if (n > max) max = n;
			}
		}
	}
};
