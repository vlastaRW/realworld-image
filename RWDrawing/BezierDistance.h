
#pragma once

struct TVector2d
{
	TVector2d() : x(0.0), y(0.0) {}
	TVector2d(double xx, double yy) : x(xx), y(yy) {}
	TVector2d(TPixelCoords a) : x(a.fX), y(a.fY) {}
	TVector2d(TVector2d const& rhs) : x(rhs.x), y(rhs.y) {}
	double Distance(TVector2d const& t) const { return sqrt((x-t.x)*(x-t.x) + (y-t.y)*(y-t.y)); }
	double DistanceSquared(TVector2d const& t) const { return (x-t.x)*(x-t.x) + (y-t.y)*(y-t.y); }
	TVector2d operator-(TVector2d const& t) const { return TVector2d(x-t.x, y-t.y); }
	TVector2d operator+(TVector2d const& t) const { return TVector2d(x+t.x, y+t.y); }
	double Length() const { return sqrt(x*x + y*y); }
	void Normalize() { double li = 1.0/sqrt(x*x + y*y); x*=li; y*=li; }
	void SetLength(double l) { double li = l/sqrt(x*x + y*y); x*=li; y*=li; }
	operator TPixelCoords() const { TPixelCoords const t = {x, y}; return t; }
	double Dot(TVector2d const& t) const { return x*t.x + y*t.y; }
	TVector2d operator*(double s) const { return TVector2d(x*s, y*s); }

	double x, y;
};

const int MAXDEPTH = 64; // maximum depth for recursion

const double EPSILON = 2.7105054312137610850186320021749e-20; // flatness control value = 2^(-MAXDEPTH-1) 
const int DEGREE = 3; // cubic Bezier curve
const int W_DEGREE = 5; // degree of eqn to find roots of
const int MAX_DEGREE = 5; // degree of eqn to find roots of

struct Bezier
{
	Bezier(double a_x1, double a_y1, double a_x2, double a_y2, double a_x3, double a_y3, double a_x4, double a_y4) : m_degree(3), m_pts(new TVector2d[4])
	{
		m_pts[0].x = a_x1;
		m_pts[0].y = a_y1;
		m_pts[1].x = a_x2;
		m_pts[1].y = a_y2;
		m_pts[2].x = a_x3;
		m_pts[2].y = a_y3;
		m_pts[3].x = a_x4;
		m_pts[3].y = a_y4;
	}
	Bezier(int a_degree) : m_degree(a_degree), m_pts(new TVector2d[a_degree+1])
	{
	}
	Bezier(int a_degree, TVector2d const* pts) : m_degree(a_degree), m_pts(new TVector2d[a_degree+1])
	{
		for (int i = 0; i < (a_degree+1); ++i)
		{
			m_pts[i] = pts[i];
		}
	}
	~Bezier()
	{
		delete[] m_pts;
	}
	TVector2d Evaluate(double t, Bezier* left = NULL, Bezier* right = NULL)
	{
		TVector2d temp[MAX_DEGREE+1][MAX_DEGREE+1];
		for (int j = 0; j <= m_degree; ++j)
		{
			temp[0][j] = m_pts[j];
		}

		for (int i = 1; i <= m_degree; ++i)
		{
			for (int j = 0; j <= m_degree - i; ++j)
			{
				temp[i][j].x = (1.0 - t) * temp[i-1][j].x + t * temp[i-1][j+1].x;
				temp[i][j].y = (1.0 - t) * temp[i-1][j].y + t * temp[i-1][j+1].y;
			}
		}

		if (left)
			for (int j = 0; j <= m_degree; ++j)
				left->m_pts[j] = temp[j][0];

		if (right)
			for (int j = 0; j <= m_degree; ++j) 
				right->m_pts[j] = temp[m_degree-j][j];

		return temp[m_degree][0];
	}
	double ComputeXIntercept()
	{
		double XLK = 1.0 - 0.0;
		double YLK = 0.0 - 0.0;
		double XNM = m_pts[m_degree].x - m_pts[0].x;
		double YNM = m_pts[m_degree].y - m_pts[0].y;
		double XMK = m_pts[0].x - 0.0;
		double YMK = m_pts[0].y - 0.0;

		double det = XNM*YLK - YNM*XLK;
		double detInv = 1.0/det;

		double S = (XNM*YMK - YNM*XMK) * detInv;
		//double T = (XLK*YMK - YLK*XMK) * detInv;
    
		double X = 0.0 + XLK * S;
		//double Y = 0.0 + YLK * S;

		return X;
	}
	bool ControlPolygonFlatEnough()
	{
		double distance[MAX_DEGREE+1]; // distances from pts to line

		double a = m_pts[0].y - m_pts[m_degree].y;
		double b = m_pts[m_degree].x - m_pts[0].x;
		double c = m_pts[0].x * m_pts[m_degree].y - m_pts[m_degree].x * m_pts[0].y;

		double abSquared = (a * a) + (b * b);

		for (int i = 1; i < m_degree; i++)
		{
			// compute distance from each of the points to that line
			distance[i] = a * m_pts[i].x + b * m_pts[i].y + c;
			if (distance[i] > 0.0)
				distance[i] = (distance[i] * distance[i]) / abSquared;
			else if (distance[i] < 0.0)
				distance[i] = -((distance[i] * distance[i]) / 						abSquared);
		}

		// find the largest distance
		double max_distance_above = 0.0;
		double max_distance_below = 0.0;
		for (int i = 1; i < m_degree; i++)
		{
			if (distance[i] < 0.0)
				max_distance_below = max_distance_below < distance[i] ? max_distance_below : distance[i];
			else if (distance[i] > 0.0)
				max_distance_above = max_distance_above > distance[i] ? max_distance_above : distance[i];
		}

		// implicit equation for zero line
		double a1 = 0.0;
		double b1 = 1.0;
		double c1 = 0.0;

		// implicit equation for "above" line
		double a2 = a;
		double b2 = b;
		double c2 = c + max_distance_above;

		double det = a1*b2 - a2*b1;
		double dInv = 1.0/det;

		double intercept_1 = (b1*c2 - b2*c1) * dInv;

		a2 = a;
		b2 = b;
		c2 = c + max_distance_below;

		det = a1 * b2 - a2 * b1;
		dInv = 1.0/det;

		double intercept_2 = (b1*c2 - b2*c1) * dInv;

		double left_intercept = intercept_1 < intercept_2 ? intercept_1 : intercept_2;
		double right_intercept = intercept_1 > intercept_2 ? intercept_1 : intercept_2;

		double error = 0.5 * (right_intercept-left_intercept);    
		return error < EPSILON;
	}
	int CrossingCount()
	{
		int n_crossings = 0;

		bool sign = m_pts[0].y < 0;
		bool old_sign = sign;
		for (int i = 1; i <= m_degree; i++) 
		{
			sign = m_pts[i].y < 0;
			if (sign != old_sign)
				n_crossings++;
			old_sign = sign;
		}
		return n_crossings;
	}

	int FindRoots(int depth, std::vector<double>& roots)
	{
		switch (CrossingCount())
		{
		case 0: return 0;
		case 1: // unique solution
			if (depth >= MAXDEPTH)
			{
				roots.push_back((m_pts[0].x + m_pts[m_degree].x) / 2.0);
				return 1;
			}
			if (ControlPolygonFlatEnough())
			{
				roots.push_back(ComputeXIntercept());
				return 1;
			}
			break;
		}

		Bezier Left(m_degree);
		Bezier Right(m_degree);
		Evaluate(0.5, &Left, &Right);
		int n = Left.FindRoots(depth+1, roots);
		n += Right.FindRoots(depth+1, roots);
		return n;
	}

	void ConvertToBezierForm(TVector2d P, Bezier& result)
	{
		double z[3][4] = // precomputed "z" for cubics
		{
			{1.0, 0.6, 0.3, 0.1},
			{0.4, 0.6, 0.6, 0.4},
			{0.1, 0.3, 0.6, 1.0},
		};

		TVector2d c[MAX_DEGREE+1];
		for (int i = 0; i <= m_degree; ++i) 
			c[i] = m_pts[i]-P;

		TVector2d d[MAX_DEGREE];
		for (int i = 0; i < m_degree; ++i) 
		{
			d[i].x = (m_pts[i+1].x-m_pts[i].x)*3.0;
			d[i].y = (m_pts[i+1].y-m_pts[i].y)*3.0;
		}

		double cdTable[3][4];
		for (int row = 0; row < m_degree; ++row)
			for (int col = 0; col <= m_degree; ++col)
				cdTable[row][col] = d[row].x*c[col].x + d[row].y*c[col].y;

		for (int i = 0; i <= (m_degree+2); ++i)
		{
			result.m_pts[i].y = 0.0;
			result.m_pts[i].x = double(i)/(m_degree+2);
		}

		int n = m_degree;
		int m = m_degree-1;
		for (int k = 0; k <= n + m; ++k)
		{
			int lb = 0 > (k - m) ? 0 : (k - m);
			int ub = k < n ? k : n;
			for (int i = lb; i <= ub; ++i)
			{
				int j = k - i;
				result.m_pts[i+j].y += cdTable[j][i] * z[j][i];
			}
		}
	}

	// return the parameter in range 0-1 that corresponds to the point nearest to P on the curve
	double NearestPointOnCurve(TVector2d P)
	{
		// end points
		double t = 0.0;
		double dist = P.DistanceSquared(m_pts[0]);
		if (P.DistanceSquared(m_pts[m_degree]) < dist)
			t = 1.0;

		// intersections
		Bezier w(5);
		ConvertToBezierForm(P, w);
		std::vector<double> solutions;
		w.FindRoots(0, solutions);

		// distances of candidate points
		for (std::vector<double>::const_iterator sol = solutions.begin(); sol != solutions.end(); ++sol)
		{
			TVector2d p = Evaluate(*sol);
			double new_dist = P.DistanceSquared(p);
			if (new_dist < dist) 
			{
				dist = new_dist;
				t = *sol;
			}
		}

		return t;
	}

	int const m_degree;
	TVector2d* m_pts;


	static void Decimate(std::vector<TVector2d> const& input, double scale, std::vector<TVector2d>& output)
	{
		if (input.size() < 2)
			return;

		size_t const skip = 8;//max(3.0, 6*scale+0.5);

		std::vector<double> dist;
		{
			std::vector<TVector2d>::const_iterator i1 = input.begin();
			std::vector<TVector2d>::const_iterator i2 = i1;
			++i2;
			while (i2 != input.end())
			{
				dist.push_back(sqrt((i1->x-i2->x)*(i1->x-i2->x)+(i1->y-i2->y)*(i1->y-i2->y)));
				i1 = i2;
				++i2;
			}
		}

		std::vector<double> weight;
		weight.push_back(1.0);
		for (size_t i = 1; i < input.size()-1; ++i)
		{
			double l = sqrt((input[i-1].x-input[i+1].x)*(input[i-1].x-input[i+1].x)+(input[i-1].y-input[i+1].y)*(input[i-1].y-input[i+1].y));
			weight.push_back(pow((dist[i-1]+dist[i])/l, 7.0));
		}
		weight.push_back(1.0);
		
		output.push_back(input[0]);
		
		size_t ptFrom;
		double d = 0.0;
		for(ptFrom = 0; ptFrom < input.size()-1; ++ptFrom)
		{
			d += dist[ptFrom]*weight[ptFrom];
			if (d > 50.0*scale || ptFrom > skip)
				break;
		}
		++ptFrom;
		d = 0.0;
		size_t ptTo;
		for(ptTo = input.size()-2; ptTo > 0; --ptTo)
		{
			d += dist[ptTo]*weight[ptTo];
			if (d > 50.0*scale || ptTo+skip < input.size())
				break;
		}
		TVector2d newPt(0.0, 0.0);
		size_t newPts = 0;
		d = 0.0;
		for(size_t i = ptFrom; i <= ptTo; ++i)
		{
			if (newPts > skip || (newPts > 0 && d+dist[i] > 100.0*scale))
			{
				newPt.x /= newPts;
				newPt.y /= newPts;
				output.push_back(newPt);
				newPt.x = 0.0;
				newPt.y = 0.0;
				newPts = 0;
				d = 0.0;
			}
			d += dist[i]*weight[i];
			newPt.x += input[i].x;
			newPt.y += input[i].y;
			++newPts;
		}
		if (newPts > 0)
		{
			newPt.x /= newPts;
			newPt.y /= newPts;
			output.push_back(newPt);
		}
		
		output.push_back(input[input.size()-1]);
		
		for (size_t i = 1; i < output.size(); ++i)
			if(output[i].x == output[i-1].x && output[i].y == output[i-1].y)
				output[i].x += 1;
	}

	//C++ implementation of
	//An Algorithm for Automatically Fitting Digitized Curves
	//by Philip J. Schneider
	//from "Graphics Gems", Academic Press, 1990

	static TVector2d computeLeftTangent(std::vector<TVector2d> const& d, int at)
	{
		int prev = max(at-1, 0);
		TVector2d tHat1 = d[at+1] - d[prev];
		tHat1.Normalize();
		return tHat1;
	}

	static TVector2d computeRightTangent(std::vector<TVector2d> const& d, int at)
	{
		int next = min(at+1, (int)(d.size())-1);
		TVector2d tHat2 = d[at-1] - d[next];
		tHat2.Normalize();
		return tHat2;
	}

	static TVector2d computeCenterTangent(std::vector<TVector2d> const& d, int at)
	{
		TVector2d tHatCenter = d[at-1] - d[at+1];
		//TVector2d V1 = d[center - 1] - d[center];
		//TVector2d V2 = d[center] - d[center + 1];
		//TVector2d tHatCenter;
		//tHatCenter.x = (V1.x + V2.x) / 2.0;
		//tHatCenter.y = (V1.y + V2.y) / 2.0;
		tHatCenter.Normalize();
		return tHatCenter;
	}

	// ChordLengthParameterize :
	//  Assign parameter values to points
	//  using relative distances between points.
	static void chordLengthParameterize(std::vector<TVector2d> const& d, int first, int last, double* u)
	{
		u[0] = 0.0;
		int const n = last-first+1;
		for (int i = 1; i < n; ++i)
			u[i] = u[i-1] + d[first+i].Distance(d[first+i-1]);

		double const f = 1.0/u[n-1];
		for (int i = 1; i < n-1; ++i)
			u[i] *= f;
		u[n-1] = 1.0;
	}

	// ComputeMaxError :
	//  Find the maximum squared distance of digitized points
	//  to fitted curve.
	static double computeMaxError(std::vector<TVector2d> const& d, int first, int last, TVector2d const* bezCurve, double const* u, int* splitPoint)
	{
		*splitPoint = (last + first + 1) / 2;
		double maxDist = 0.0;
		for (int i = first + 1; i < last; ++i)
		{
			TVector2d P = bezierII3(bezCurve, u[i - first]); // Point on curve
			double dist = P.DistanceSquared(d[i]);
			if (dist >= maxDist)
			{
				maxDist = dist;
				*splitPoint = i;
			}
		}
		return maxDist;
	}

	template<typename Consumer>
	static void FitCubic(std::vector<TVector2d> const &d, int first, int last, TVector2d tHat1, TVector2d tHat2, double error, Consumer& consumer)
	{
		// Control points of fitted Bezier curve;
		TVector2d bezCurve[4];

		double maxError;     // Maximum fitting error
		int maxIterations = 20; // Max times to try iterating
		TVector2d tHatCenter; // Unit tangent vector at splitPoint
		int i;

		double iterationError = error * error; // Error below which you try iterating
		int nPts = last - first + 1; // Number of points in subset 
		if (nPts <= 1)
			return;

		//  Use line if region only has two points in it
		if (nPts == 2)
		{
			bezCurve[0] = d[first];
			bezCurve[3] = d[last];
			bezCurve[1] = bezCurve[0];
			bezCurve[2] = bezCurve[3];
			consumer.add(bezCurve);
			return;
		}

		// Parameterize points, and attempt to fit curve
		CAutoVectorPtr<double> u(new double[last - first + 1]);
		chordLengthParameterize(d, first, last, u);
		generateBezier(d, bezCurve, first, last, u, tHat1, tHat2);

		int splitPoint;      // Point to split point set at
		//  Find max deviation of points to fitted curve    
		maxError = computeMaxError(d, first, last, bezCurve, u, &splitPoint);
		//cout << maxError << endl;
		//cout << splitPoint << endl;
		if (maxError < error)
		{
			consumer.add(bezCurve);
			return;
		}

		//  If error not too large, try some reparameterization
		//  and iteration 
		if (maxError < iterationError)
		{
			// Improved parameter values
			CAutoVectorPtr<double> uPrime(new double[last - first + 1]);
			for (i = 0; i < maxIterations; ++i)
			{
				reparameterize(d, first, last, u, uPrime, bezCurve);
				generateBezier(d, bezCurve, first, last, uPrime, tHat1, tHat2);
				maxError = computeMaxError(d, first, last, bezCurve, uPrime, &splitPoint);
				if (maxError < error)
				{
					consumer.add(bezCurve);
					return;
				}
				std::swap(u.m_p, uPrime.m_p);
			}
		}

		// Fitting failed -- split at max error point and fit recursively
		tHatCenter = computeCenterTangent(d, splitPoint);
		FitCubic(d, first, splitPoint, tHat1, tHatCenter, error, consumer);
		tHatCenter.x = -tHatCenter.x;
		tHatCenter.y = -tHatCenter.y;
		FitCubic(d, splitPoint, last, tHatCenter, tHat2, error, consumer);
	}

	template<typename Consumer>
	static void FitCurve(std::vector<TVector2d> const &d, int startIndex, int endIndex, double error, Consumer& consumer)
	{
		if (error <= 0.0)
			error = 2.0;

		// Unit tangent vectors at endpoints.
		// if startIndex is the beginning of the curve.
		TVector2d tHat1 = computeLeftTangent(d, startIndex);
		// if endIndex is the end of the curve.
		TVector2d tHat2 = computeRightTangent(d, endIndex - 1);
		FitCubic(d, startIndex, endIndex - 1, tHat1, tHat2, error, consumer);
	}



#define MAXPOINTS 1000
	static void generateBezier(std::vector<TVector2d> const& d, TVector2d* bezCurve, int first, int last, double const* uPrime, TVector2d tHat1, TVector2d tHat2)
	{
		int nPts = last - first + 1; // Number of pts in sub-curve
		double C[2][2];         // Matrix C
		double X[2];            // Matrix X
		double det_C0_C1, det_C0_X, det_X_C1; // Determinants of matrices
		double alpha_l, alpha_r; // Alpha values, left and right
		TVector2d tmp;          // Utility variable

		// Compute the A
		CAutoVectorPtr<TVector2d> A0(new TVector2d[MAXPOINTS*2]);  // Precomputed rhs for eqn
		TVector2d* A1 = A0.m_p+MAXPOINTS;
		for (int i = 0; i < nPts; ++i)
		{
			A0[i] = tHat1;
			A0[i].SetLength(B1(uPrime[i]));
			A1[i] = tHat2;
			A1[i].SetLength(B2(uPrime[i]));
		}

		// Create the C and X matrices
		C[0][0] = 0.0;
		C[0][1] = 0.0;
		C[1][0] = 0.0;
		C[1][1] = 0.0;
		X[0] = 0.0;
		X[1] = 0.0;

		TVector2d const helper[] = {d[first], d[first], d[last]};
		for (int i = 0; i < nPts; i++)
		{
			C[0][0] += A0[i].Dot(A0[i]);
			C[0][1] += A0[i].Dot(A1[i]);

			C[1][0] = C[0][1];

			C[1][1] += A1[i].Dot(A1[i]);

			tmp = (d[first + i]) - bezierII3(helper, uPrime[i]);
					//((d[first] * B0(uPrime[i])) +
					//	((d[first] * B1(uPrime[i])) +
					//		((d[last] * B2(uPrime[i])) +
					//			(d[last] * B3(uPrime[i])))));

			X[0] += A0[i].Dot(tmp);
			X[1] += A1[i].Dot(tmp);
		}

		// Compute the determinants of C and X
		det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
		det_C0_X = C[0][0] * X[1] - C[1][0] * X[0];
		det_X_C1 = X[0] * C[1][1] - X[1] * C[0][1];

		// Finally, derive alpha values
		alpha_l = (det_C0_C1 == 0) ? 0.0 : det_X_C1 / det_C0_C1;
		alpha_r = (det_C0_C1 == 0) ? 0.0 : det_C0_X / det_C0_C1;

		// Checks for "dangerous" points, meaning that the alpha_l or alpha_r are abnormally large
		// from here http://newsgroups.derkeiler.com/Archive/Comp/comp.graphics.algorithms/2005-08/msg00419.html
		// This is a common problem with this algoithm.

		double dif1 = d[first].Distance(d[last]);
		bool danger = false;
		if ((alpha_l > dif1 * 2) || (alpha_r > dif1 * 2)) {
			first += 0;
			danger = true;
		}

		//  If alpha negative, use the Wu/Barsky heuristic (see text)
		//  (if alpha is 0, you get coincident control points that lead to
		//  divide by zero in any subsequent NewtonRaphsonRootFind() call.
		double segLength = d[first].Distance(d[last]);
		double epsilon = 1.0e-6 * segLength;
		if (alpha_l < epsilon || alpha_r < epsilon || danger)
		{
			// fall back on standard (probably inaccurate) formula, and subdivide further if needed. 
			double dist = segLength / 3.0;
			bezCurve[0] = d[first];
			bezCurve[3] = d[last];
			bezCurve[1] = bezCurve[0] + (tHat1*dist);
			bezCurve[2] = bezCurve[3] + (tHat2*dist);
			return; //bezCurve;
		}

		//  First and last control points of the Bezier curve are
		//  positioned exactly at the first and last data points 
		//  Control points 1 and 2 are positioned an alpha distance out
		//  on the tangent vectors, left and right, respectively
		bezCurve[0] = d[first];
		bezCurve[3] = d[last];
		bezCurve[1] = bezCurve[0] + (tHat1*alpha_l);
		bezCurve[2] = bezCurve[3] + (tHat2*alpha_r);
	}

	// NewtonRaphsonRootFind :
	//  Use Newton-Raphson iteration to find better root.
	static double newtonRaphsonRootFind(TVector2d const* Q, TVector2d const* Q1, TVector2d const* Q2, TVector2d P, double u)
	{
		// Compute Q(u)
		TVector2d Q_u = bezierII3(Q, u);

		// Compute Q'(u) and Q''(u)	
		TVector2d Q1_u = bezierII2(Q1, u);
		TVector2d Q2_u = bezierII1(Q2, u);

		// Compute f(u)/f'(u) 
		double denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
			(Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
		if (denominator == 0.0) return u;
		double numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);

		// u = u - f(u)/f'(u) // Improved u
		return u - (numerator / denominator);
	}

	static void computeQ1Q2(TVector2d const* Q, TVector2d* Q1, TVector2d* Q2)
	{
		// Generate control vertices for Q'
		for (int i = 0; i <= 2; i++)
		{
			Q1[i].x = (Q[i + 1].x - Q[i].x) * 3.0;
			Q1[i].y = (Q[i + 1].y - Q[i].y) * 3.0;
		}

		// Generate control vertices for Q'' 
		for (int i = 0; i <= 1; i++)
		{
			Q2[i].x = (Q1[i + 1].x - Q1[i].x) * 2.0;
			Q2[i].y = (Q1[i + 1].y - Q1[i].y) * 2.0;
		}
	}

	// Reparameterize:
	//  Given set of points and their parameterization, try to find
	//  a better parameterization.
	static void reparameterize(std::vector<TVector2d> const& d, int first, int last, double const* u, double* uPrime, TVector2d* bezCurve)
	{ 
		TVector2d Q1[3];
		TVector2d Q2[2]; //  Q' and Q''
		computeQ1Q2(bezCurve, Q1, Q2);

		for (int i = first; i <= last; ++i)
			uPrime[i - first] = newtonRaphsonRootFind(bezCurve, Q1, Q2, d[i], u[i - first]);
	}


/*
*  Bezier :
*  	Evaluate a Bezier curve at a particular parameter value
*/

//TVector2d bezierII(int degree, TVector2d const* V, double t)
//{
//	TVector2d Q;  // Point on curve at parameter t	
//	TVector2d *Vtemp; // Local copy of control points
//
//	// copy array
//	Vtemp = new cv::Vec2d[degree + 1];
//	for (int i = 0; i <= degree; ++i)
//	{
//		Vtemp[i] = V[i];
//	}
//	// Triangle computation	
//	for (int i = 1; i <= degree; i++) {
//		for (int j = 0; j <= degree - i; j++) {
//			Vtemp[j][0] = (1.0 - t) * Vtemp[j][0] + t * Vtemp[j + 1][0];
//			Vtemp[j][1] = (1.0 - t) * Vtemp[j][1] + t * Vtemp[j + 1][1];
//		}
//	}
//	Q = Vtemp[0];
//	return Q;
//}

/*
*  B0, B1, B2, B3 :
*	Bezier multipliers
*/

static double B0(double u)
{
	double tmp = 1.0 - u;
	return (tmp * tmp * tmp);
}
static double B1(double u)
{
	double tmp = 1.0 - u;
	return (3 * u * (tmp * tmp));
}

static double B2(double u)
{
	double tmp = 1.0 - u;
	return (3 * u * u * tmp);
}

static double B3(double u)
{
	return (u * u * u);
}

static TVector2d bezierII3(TVector2d const* V, double t)
{
	double const it = 1.0-t;
	double const w0 = it*it*it;
	double const w1 = 3*t*it*it;
	double const w2 = 3*t*t*it;
	double const w3 = t*t*t;
	return TVector2d(V[0].x*w0 + V[1].x*w1 + V[2].x*w2 + V[3].x*w3, V[0].y*w0 + V[1].y*w1 + V[2].y*w2 + V[3].y*w3);
}

static TVector2d bezierII2(TVector2d const* V, double t)
{
	double const it = 1.0-t;
	double const w0 = it*it;
	double const w1 = 2*t*it;
	double const w2 = t*t;
	return TVector2d(V[0].x*w0 + V[1].x*w1 + V[2].x*w2, V[0].y*w0 + V[1].y*w1 + V[2].y*w2);
}

static TVector2d bezierII1(TVector2d const* V, double t)
{
	double const it = 1.0-t;
	return TVector2d(V[0].x*it + V[1].x*t, V[0].y*it + V[1].y*t);
}



//	{
/**
 * fitCurves.js - Piecewise cubic fitting code
 *
 * original: fitCurves.c
 * http://tog.acm.org/resources/GraphicsGems/gems/fitCurves.c
 *
 * ported by ynakajima (https://github.com/ynakajima).
 *
 * THIS SOURCE CODE IS PUBLIC DOMAIN, and
 * is freely available to the entire computer graphics community
 * for study, use, and modification.  We do request that the
 * comment at the top of each file, identifying the original
 * author and its original publication in the book Graphics
 * Gems, be retained in all programs that use these files.
 *
 */
/**
  An Algorithm for Automatically Fitting Digitized Curves
  by Philip J. Schneider
  from "Graphics Gems", Academic Press, 1990
*/
//(function() {
//  var bezierSegments = [];
//
//  if (typeof module !== 'undefined') {
//    module.exports = polyline2bezier;
//  } else {
//    window.polyline2bezier = polyline2bezier;
//  }
//
//	static void Decimate2(std::vector<TVector2d> const& input, double scale, std::vector<TVector2d>& output)
//	{
//	}
//  function polyline2bezier(polyline, error) {
//    var d = [];
//    error = error || 4.0;    // Squared error
//    bezierSegments = [];
//    polyline.forEach(function(point) {
//      d.push(new Point2(point[0], point[1]));
//    });
//    fitCurve(d, d.length, error); // Fit the Bezier curves
//    return bezierSegments;
//  }
//
//  /**
//   * drawBezierCurve
//   * @param {Number} n
//   * @param {Array.<Point2>} curve
//   */
//  function drawBezierCurve(n, curve) {
//    bezierSegments.push(curve);
//  } 
//
//  /**
//   * fitCurve :
//   * Fit a Bezier curve to a set of digitized points 
//   * @param {Array.<Point2>} d Array of digitized points.
//   * @param {Number} nPts Number of digitized points
//   * @param {Number} error  User-defined error squared
//   */
//  function fitCurve(d, nPts, error) {
//    /*  Unit tangent vectors at endpoints */
//    var tHat1 = new Vector2(), tHat2 = new Vector2();  
//
//    tHat1 = computeLeftTangent(d, 0);
//    tHat2 = computeRightTangent(d, nPts - 1);
//    fitCubic(d, 0, nPts - 1, tHat1, tHat2, error);
//  }
//
//  /**
//   * fitCubic :
//   * Fit a Bezier curve to a (sub)set of digitized points
//   * @param {Array.<Point2>} d Array of digitized points
//   * @param {Number} first Indices of first pts in region
//   * @param {Number} last Indices of last pts in region
//   * @param {Point2} tHat1 Unit tangent vectors at endpoints
//   * @param {Point2} tHat2 Unit tangent vectors at endpoints
//   * @param {Number} error User-defined error squared
//   */
//  function fitCubic(d, first, last, tHat1, tHat2, error) {
//    var bezCurve,           /*Control points of fitted Bezier curve*/
//        u = [],             /*  Parameter values for point  */
//        uPrime = [],        /*  Improved parameter values */
//        maxError,           /*  Maximum fitting error   */
//        splitPoint,         /*  Point to split point set at   */
//        nPts,               /*  Number of points in subset  */
//        iterationError,     /*Error below which you try iterating  */
//        maxIterations = 4,  /*  Max times to try iterating  */
//        tHatCenter = new Vector2(),/* Unit tangent vector at splitPoint */
//        i;
//
//    iterationError = error * error;
//    nPts = last - first + 1;
//
//    /*  Use heuristic if region only has two points in it */
//    if (nPts === 2) {
//      var dist = v2DistanceBetween2Points(d[last], d[first]) / 3.0;
//
//      bezCurve = [];
//      bezCurve[0] = d[first];
//      bezCurve[3] = d[last];
//      tHat1 = v2Scale(tHat1, dist);
//      tHat2 = v2Scale(tHat2, dist);
//      bezCurve[1] = v2Add(bezCurve[0], tHat1);
//      bezCurve[2] = v2Add(bezCurve[3], tHat2);
//      drawBezierCurve(3, bezCurve);
//      return;
//    }
//
//    /*  Parameterize points, and attempt to fit curve */
//    u = chordLengthParameterize(d, first, last);
//    bezCurve = generateBezier(d, first, last, u, tHat1, tHat2);
//
//    /*  Find max deviation of points to fitted curve */
//    var resultMaxError =
//      computeMaxError(d, first, last, bezCurve, u, splitPoint);
//    maxError = resultMaxError.maxError;
//    splitPoint = resultMaxError.splitPoint;
//
//    if (maxError < error) {
//      drawBezierCurve(3, bezCurve);
//      return;
//    }
//
//    /*  If error not too large, try some reparameterization  */
//    /*  and iteration */
//    if (maxError < iterationError) {
//      for (i = 0; i < maxIterations; i++) {
//        uPrime = reparameterize(d, first, last, u, bezCurve);
//        bezCurve = generateBezier(d, first, last, uPrime, tHat1, tHat2);
//        resultMaxError = computeMaxError(d, first, last,
//               bezCurve, uPrime, splitPoint);
//        maxError = resultMaxError.maxError;
//        splitPoint = resultMaxError.splitPoint;
//        if (maxError < error) {
//          drawBezierCurve(3, bezCurve);
//          return;
//        }
//        u = uPrime;
//      }
//    }
//
//    /* Fitting failed -- split at max error point and fit recursively */
//    tHatCenter = computeCenterTangent(d, splitPoint);
//    fitCubic(d, first, splitPoint, tHat1, tHatCenter, error);
//    tHatCenter = v2Negate(tHatCenter);
//    fitCubic(d, splitPoint, last, tHatCenter, tHat2, error);
//  }
//
//
//  /**
//   * generateBezier :
//   * Use least-squares method to find Bezier control points for region.
//   * @param {Array.<Point2>} d  Array of digitized points
//   * @param {Number} first Indices defining region
//   * @param {Number} last Indices defining region
//   * @param {Array.<Number>} uPrime Parameter values for region
//   * @param {Vector2} tHat1 Unit tangents at endpoints
//   * @param {Vector2} tHat2 Unit tangents at endpoints
//   * @return {Array.<Point2> BezierCurve
//   */
//  function generateBezier(d, first, last, uPrime, tHat1, tHat2)
//  {
//    var i,
//        A = [],              /* Precomputed rhs for eqn  */
//        nPts,                /* Number of pts in sub-curve */
//        C = [[], []],        /* Matrix C    */
//        X = [],              /* Matrix X      */
//        det_C0_C1,           /* Determinants of matrices  */
//        det_C0_X,
//        det_X_C1,
//        alpha_l,             /* Alpha values, left and right  */
//        alpha_r,
//        tmp = new Vector2(), /* Utility variable    */
//        bezCurve;            /* RETURN bezier curve ctl pts  */
//
//    bezCurve = [];
//    nPts = last - first + 1;
//
//
//    /* Compute the A's  */
//    for (i = 0; i < nPts; i++) {
//      var v1 = new Vector2(tHat1.x, tHat1.y),
//          v2 = new Vector2(tHat2.x, tHat2.y);
//      v1 = v2Scale(v1, B1(uPrime[i]));
//      v2 = v2Scale(v2, B2(uPrime[i]));
//      A[i] = [];
//      A[i][0] = v1;
//      A[i][1] = v2;
//    }
//
//    /* Create the C and X matrices  */
//    C[0][0] = 0.0;
//    C[0][1] = 0.0;
//    C[1][0] = 0.0;
//    C[1][1] = 0.0;
//    X[0]    = 0.0;
//    X[1]    = 0.0;
//
//    for (i = 0; i < nPts; i++) {
//      C[0][0] += v2Dot(A[i][0], A[i][0]);
//      C[0][1] += v2Dot(A[i][0], A[i][1]);
//      // C[1][0] += v2Dot(A[i][0], A[i][1]);
//      C[1][0] = C[0][1];
//      C[1][1] += v2Dot(A[i][1], A[i][1]);
//
//      tmp = v2SubII(d[first + i],
//            v2AddII(
//              v2ScaleIII(d[first], B0(uPrime[i])),
//            v2AddII(
//                v2ScaleIII(d[first], B1(uPrime[i])),
//                    v2AddII(
//                          v2ScaleIII(d[last], B2(uPrime[i])),
//                            v2ScaleIII(d[last], B3(uPrime[i]))))));
//
//      X[0] += v2Dot(A[i][0], tmp);
//      X[1] += v2Dot(A[i][1], tmp);
//    }
//
//    /* Compute the determinants of C and X  */
//    det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
//    det_C0_X  = C[0][0] * X[1]    - C[1][0] * X[0];
//    det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];
//
//    /* Finally, derive alpha values  */
//    alpha_l = (det_C0_C1 === 0) ? 0.0 : det_X_C1 / det_C0_C1;
//    alpha_r = (det_C0_C1 === 0) ? 0.0 : det_C0_X / det_C0_C1;
//
//    /* If alpha negative, use the Wu/Barsky heuristic (see text) */
//    /* (if alpha is 0, you get coincident control points that lead to
//     * divide by zero in any subsequent newtonRaphsonRootFind() call. */
//    var segLength = v2DistanceBetween2Points(d[last], d[first]);
//    var epsilon = 1.0e-6 * segLength;
//    if (alpha_l < epsilon || alpha_r < epsilon) {
//      /* fall back on standard (probably inaccurate) formula, */
//      /* and subdivide further if needed. */
//      var dist = segLength / 3.0;
//      bezCurve[0] = d[first];
//      bezCurve[3] = d[last];
//      bezCurve[1] = v2Add(bezCurve[0], v2Scale(tHat1, dist));
//      bezCurve[2] = v2Add(bezCurve[3], v2Scale(tHat2, dist));
//      return (bezCurve);
//    }
//
//    /*  First and last control points of the Bezier curve are */
//    /*  positioned exactly at the first and last data points */
//    /*  Control points 1 and 2 are positioned an alpha distance out */
//    /*  on the tangent vectors, left and right, respectively */
//    bezCurve[0] = d[first];
//    bezCurve[3] = d[last];
//    bezCurve[1] = v2Add(bezCurve[0], v2Scale(tHat1, alpha_l));
//    bezCurve[2] = v2Add(bezCurve[3], v2Scale(tHat2, alpha_r));
//    return (bezCurve);
//  }
//
//
//  /**
//   * reparameterize:
//   * Given set of points and their parameterization, try to find
//   * a better parameterization.
//   * @param {Array.<Point2>} d Array of digitized points
//   * @param {Number} first Indices defining region
//   * @param {Number} last Indices defining region
//   * @param {Array.<Number>} u Current parameter values
//   * @param {Array.<Point2>} bezCurve Current fitted curve
//   * @return {Number}
//   */
//  function reparameterize(d, first, last, u, bezCurve) {
//    //nPts = last - first + 1,
//    var i,
//        uPrime = [], /*  New parameter values  */
//        _bezCurve = [
//          new Point2(bezCurve[0].x, bezCurve[0].y),
//          new Point2(bezCurve[1].x, bezCurve[1].y),
//          new Point2(bezCurve[2].x, bezCurve[2].y),
//          new Point2(bezCurve[3].x, bezCurve[3].y)
//        ];
//
//    for (i = first; i <= last; i++) {
//      uPrime[i-first] =
//        newtonRaphsonRootFind(_bezCurve, d[i], u[i-first]);
//    }
//    return (uPrime);
//  }
//
//  /**
//   * newtonRaphsonRootFind :
//   * Use Newton-Raphson iteration to find better root.
//   * @param {Array.<Point2>} _Q Current fitted curve
//   * @param {Point2} _P Digitized point
//   * @param {Number} u Parameter value for "P"
//   * @return {Number}
//   */
//  function newtonRaphsonRootFind(_Q, _P, u) {
//    var numerator, denominator,
//        Q1 = [new Point2(), new Point2(), new Point2()], /*  Q' and Q'' */
//        Q2 = [new Point2(), new Point2()],
//        /*u evaluated at Q, Q', & Q'' */
//        Q_u = new Point2(), Q1_u = new Point2(), Q2_u = new Point2(), 
//        uPrime,    /*  Improved u */
//        i,
//        Q = [
//          new Point2(_Q[0].x, _Q[0].y),
//          new Point2(_Q[1].x, _Q[1].y),
//          new Point2(_Q[2].x, _Q[2].y),
//          new Point2(_Q[3].x, _Q[3].y),
//        ],
//        P = new Point2(_P.x, _P.y);
//    
//    /* Compute Q(u)  */
//    Q_u = bezierII(3, Q, u);
//    
//    /* Generate control vertices for Q'  */
//    for (i = 0; i <= 2; i++) {
//      Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
//      Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
//    }
//    
//    /* Generate control vertices for Q'' */
//    for (i = 0; i <= 1; i++) {
//      Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
//      Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
//    }
//    
//    /* Compute Q'(u) and Q''(u)  */
//    Q1_u = bezierII(2, Q1, u);
//    Q2_u = bezierII(1, Q2, u);
//    
//    /* Compute f(u)/f'(u) */
//    numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
//    denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
//              (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
//    if (denominator === 0.0) return u;
//
//    /* u = u - f(u)/f'(u) */
//    uPrime = u - (numerator/denominator);
//    return (uPrime);
//  }
//
//  /**
//   * Bezier :
//   * Evaluate a Bezier curve at a particular parameter value
//   * @param {Number} degree The degree of the bezier curve
//   * @param {Array.<Point2>} V Array of control points
//   * @param {Number} t Parametric value to find point for
//   * @return {Point2}
//   */
//  function bezierII(degree, V, t) {
//    var i, j,    
//        Q,        /* Point on curve at parameter t  */
//        Vtemp = [];    /* Local copy of control points    */
//
//    /* Copy array  */
//    for (i = 0; i <= degree; i++) {
//      Vtemp[i] = new Point2(V[i].x, V[i].y);
//    }
//
//    /* Triangle computation  */
//    for (i = 1; i <= degree; i++) {  
//      for (j = 0; j <= degree-i; j++) {
//        Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
//        Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
//      }
//    }
//
//    Q = new Point2(Vtemp[0].x, Vtemp[0].y);
//    return Q;
//  }
//
//  /*
//   *  B0, B1, B2, B3 :
//   *  Bezier multipliers
//   */
//  function B0(u)
//  {
//    var tmp = 1.0 - u;
//    return (tmp * tmp * tmp);
//  }
//
//
//  function B1(u)
//  {
//    var tmp = 1.0 - u;
//    return (3 * u * (tmp * tmp));
//  }
//
//  function B2(u)
//  {
//    var tmp = 1.0 - u;
//    return (3 * u * u * tmp);
//  }
//
//  function B3(u)
//  {
//    return (u * u * u);
//  }
//
//  /**
//   * computeLeftTangent, computeRightTangent, computeCenterTangent :
//   * Approximate unit tangents at endpoints and "center" of digitized curve
//   */
//  /**
//   * @param {Array.<Point2>} d Digitized points.
//   * @param {Number} end Index to "left" end of region.
//   * @return {Vector2}
//   */
//  function computeLeftTangent(d, end)
//  {
//    var  tHat1 = new Vector2();
//    tHat1 = v2SubII(d[end+1], d[end]);
//    tHat1 = v2Normalize(tHat1);
//    return tHat1;
//  }
//
//  /**
//   * @param {Array.<Point2>} d Digitized points.
//   * @param {Number} end Index to "right" end of region.
//   * @return {Vector2}
//   */
//  function computeRightTangent(d, end)
//  {
//    var  tHat2 = new Vector2();
//    tHat2 = v2SubII(d[end-1], d[end]);
//    tHat2 = v2Normalize(tHat2);
//    return tHat2;
//  }
//
//  /**
//   * @param {Array.<Point2>} d Digitized points.
//   * @param {Number} end Index to point inside region.
//   * @return {Vector2}
//   */
//  function computeCenterTangent(d, center)
//  {
//    var V1 = new Vector2(), V2 = new Vector2(), tHatCenter = new Vector2();
//
//    V1 = v2SubII(d[center-1], d[center]);
//    V2 = v2SubII(d[center], d[center+1]);
//    tHatCenter.x = (V1.x + V2.x)/2.0;
//    tHatCenter.y = (V1.y + V2.y)/2.0;
//    tHatCenter = v2Normalize(tHatCenter);
//    return tHatCenter;
//  }
//
//  /**
//   * chordLengthParameterize :
//   * Assign parameter values to digitized points 
//   * using relative distances between points.
//   * @param {Array.<Point2>} d Array of digitized points
//   * @param {Number} first Indices defining region
//   * @param {Number} last Indices defining region
//   * @return {Number}
//   */
//  function chordLengthParameterize(d, first, last)
//  {
//    var i, 
//        u;      /*  Parameterization    */
//
//    u = []; 
//
//    u[0] = 0.0;
//    for (i = first+1; i <= last; i++) {
//      u[i-first] = u[i-first-1] +
//        v2DistanceBetween2Points(d[i], d[i-1]);
//    }
//
//    for (i = first + 1; i <= last; i++) {
//      u[i-first] = u[i-first] / u[last-first];
//    }
//    return u;
//  }
//
//  /**
//   * computeMaxError :
//   * Find the maximum squared distance of digitized points
//   * to fitted curve.
//   * @param {Array.<Point2>} d Array of digitized points
//   * @param {Number} first Indices defining region
//   * @param {Number} last Indices defining region
//   * @param {Array.<Point2>} bezCurve Fitted Bezier curve
//   * @param {Array.<Number>} u Parameterization of points
//   * @param {Number} splitPoint Point of maximum error
//   */
//  function computeMaxError(d, first, last, bezCurve, u, splitPoint)
//  {
//    var i,
//        maxDist,                /*  Maximum error    */
//        dist,                   /*  Current error    */
//        P = new Point2(),       /*  Point on curve    */
//        v = new Vector2();      /*  Vector from point to curve  */
//
//    splitPoint = (last - first + 1)/2;
//    maxDist = 0.0;
//    for (i = first + 1; i < last; i++) {
//      P = bezierII(3, bezCurve, u[i-first]);
//      v = v2SubII(P, d[i]);
//      dist = v2SquaredLength(v);
//      if (dist >= maxDist) {
//          maxDist = dist;
//          splitPoint = i;
//      }
//    }
//    return {maxError: maxDist, splitPoint: splitPoint};
//  }
//
//  function v2AddII(a, b)
//  {
//    var c = new Vector2();
//    c.x = a.x + b.x;  c.y = a.y + b.y;
//    return (c);
//  }
//  function v2ScaleIII(v, s)
//  {
//    var result = new Vector2();
//    result.x = v.x * s; result.y = v.y * s;
//    return (result);
//  }
//
//  function v2SubII(a, b)
//  {
//    var  c = new Vector2();
//    c.x = a.x - b.x; c.y = a.y - b.y;
//    return c;
//  }
//
//  // include "GraphicsGems.h"          
//  /* 
//   * GraphicsGems.h  
//   * Version 1.0 - Andrew Glassner
//   * from "Graphics Gems", Academic Press, 1990
//   */
//
//  /*********************/
//  /* 2d geometry types */
//  /*********************/
//
//  function Point2Struct(x, y) {  /* 2d point */
//    this.x = typeof x === 'number' ? x : 0;
//    this.y = typeof y === 'number' ? y : 0;
//  }
//  function Point2(x, y) {
//    Point2Struct.call(this, x, y);  
//  }
//  function Vector2(x, y) {
//    Point2Struct.call(this, x, y);  
//  }
//
//  /***********************/
//  /* two-argument macros */
//  /***********************/
//  /* 
//  2d and 3d Vector C Library 
//  by Andrew Glassner
//  from "Graphics Gems", Academic Press, 1990
//  */
//
//  /******************/
//  /*   2d Library   */
//  /******************/
//
//  /* returns squared length of input vector */  
//  function v2SquaredLength(a) 
//  {
//    return((a.x * a.x) + (a.y * a.y));
//  }
//    
//  /* returns length of input vector */
//  function v2Length(a) 
//  {
//    return(Math.sqrt(v2SquaredLength(a)));
//  }
//    
//  /* negates the input vector and returns it */
//  function v2Negate(v) 
//  {
//    var result = new Point2();
//    result.x = -v.x;  result.y = -v.y;
//    return(result);
//  }
//
//  /* normalizes the input vector and returns it */
//  function v2Normalize(v) 
//  {
//    var result = new Point2(),
//        len = v2Length(v);
//    if (len !== 0.0) { result.x = v.x / len;  result.y = v.y / len; }
//    return(result);
//  }
//
//
//  /* scales the input vector to the new length and returns it */
//  function v2Scale(v, newlen) 
//  {
//    var result = new Point2(),
//        len = v2Length(v);
//    if (len !== 0.0) {
//      result.x = v.x * newlen/len;   result.y = v.y * newlen/len;
//    }
//    return(result);
//  }
//
//  /* return vector sum c = a+b */
//  function v2Add(a, b)
//  {
//    var c = new Point2();
//    c.x = a.x + b.x;  c.y = a.y + b.y;
//    return(c);
//  }
//    
//  /* return the dot product of vectors a and b */
//  function v2Dot(a, b) 
//  {
//    return((a.x * b.x) + (a.y * b.y));
//  }
//
//  /* return the distance between two points */
//  function v2DistanceBetween2Points(a, b)
//  {
//    var dx = a.x - b.x;
//    var dy = a.y - b.y;
//    return(Math.sqrt((dx*dx)+(dy*dy)));
//  }
//})();
//	}
	static void ToBeziers(std::vector<TVector2d> const& input, std::vector<TVector2d>& output)
	{
		if (input.size() < 2)
			return;
			
		output.push_back(input[0]);
		TVector2d vec1 = input[1]-input[0];
		double l1 = vec1.Length();
		vec1.x /= l1;
		vec1.y /= l1;
		output.push_back(TVector2d(input[0].x+vec1.x*l1*0.3, input[0].y+vec1.y*l1*0.3));
		
		for (size_t i = 1; i < input.size()-1; ++i)
		{
			vec1 = input[i+1]-input[i-1];
			double n = 1.0/vec1.Length();
			vec1.x *= n;
			vec1.y *= n;
			double l1 = (input[i]-input[i-1]).Length();
			double l2 = (input[i+1]-input[i]).Length();
			output.push_back(TVector2d(input[i].x-vec1.x*l1*0.3, input[i].y-vec1.y*l1*0.3));
			output.push_back(input[i]);
			output.push_back(TVector2d(input[i].x+vec1.x*l2*0.3, input[i].y+vec1.y*l2*0.3));
		}

		vec1 = input[input.size()-2]-input[input.size()-1];
		l1 = vec1.Length();
		vec1.x /= l1;
		vec1.y /= l1;
		output.push_back(TVector2d(input[input.size()-1].x+vec1.x*l1*0.3, input[input.size()-1].y+vec1.y*l1*0.3));
		output.push_back(input[input.size()-1]);
	}
};