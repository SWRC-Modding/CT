/*=============================================================================
	UnMath.h: Unreal math routines
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
=============================================================================*/

/*-----------------------------------------------------------------------------
	Defintions.
-----------------------------------------------------------------------------*/

// Forward declarations.
class  FVector;
class  FPlane;
class  FCoords;
class  FRotator;
class  FScale;
class  FGlobalMath;
class  FMatrix;
class  FQuat;

// Fixed point conversion.
inline	INT Fix		(INT A)			{return A<<16;};
inline	INT Fix		(FLOAT A)		{return (INT)(A*65536.0f);};
inline	INT Unfix	(INT A)			{return A>>16;};

// Constants.
#undef  PI
#define PI 					(3.1415926535897932)
#define SMALL_NUMBER		(1.e-8)
#define KINDA_SMALL_NUMBER	(1.e-4)

// Aux constants.
#define INV_PI			(0.31830988618)
#define HALF_PI			(1.57079632679)

// Magic numbers for numerical precision.
#define DELTA			(0.00001f)
#define SLERP_DELTA		(0.0001f)

/*-----------------------------------------------------------------------------
	Global functions.
-----------------------------------------------------------------------------*/

// Snap a value to the nearest grid multiple.
inline FLOAT FSnap(FLOAT Location, FLOAT Grid)
{
	if(Grid==0.0f)	return Location;
	else			return appFloor((Location + 0.5*Grid)/Grid)*Grid;
}

// Internal sheer adjusting function so it snaps nicely at 0 and 45 degrees.
inline FLOAT FSheerSnap (FLOAT Sheer)
{
	if		(Sheer < -0.65f) return Sheer + 0.15f;
	else if (Sheer > +0.65f) return Sheer - 0.15f;
	else if (Sheer < -0.55f) return -0.50f;
	else if (Sheer > +0.55f) return 0.50f;
	else if (Sheer < -0.05f) return Sheer + 0.05f;
	else if (Sheer > +0.05f) return Sheer - 0.05f;
	else					 return 0.0f;
}

// Find the closest power of 2 that is >= N.
inline DWORD FNextPowerOfTwo(DWORD N)
{
	if (N<=0L		) return 0L;
	if (N<=1L		) return 1L;
	if (N<=2L		) return 2L;
	if (N<=4L		) return 4L;
	if (N<=8L		) return 8L;
	if (N<=16L		) return 16L;
	if (N<=32L		) return 32L;
	if (N<=64L		) return 64L;
	if (N<=128L		) return 128L;
	if (N<=256L		) return 256L;
	if (N<=512L		) return 512L;
	if (N<=1024L	) return 1024L;
	if (N<=2048L	) return 2048L;
	if (N<=4096L	) return 4096L;
	if (N<=8192L	) return 8192L;
	if (N<=16384L	) return 16384L;
	if (N<=32768L	) return 32768L;
	if (N<=65536L	) return 65536L;
	else			  return 0;
}

inline UBOOL FIsPowerOfTwo(DWORD N)
{
	return !(N & (N - 1));
}

/*
 * Add to a word angle, constraining it within a min (not to cross)
 * and a max (not to cross).  Accounts for funkyness of word angles.
 * Assumes that angle is initially in the desired range.
 */
inline _WORD FAddAngleConfined(INT Angle, INT Delta, INT MinThresh, INT MaxThresh)
{
	if(Delta < 0)
	{
		if (Delta<=-0x10000L || Delta<=-(INT)((_WORD)(Angle-MinThresh)))
			return MinThresh;
	}
	else if(Delta > 0)
	{
		if(Delta>=0x10000L || Delta>=(INT)((_WORD)(MaxThresh-Angle)))
			return MaxThresh;
	}
	return (_WORD)(Angle+Delta);
}

//Eliminate all fractional precision from an angle.
INT ReduceAngle(INT Angle);

/*
 * Fast 32-bit float evaluations.
 * Warning: likely not portable, and useful on Pentium class processors only.
 */

inline UBOOL IsSmallerPositiveFloat(float F1,float F2)
{
	return ((*(DWORD*)&F1) < (*(DWORD*)&F2));
}

inline FLOAT MinPositiveFloat(float F1, float F2)
{
	if ((*(DWORD*)&F1) < (*(DWORD*)&F2)) return F1; else return F2;
}

// Warning: 0 and -0 have different binary representations.

inline UBOOL EqualPositiveFloat(float F1, float F2)
{
	return (*(DWORD*)&F1 == *(DWORD*)&F2);
}

inline UBOOL IsNegativeFloat(float F1)
{
	return ((*(DWORD*)&F1) >= (DWORD)0x80000000); // Detects sign bit.
}

inline FLOAT MaxPositiveFloat(float F1, float F2)
{
	if ((*(DWORD*)&F1) < (*(DWORD*)&F2)) return F2; else return F1;
}

// Clamp F0 between F1 and F2, all positive assumed.
inline FLOAT ClampPositiveFloat(float F0, float F1, float F2)
{
	if      ((*(DWORD*)&F0) < (*(DWORD*)&F1)) return F1;
	else if ((*(DWORD*)&F0) > (*(DWORD*)&F2)) return F2;
	else return F0;
}

// Clamp any float F0 between zero and positive float Range
#define ClipFloatFromZero(F0,Range)\{\
	if ((*(DWORD*)&F0) >= (DWORD)0x80000000) F0 = 0.0f;\
	else if	((*(DWORD*)&F0) > (*(DWORD*)&Range)) F0 = Range;\
}

/*-----------------------------------------------------------------------------
	FVector.
-----------------------------------------------------------------------------*/

/*
 * Information associated with a floating point vector, describing its
 * status as a point in a rendering context.
 */
enum EVectorFlags{
	FVF_OutXMin		= 0x04,	// Outcode rejection, off left hand side of screen.
	FVF_OutXMax		= 0x08,	// Outcode rejection, off right hand side of screen.
	FVF_OutYMin		= 0x10,	// Outcode rejection, off top of screen.
	FVF_OutYMax		= 0x20,	// Outcode rejection, off bottom of screen.
	FVF_OutNear     = 0x40, // Near clipping plane.
	FVF_OutFar      = 0x80, // Far clipping plane.
	FVF_OutReject   = (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax), // Outcode rejectable.
	FVF_OutSkip		= (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax), // Outcode clippable.
};

/*
 * Floating point vector.
 */
class FVector{
public:
	// Variables.
	FLOAT X,Y,Z;

	// Constructors.
	FVector(){}
	FVector(FLOAT In) : X(In), Y(In), Z(In){}
	FVector(FLOAT InX, FLOAT InY, FLOAT InZ)
	:	X(InX), Y(InY), Z(InZ){}

	// Binary math operators.
	FVector operator^(const FVector& V) const{
		return FVector
		(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}
	FLOAT operator|(const FVector& V) const{
		return X*V.X + Y*V.Y + Z*V.Z;
	}
	friend FVector operator*(FLOAT Scale, const FVector& V)
	{
		return FVector(V.X * Scale, V.Y * Scale, V.Z * Scale);
	}
	FVector operator+(const FVector& V) const{
		return FVector(X + V.X, Y + V.Y, Z + V.Z);
	}
	FVector operator-(const FVector& V) const{
		return FVector(X - V.X, Y - V.Y, Z - V.Z);
	}
	FVector operator*(FLOAT Scale) const{
		return FVector(X * Scale, Y * Scale, Z * Scale);
	}
	FVector operator/(FLOAT Scale) const{
		FLOAT RScale = 1.0f/Scale;
		return FVector(X * RScale, Y * RScale, Z * RScale);
	}
	FVector operator*(const FVector& V) const{
		return FVector(X * V.X, Y * V.Y, Z * V.Z);
	}

	// Binary comparison operators.
	UBOOL operator==(const FVector& V) const{
		return X==V.X && Y==V.Y && Z==V.Z;
	}
	UBOOL operator!=(const FVector& V) const{
		return X!=V.X || Y!=V.Y || Z!=V.Z;
	}

	// Unary operators.
	FVector operator-() const{
		return FVector(-X, -Y, -Z);
	}

	// Assignment operators.
	FVector operator+=(const FVector& V)
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}
	FVector operator-=(const FVector& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}
	FVector operator*=(FLOAT Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}
	FVector operator/=(FLOAT V)
	{
		FLOAT RV = 1.0f/V;
		X *= RV; Y *= RV; Z *= RV;
		return *this;
	}
	FVector operator*=(const FVector& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z;
		return *this;
	}
	FVector operator/=(const FVector& V)
	{
		X /= V.X; Y /= V.Y; Z /= V.Z;
		return *this;
	}
    FLOAT& operator[](INT i)
    {
		checkSlow(i>-1);
		checkSlow(i<3);
		if(i == 0)		return X;
		else if(i == 1)	return Y;
		else				return Z;
	}

	// Simple functions.
	FLOAT GetMax() const{
		return Max(Max(X,Y),Z);
	}
	FLOAT GetAbsMax() const{
		return Max(Max(Abs(X),Abs(Y)),Abs(Z));
	}
	FLOAT GetMin() const{
		return Min(Min(X,Y),Z);
	}
	FLOAT Size() const{
		return appSqrt(X*X + Y*Y + Z*Z);
	}
	FLOAT SizeSquared() const{
		return X*X + Y*Y + Z*Z;
	}
	FLOAT Size2D() const {
		return appSqrt(X*X + Y*Y);
	}
	FLOAT SizeSquared2D() const {
		return X*X + Y*Y;
	}
	int IsNearlyZero() const{
		return
				Abs(X)<KINDA_SMALL_NUMBER
			&&	Abs(Y)<KINDA_SMALL_NUMBER
			&&	Abs(Z)<KINDA_SMALL_NUMBER;
	}
	UBOOL IsZero() const{
		return X==0.0f && Y==0.0f && Z==0.0f;
	}
	UBOOL Normalize()
	{
		FLOAT SquareSum = X*X+Y*Y+Z*Z;
		if(SquareSum >= SMALL_NUMBER)
		{
			FLOAT Scale = 1.0f/appSqrt(SquareSum);
			X *= Scale; Y *= Scale; Z *= Scale;
			return 1;
		}
		else return 0;
	}
	FVector GetNormalized()
	{
		FLOAT SquareSum = X*X+Y*Y+Z*Z;
		if(SquareSum >= SMALL_NUMBER)
		{
			FLOAT Scale = 1.0f/appSqrt(SquareSum);
			X *= Scale; Y *= Scale; Z *= Scale;
			return *this;
		}

		return FVector(0,0,0);
	}
	//Expects a unit vector and returns a vector that is sufficiently non parallel ;)
	FVector GetNonParallel()
	{
		// One of the components in a unit vector has to be > 0.57f [sqrt(1/3)].
		if (Abs(X) > 0.57f)
			return FVector(0,1,0);
		else if (Abs(Y) > 0.57f)
			return FVector(0,0,1);
		else
			return FVector(1,0,0);
	}
	FVector Projection() const{
		FLOAT RZ = 1.0f/Z;
		return FVector(X*RZ, Y*RZ, 1);
	}
	FVector UnsafeNormal() const{
		FLOAT Scale = 1.0f/appSqrt(X*X+Y*Y+Z*Z);

		return FVector(X*Scale, Y*Scale, Z*Scale);
	}
	FVector GridSnap(const FVector& Grid)
	{
		return FVector(FSnap(X, Grid.X),FSnap(Y, Grid.Y),FSnap(Z, Grid.Z));
	}
	FVector BoundToCube(FLOAT Radius)
	{
		return FVector
		(
			Clamp(X,-Radius,Radius),
			Clamp(Y,-Radius,Radius),
			Clamp(Z,-Radius,Radius)
		);
	}
	void AddBounded(const FVector& V, FLOAT Radius=MAXSWORD)
	{
		*this = (*this + V).BoundToCube(Radius);
	}
	FLOAT& Component(INT Index)
	{
		return (&X)[Index];
	}

	/*
	 *  Return a boolean that is based on the vector's direction.
	 *
	 * When      V==(0,0,0) Booleanize(0)=1.
	 * Otherwise Booleanize(V) <-> !Booleanize(!B).
	 */
	UBOOL Booleanize()
	{
		return
			X >  0.0f ? 1 :
			X <  0.0f ? 0 :
			Y >  0.0f ? 1 :
			Y <  0.0f ? 0 :
			Z >= 0.0f ? 1 : 0;
	}

	//See if X == Y == Z (within fairly small tolerance)
	UBOOL IsUniform()
	{
		return (Abs(X-Y) < KINDA_SMALL_NUMBER) && (Abs(Y-Z) < KINDA_SMALL_NUMBER);
	}

	// Transformation.
	FVector TransformVectorBy(const FCoords& Coords) const;
	FVector TransformPointBy(const FCoords& Coords) const;
	FVector MirrorByVector(const FVector& MirrorNormal) const;
	FVector MirrorByPlane(const FPlane& MirrorPlane) const;
	FVector PivotTransform(const FCoords& Coords) const;
	FVector TransformVectorByTranspose(const FCoords& Coords) const;

	// Complicated functions.
	FRotator Rotation();
	void FindBestAxisVectors(FVector& Axis1, FVector& Axis2);
	FVector SafeNormal() const; //warning: Not inline because of compiler bug.
	FVector RotateAngleAxis(const INT Angle, const FVector& Axis) const;

	// Friends.
	friend FLOAT FDist(const FVector& V1, const FVector& V2);
	friend FLOAT FDistSquared(const FVector& V1, const FVector& V2);
	friend bool FPointsAreSame(const FVector& P, const FVector& Q);
	friend bool FPointsAreNear(const FVector& Point1, const FVector& Point2, FLOAT Dist);
	friend FLOAT FPointPlaneDist(const FVector& Point, const FVector& PlaneBase, const FVector& PlaneNormal);
	friend FVector FLinePlaneIntersection(const FVector& Point1, const FVector& Point2, const FVector& PlaneOrigin, const FVector& PlaneNormal);
	friend FVector FLinePlaneIntersection(const FVector& Point1, const FVector& Point2, const FPlane& Plane);
	friend UBOOL FParallel(const FVector& Normal1, const FVector& Normal2);
	friend UBOOL FCoplanar(const FVector& Base1, const FVector& Normal1, const FVector& Base2, const FVector& Normal2);

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FVector& V)
	{
		return Ar << V.X << V.Y << V.Z;
	}
};

class ABrush;

/*
 * Used by the multiple vertex editing function to keep track of selected vertices.
 */
class CORE_API FVertexHit{
public:
	// Variables.
	ABrush* pBrush;
	INT PolyIndex;
	INT VertexIndex;

	// Constructors.
	FVertexHit()
	{
		pBrush = NULL;
		PolyIndex = VertexIndex = 0;
	}
	FVertexHit(ABrush* InBrush, INT InPolyIndex, INT InVertexIndex)
	{
		pBrush = InBrush;
		PolyIndex = InPolyIndex;
		VertexIndex = InVertexIndex;
	}

	// Functions.
	UBOOL operator==(const FVertexHit& V) const{
		return pBrush==V.pBrush && PolyIndex==V.PolyIndex && VertexIndex==V.VertexIndex;
	}
	UBOOL operator!=(const FVertexHit& V) const{
		return pBrush!=V.pBrush || PolyIndex!=V.PolyIndex || VertexIndex!=V.VertexIndex;
	}
};

/*-----------------------------------------------------------------------------
	FEdge.
-----------------------------------------------------------------------------*/

class CORE_API FEdge{
public:
	// Constructors.
	FEdge(){}
	FEdge(FVector v1, FVector v2)
	{
		Vertex[0] = v1;
		Vertex[1] = v2;
	}

	FVector Vertex[2];

	UBOOL operator==(const FEdge& E) const{
		return ((E.Vertex[0] == Vertex[0] && E.Vertex[1] == Vertex[1])
			|| (E.Vertex[0] == Vertex[1] && E.Vertex[1] == Vertex[0]));
	}
};

/*-----------------------------------------------------------------------------
	FPlane.
-----------------------------------------------------------------------------*/

class FPlane : public FVector{
public:
	// Variables.
	FLOAT W;

	// Constructors.
	FPlane(){}
	FPlane(const FPlane& P)
	:	FVector(P)
	,	W(P.W){}
	FPlane(const FVector& V)
	:	FVector(V)
	,	W(0){}
	FPlane(FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW)
	:	FVector(InX,InY,InZ)
	,	W(InW){}
	FPlane(FVector InNormal, FLOAT InW)
	:	FVector(InNormal), W(InW){}
	FPlane(FVector InBase, const FVector &InNormal)
	:	FVector(InNormal)
	,	W(InBase | InNormal){}
	FPlane(FVector A, FVector B, FVector C)
	:	FVector(((B-A)^(C-A)).SafeNormal())
	,	W(A | ((B-A)^(C-A)).SafeNormal()){}

	// Functions.
	FLOAT PlaneDot(const FVector &P) const{
		return X*P.X + Y*P.Y + Z*P.Z - W;
	}
	FPlane Flip() const{
		return FPlane(-X,-Y,-Z,-W);
	}
	FPlane TransformPlaneByOrtho(const FMatrix& M) const;
	FPlane TransformBy(const FMatrix& M) const;
	FPlane TransformByUsingAdjointT(const FMatrix& M, const FMatrix& TA) const;
	FPlane TransformPlaneByOrtho(const FCoords& Coords) const;
	FPlane TransformBy(const FCoords& Coords) const;
	UBOOL operator==(const FPlane& V) const{
		return X==V.X && Y==V.Y && Z==V.Z && W==V.W;
	}
	UBOOL operator!=(const FPlane& V) const{
		return X!=V.X || Y!=V.Y || Z!=V.Z || W!=V.W;
	}
	FPlane operator+(const FPlane& V) const{
		return FPlane(X + V.X, Y + V.Y, Z + V.Z, W + V.W);
	}
	FPlane operator-(const FPlane& V) const{
		return FPlane(X - V.X, Y - V.Y, Z - V.Z, W - V.W);
	}
	FPlane operator/(FLOAT Scale) const{
		FLOAT RScale = 1.0f/Scale;
		return FPlane(X * RScale, Y * RScale, Z * RScale, W * RScale);
	}
	FPlane operator*(FLOAT Scale) const{
		return FPlane(X * Scale, Y * Scale, Z * Scale, W * Scale);
	}
	FPlane operator*(const FPlane& V)
	{
		return FPlane (X*V.X,Y*V.Y,Z*V.Z,W*V.W);
	}
	FPlane operator+=(const FPlane& V)
	{
		X += V.X; Y += V.Y; Z += V.Z; W += V.W;
		return *this;
	}
	FPlane operator-=(const FPlane& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z; W -= V.W;
		return *this;
	}
	FPlane operator*=(FLOAT Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale; W *= Scale;
		return *this;
	}
	FPlane operator*=(const FPlane& V)
	{
		X *= V.X; Y *= V.Y; Z *= V.Z; W *= V.W;
		return *this;
	}
	FPlane operator/=(FLOAT V)
	{
		FLOAT RV = 1.0f/V;
		X *= RV; Y *= RV; Z *= RV; W *= RV;
		return *this;
	}
	FLOAT& operator[](INT i)
	{
		checkSlow(i < 4);
		checkSlow(i >= 0);
		switch(i)
		{
		case 0:
			return X;
		case 1:
			return Y;
		case 2:
			return Z;
		case 3:
			return W;
		default:
			return X;
		}
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FPlane &P)
	{
		return Ar << (FVector&)P << P.W;
	}
};

// gam ---
template<class T> struct TBox{
    T X1, Y1, X2, Y2;

    bool Test(T X, T Y) const
    {
        return ((X >= X1) && (X <= X2) && (Y >= Y1) && (Y <= Y2));
    }
};

typedef TBox<INT> FIntBox;
typedef TBox<FLOAT> FFloatBox;
// --- gam

/*-----------------------------------------------------------------------------
	FSphere.
-----------------------------------------------------------------------------*/

class FSphere : public FPlane{
public:
	// Constructors.
	FSphere(){}
	FSphere(INT) : FPlane(0,0,0,0){}
	FSphere(FVector V, FLOAT W) : FPlane(V, W){}
	FSphere(const FVector* Pts, INT Count);

	FSphere TransformBy(const FMatrix& M) const;

	friend FArchive& operator<<(FArchive& Ar, FSphere& S)
	{
		guardSlow(FSphere<<);
		if(Ar.Ver()<=61)//oldver
			Ar << (FVector&)S;
		else
			Ar << (FPlane&)S;
		return Ar;
		unguardSlow
	}
};

/*-----------------------------------------------------------------------------
	FScale.
-----------------------------------------------------------------------------*/

/*
 * An axis along which sheering is performed.
 */
enum ESheerAxis{
	SHEER_None = 0,
	SHEER_XY   = 1,
	SHEER_XZ   = 2,
	SHEER_YX   = 3,
	SHEER_YZ   = 4,
	SHEER_ZX   = 5,
	SHEER_ZY   = 6,
};

/*
 * Scaling and sheering info associated with a brush.
 *
 * This is easily-manipulated information which is built into a transformation matrix later.
 */
class CORE_API FScale {
public:
	// Variables.
	FVector		Scale;
	FLOAT		SheerRate;
	BYTE		SheerAxis; // From ESheerAxis

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FScale &S)
	{
		return Ar << S.Scale << S.SheerRate << S.SheerAxis;
	}

	// Constructors.
	FScale() {}
	FScale(const FVector &InScale, FLOAT InSheerRate, ESheerAxis InSheerAxis)
	:	Scale(InScale), SheerRate(InSheerRate), SheerAxis(InSheerAxis) {}

	// Operators.
	UBOOL operator==(const FScale &S) const{
		return Scale==S.Scale && SheerRate==S.SheerRate && SheerAxis==S.SheerAxis;
	}

	// Functions.
	FLOAT  Orientation()
	{
		return Sgn(Scale.X * Scale.Y * Scale.Z);
	}
};

/*-----------------------------------------------------------------------------
	FCoords.
-----------------------------------------------------------------------------*/

/*
 * A coordinate system matrix.
 */
class CORE_API FCoords{
public:
	FVector	Origin;
	FVector	XAxis;
	FVector YAxis;
	FVector ZAxis;

	// Constructors.
	FCoords() {}
	FCoords(const FVector &InOrigin)
	:	Origin(InOrigin), XAxis(1,0,0), YAxis(0,1,0), ZAxis(0,0,1) {}
	FCoords(const FVector &InOrigin, const FVector &InX, const FVector &InY, const FVector &InZ)
	:	Origin(InOrigin), XAxis(InX), YAxis(InY), ZAxis(InZ) {}

	// Functions.
	FCoords MirrorByVector(const FVector& MirrorNormal) const;
	FCoords MirrorByPlane(const FPlane& MirrorPlane) const;
	FCoords	Transpose() const;
	FCoords Inverse() const;
	FCoords PivotInverse() const;
	FCoords ApplyPivot(const FCoords& CoordsB) const;
	FRotator OrthoRotation() const;
	FMatrix Matrix() const;

	// Operators.
	FCoords& operator*=	(const FCoords   &TransformCoords);
	FCoords	 operator*	(const FCoords   &TransformCoords) const;
	FCoords& operator*=	(const FVector   &Point);
	FCoords  operator*	(const FVector   &Point) const;
	FCoords& operator*=	(const FRotator  &Rot);
	FCoords  operator*	(const FRotator  &Rot) const;
	FCoords& operator*=	(const FScale    &Scale);
	FCoords  operator*	(const FScale    &Scale) const;
	FCoords& operator/=	(const FVector   &Point);
	FCoords  operator/	(const FVector   &Point) const;
	FCoords& operator/=	(const FRotator  &Rot);
	FCoords  operator/	(const FRotator  &Rot) const;
	FCoords& operator/=	(const FScale    &Scale);
	FCoords  operator/	(const FScale    &Scale) const;

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FCoords& F)
	{
		return Ar << F.Origin << F.XAxis << F.YAxis << F.ZAxis;
	}
};

/*-----------------------------------------------------------------------------
	FModelCoords.
-----------------------------------------------------------------------------*/

/*
 * A model coordinate system, describing both the covariant and contravariant
 * transformation matrices to transform points and normals by.
 */
class CORE_API FModelCoords{
public:
	// Variables.
	FCoords PointXform;		// Coordinates to transform points by  (covariant).
	FCoords VectorXform;	// Coordinates to transform normals by (contravariant).

	// Constructors.
	FModelCoords(){}
	FModelCoords(const FCoords& InCovariant, const FCoords& InContravariant)
	:	PointXform(InCovariant), VectorXform(InContravariant){}

	// Functions.
	FModelCoords Inverse()
	{
		return FModelCoords(VectorXform.Transpose(), PointXform.Transpose());
	}
};

/*-----------------------------------------------------------------------------
	FRotator.
-----------------------------------------------------------------------------*/

/*
 * Rotation.
 */
class CORE_API FRotator{
public:
	// Variables.
	INT Pitch; // Looking up and down (0=Straight Ahead, +Up, -Down).
	INT Yaw;   // Rotating around (running in circles), 0=East, +North, -South.
	INT Roll;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FRotator& R)
	{
		return Ar << R.Pitch << R.Yaw << R.Roll;
	}

	// Constructors.
	FRotator(){}
	FRotator(INT In) : Pitch(In), Roll(In), Yaw(In){}
	FRotator(INT InPitch, INT InYaw, INT InRoll)
	:	Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}

	// Binary arithmetic operators.
	FRotator operator+(const FRotator &R) const{
		return FRotator(Pitch+R.Pitch, Yaw+R.Yaw, Roll+R.Roll);
	}
	FRotator operator-(const FRotator &R) const{
		return FRotator(Pitch-R.Pitch, Yaw-R.Yaw, Roll-R.Roll);
	}
	FRotator operator*(FLOAT Scale) const{
		return FRotator((INT)(Pitch*Scale), (INT)(Yaw*Scale), (INT)(Roll*Scale));
	}
	friend FRotator operator*(FLOAT Scale, const FRotator &R)
	{
		return FRotator((INT)(R.Pitch*Scale), (INT)(R.Yaw*Scale), (INT)(R.Roll*Scale));
	}
	FRotator operator*= (FLOAT Scale)
	{
		Pitch = (INT)(Pitch*Scale); Yaw = (INT)(Yaw*Scale); Roll = (INT)(Roll*Scale);
		return *this;
	}
	// Binary comparison operators.
	UBOOL operator==(const FRotator &R) const{
		return Pitch==R.Pitch && Yaw==R.Yaw && Roll==R.Roll;
	}
	UBOOL operator!=(const FRotator &V) const{
		return Pitch!=V.Pitch || Yaw!=V.Yaw || Roll!=V.Roll;
	}
	// Assignment operators.
	FRotator operator+=(const FRotator &R)
	{
		Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
		return *this;
	}
	FRotator operator-=(const FRotator &R)
	{
		Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
		return *this;
	}
	// Functions.
	FRotator Reduce() const{
		return FRotator(ReduceAngle(Pitch), ReduceAngle(Yaw), ReduceAngle(Roll));
	}
	int IsZero() const{
		return ((Pitch&65535)==0) && ((Yaw&65535)==0) && ((Roll&65535)==0);
	}
	FRotator Add(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
		Yaw   += DeltaYaw;
		Pitch += DeltaPitch;
		Roll  += DeltaRoll;
		return *this;
	}
	FRotator AddBounded(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll)
	{
		Yaw  += DeltaYaw;
		Pitch = FAddAngleConfined(Pitch,DeltaPitch,192*0x100,64*0x100);
		Roll  = FAddAngleConfined(Roll, DeltaRoll, 192*0x100,64*0x100);
		return *this;
	}
	FRotator GridSnap(const FRotator &RotGrid)
	{
		return FRotator
		(
			(INT)(FSnap(Pitch,RotGrid.Pitch)),
			(INT)(FSnap(Yaw,  RotGrid.Yaw)),
			(INT)(FSnap(Roll, RotGrid.Roll))
		);
	}
	FVector Vector();
	//Resets the rotation values so they fall within the range -65535,65535
	FRotator Clamp()
	{
		guard(FRotator::Clamp);
		return FRotator(Pitch%65535, Yaw%65535, Roll%65535);
		unguard;
	}
	FRotator ClampPos()
	{
		guard(FRotator::Clamp);
		return FRotator(Abs(Pitch)%65535, Abs(Yaw)%65535, Abs(Roll)%65535);
		unguard;
	}
};

/*-----------------------------------------------------------------------------
	FPosition.
-----------------------------------------------------------------------------*/

/*
 * A convenience class for keeping track of positions.
 */
class CORE_API FPosition{
public:
	// Variables.
	FVector Location;
	FCoords Coords;

	// Constructors.
	FPosition(){}
	FPosition(FVector InLocation, FCoords InCoords)
	:	Location(InLocation), Coords(InCoords){}
};

/*-----------------------------------------------------------------------------
	FRange.
-----------------------------------------------------------------------------*/

/*
 * Floating point range. Aaron Leiby
 *
 * - changed to Min/Max: vogel
 */
class CORE_API FRange {
public:
	// Variables.
	FLOAT Min, Max;

	// Constructors.
	FRange(){}
	FRange(FLOAT InMin, FLOAT InMax)
	{
		Min = ::Min(InMin, InMax);
		Max = ::Max(InMin, InMax);
	}
	FRange(FLOAT Value) {
		Min = Value;
		Max = Value;
	}
	// Binary math operators.
	friend FRange operator*(FLOAT Scale, const FRange& R)
	{
		return FRange(R.Min * Scale, R.Max * Scale);
	}
	FRange operator+(const FRange& R) const{
		return FRange(Min + R.Min, Max + R.Max);
	}
	FRange operator+(FLOAT V) const{
		return FRange(Min + V, Max + V);
	}
	FRange operator-(FLOAT V) const{
		return FRange(Min - V, Max - V);
	}
	FRange operator-(const FRange& R) const{
		return FRange(Min - R.Min, Max - R.Max);
	}
	FRange operator*(FLOAT Scale) const{
		return FRange(Min * Scale, Max * Scale);
	}
	FRange operator/(FLOAT Scale) const{
		FLOAT RScale = 1.0/Scale;
		return FRange(Min * RScale, Max * RScale);
	}
	FRange operator*(const FRange& R) const{
		return FRange(Min * R.Min, Max * R.Max);
	}

	// Binary comparison operators.
	UBOOL operator==(const FRange& R) const{
		return Min==R.Min && Max==R.Max;
	}
	UBOOL operator!=(const FRange& R) const{
		return Min!=R.Min || Max!=R.Max;
	}

	// Unary operators.
	FRange operator-() const{
		return FRange(-Min, -Max);
	}

	// Assignment operators.
	FRange operator+=(const FRange& R)
	{
		Min += R.Min; Max += R.Max;
		return *this;
	}
	FRange operator-=(const FRange& R)
	{
		Min -= R.Min; Max -= R.Max;
		return *this;
	}
	FRange operator+=(FLOAT V)
	{
		Min += V; Max += V;
		return *this;
	}
	FRange operator-=(FLOAT V)
	{
		Min -= V; Max -= V;
		return *this;
	}
	FRange operator*=(FLOAT Scale)
	{
		Min *= Scale; Max *= Scale;
		return *this;
	}
	FRange operator/=(FLOAT Scale)
	{
		FLOAT RScale = 1.0/Scale;
		Min *= RScale; Max *= RScale;
		return *this;
	}
	FRange operator*=(const FRange& R)
	{
		Min *= R.Min; Max *= R.Max;
		return *this;
	}
	FRange operator/=(const FRange& R)
	{
		Min /= R.Min; Max /= R.Max;
		return *this;
	}

	// Simple functions.
	FLOAT GetMax() const{
		return ::Max(Min,Max);
	}
	FLOAT GetMin() const{
		return ::Min(Min,Max);
	}
	FLOAT Size() const{
		return GetMax() - GetMin();
	}
	FLOAT GetCenter() const{
		return (Max + Min) / 2.0f;
	}
	FLOAT GetRand() const{
		return Max + (Min - Max) * appFrand();	// order is irrelevant since appFrand() is equally distributed between 0 and 1.
	}
	FLOAT GetSRand() const{
		return Max + (Min - Max) * appSRand();
	}
#if 0
	INT GetRandInt() const{
		return appRandRange((INT)Min, (INT)Max);
	}
#endif
	int IsNearlyZero() const{
		return
				Abs(Min)<KINDA_SMALL_NUMBER
			&&	Abs(Max)<KINDA_SMALL_NUMBER;
	}
	UBOOL IsZero() const{
		return Min==0.0 && Max==0.0;
	}
	FRange GridSnap(const FRange& Grid)
	{
		return FRange(FSnap(Min, Grid.Min),FSnap(Max, Grid.Max));
	}
	FLOAT& Component(INT Index)
	{
		return (&Min)[Index];
	}

	/*
	 * When      R==(0.0) Booleanize(0)=1.
	 * Otherwise Booleanize(R) <-> !Booleanize(!R).
	 */
	UBOOL Booleanize()
	{
		return
			Min >  0.0 ? 1 :
			Min <  0.0 ? 0 :
			Max >= 0.0 ? 1 : 0;
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FRange& R)
	{
		return Ar << R.Min << R.Max;
	}
};

/*-----------------------------------------------------------------------------
	FRangeVector.
-----------------------------------------------------------------------------*/

/*
 * Vector of floating point ranges.
 */
class CORE_API FRangeVector{
public:
	// Variables.
	FRange X, Y, Z;

	// Constructors.
	FRangeVector(){}
	FRangeVector(FRange InX, FRange InY, FRange InZ)
	:	X(InX), Y(InY), Z(InZ){}
	FRangeVector(FVector V)
	:	X(V.X), Y(V.Y), Z(V.Z){}
	// Binary math operators.
	friend FRangeVector operator*(FLOAT Scale, const FRangeVector& R)
	{
		return FRangeVector(R.X * Scale, R.Y * Scale, R.Z * Scale);
	}
	FRangeVector operator+(const FRangeVector& R) const{
		return FRangeVector(X + R.X, Y + R.Y, Z + R.Z);
	}
	FRangeVector operator-(const FRangeVector& R) const{
		return FRangeVector(X - R.X, Y - R.Y, Z - R.Z);
	}
	FRangeVector operator+(const FVector& V) const{
		return FRangeVector(X + V.X, Y + V.Y, Z + V.Z);
	}
	FRangeVector operator-(const FVector& V) const{
		return FRangeVector(X - V.X, Y - V.Y, Z - V.Z);
	}
	FRangeVector operator*(FLOAT Scale) const{
		return FRangeVector(X * Scale, Y * Scale, Y * Scale);
	}
	FRangeVector operator/(FLOAT Scale) const{
		FLOAT RScale = 1.0/Scale;
		return FRangeVector(X * RScale, Y * RScale, Z * RScale);
	}
	FRangeVector operator*(const FRangeVector& R) const{
		return FRangeVector(X * R.X, Y * R.Y, Z * R.Z);
	}

	// Binary comparison operators.
	UBOOL operator==(const FRangeVector& R) const{
		return X==R.X && Y==R.Y && Z==R.Z;
	}
	UBOOL operator!=(const FRangeVector& R) const{
		return X!=R.X || Y!=R.Y || Z!=R.Z;
	}

	// Unary operators.
	FRangeVector operator-() const{
		return FRangeVector(-X, -Y, -Z);
	}

	// Assignment operators.
	FRangeVector operator+=(const FRangeVector& R)
	{
		X += R.X; Y += R.Y; Z += R.Z;
		return *this;
	}
	FRangeVector operator-=(const FRangeVector& R)
	{
		X -= R.X; Y -= R.Y; Z -= R.Z;
		return *this;
	}
	FRangeVector operator+=(const FVector& V)
	{
		X += V.X; Y += V.Y; Z += V.Z;
		return *this;
	}
	FRangeVector operator-=(const FVector& V)
	{
		X -= V.X; Y -= V.Y; Z -= V.Z;
		return *this;
	}
	FRangeVector operator*=(FLOAT Scale)
	{
		X *= Scale; Y *= Scale; Z *= Scale;
		return *this;
	}
	FRangeVector operator/=(FLOAT Scale)
	{
		FLOAT RScale = 1.0/Scale;
		X *= RScale; Y *= RScale; Z *= RScale;
		return *this;
	}
	FRangeVector operator*=(const FRangeVector& R)
	{
		X *= R.X; Y *= R.Y; Z *= R.Z;
		return *this;
	}
	FRangeVector operator/=(const FRangeVector& R)
	{
		X /= R.X; Y /= R.Y; Z /= R.Z;
		return *this;
	}
	FVector GetCenter() const{
		return FVector(X.GetCenter(), Y.GetCenter(), Z.GetCenter());
	}
	FVector GetMax() const{
		return FVector(X.GetMax(), Y.GetMax(), Z.GetMax());
	}
	FVector GetRand() const{
		return FVector(X.GetRand(), Y.GetRand(), Z.GetRand());
	}
	FVector GetSRand() const{
		return FVector(X.GetSRand(), Y.GetSRand(), Z.GetSRand());
	}
	int IsNearlyZero() const{
		return
				X.IsNearlyZero()
			&&	Y.IsNearlyZero()
			&&  Z.IsNearlyZero();
	}
	UBOOL IsZero() const{
		return X.IsZero() && Y.IsZero() && Z.IsZero();
	}
	FRangeVector GridSnap(const FRangeVector& Grid)
	{
		return FRangeVector(X.GridSnap(Grid.X), Y.GridSnap(Grid.Y), Z.GridSnap(Grid.Z));
	}
	FRange& Component(INT Index)
	{
		return (&X)[Index];
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FRangeVector& R)
	{
		return Ar << R.X << R.Y << R.Z;
	}
};


/*-----------------------------------------------------------------------------
	Bounds.
-----------------------------------------------------------------------------*/

/*
 * A rectangular minimum bounding volume.
 */
class CORE_API FBox{
public:
	// Variables.
	FVector Min;
	FVector Max;
	BYTE IsValid;

	// Constructors.
	FBox() {}
	FBox(INT) { Init(); }
	FBox(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax), IsValid(1) {}
	FBox(const FVector* Points, INT Count);

	// Accessors.
	FVector& GetExtrema(int i)
	{
		return (&Min)[i];
	}
	const FVector& GetExtrema(int i) const{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = Max = FVector(0,0,0);
		IsValid = 0;
	}
	FBox& operator+=(const FVector &Other)
	{
		if(IsValid)
		{
			Min.X = ::Min(Min.X, Other.X);
			Min.Y = ::Min(Min.Y, Other.Y);
			Min.Z = ::Min(Min.Z, Other.Z);

			Max.X = ::Max(Max.X, Other.X);
			Max.Y = ::Max(Max.Y, Other.Y);
			Max.Z = ::Max(Max.Z, Other.Z);
		}
		else
		{
			Min = Max = Other;
			IsValid = 1;
		}
		return *this;
	}
	FBox operator+(const FVector& Other) const{
		return FBox(*this) += Other;
	}
	FBox& operator+=(const FBox& Other)
	{
		if(IsValid && Other.IsValid)
		{
			Min.X = ::Min(Min.X, Other.Min.X);
			Min.Y = ::Min(Min.Y, Other.Min.Y);
			Min.Z = ::Min(Min.Z, Other.Min.Z);

			Max.X = ::Max(Max.X, Other.Max.X);
			Max.Y = ::Max(Max.Y, Other.Max.Y);
			Max.Z = ::Max(Max.Z, Other.Max.Z);
		}
		else *this = Other;
		return *this;
	}
	FBox operator+(const FBox& Other) const{
		return FBox(*this) += Other;
	}
    FVector& operator[](INT i)
    {
		check(i>-1);
		check(i<2);
		if(i == 0)		return Min;
		else				return Max;
	}
	FBox TransformBy(const FMatrix& M) const;
	FBox TransformBy(const FCoords& Coords) const{
		FBox NewBox(0);
		for(int i=0; i<2; i++)
			for(int j=0; j<2; j++)
				for(int k=0; k<2; k++)
					NewBox += FVector(GetExtrema(i).X, GetExtrema(j).Y, GetExtrema(k).Z).TransformPointBy(Coords);
		return NewBox;
	}
	FBox ExpandBy(FLOAT W) const{
		return FBox(Min - FVector(W,W,W), Max + FVector(W,W,W));
	}
	//Returns the midpoint between the min and max points.
	FVector GetCenter() const{
		return FVector((Min + Max) * 0.5f);
	}
	//Returns the extent around the center
	FVector GetExtent() const{
		return 0.5f*(Max - Min);
	}

	void GetCenterAndExtents(FVector & center, FVector & Extents) const{
		Extents = Max - Min;
		Extents *= .5f;
		center = Min + Extents;
	}

	bool Intersect(const FBox & other) const{
		if(Min.X > other.Max.X || other.Min.X > Max.X)
			return false;
		if(Min.Y > other.Max.Y || other.Min.Y > Max.Y)
			return false;
		if(Min.Z > other.Max.Z || other.Min.Z > Max.Z)
			return false;
		return true;
	}


	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FBox& Bound)
	{
		return Ar << Bound.Min << Bound.Max << Bound.IsValid;
	}
};

/*-----------------------------------------------------------------------------
	FInterpCurve.
-----------------------------------------------------------------------------*/
class CORE_API FInterpCurvePoint{
public:
	FLOAT	InVal;
	FLOAT	OutVal;

	FInterpCurvePoint() {}
	FInterpCurvePoint(FLOAT I, FLOAT O) : InVal(I), OutVal(O) {}

	UBOOL operator==(const FInterpCurvePoint &Other)
	{
		return (InVal == Other.InVal && OutVal == Other.OutVal);
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FInterpCurvePoint& Point)
	{
		return Ar << Point.InVal << Point.OutVal;
	}
};

class CORE_API FInterpCurve{
public:
	TArray<FInterpCurvePoint>	Points;

	FInterpCurve();

	void	AddPoint(FLOAT inV, FLOAT outV);
	void	Reset();
	FLOAT	Eval(FLOAT in);

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FInterpCurve& Curve)
	{
		return Ar << Curve.Points;
	}

	//Assignment (copy)
	void operator=(const FInterpCurve &Other)
	{
		Points = Other.Points;
	}
};


/*-----------------------------------------------------------------------------
	FGlobalMath.
-----------------------------------------------------------------------------*/

/*
 * Global mathematics info.
 */
class CORE_API FGlobalMath{
public:
	// Constants.
	enum {ANGLE_SHIFT 	= 2};		// Bits to right-shift to get lookup value.
	enum {ANGLE_BITS	= 14};		// Number of valid bits in angles.
	enum {NUM_ANGLES 	= 16384}; 	// Number of angles that are in lookup table.
	enum {NUM_SQRTS		= 16384};	// Number of square roots in lookup table.
	enum {ANGLE_MASK    =  (((1<<ANGLE_BITS)-1)<<(16-ANGLE_BITS))};

	// Class constants.
	const FVector  	WorldMin;
	const FVector  	WorldMax;
	const FCoords  	UnitCoords;
	const FScale   	UnitScale;
	const FCoords	ViewCoords;

	// Basic math functions.
	FLOAT Sqrt(int i)
	{
		return SqrtFLOAT[i];
	}
	FLOAT SinTab(int i)
	{
		return TrigFLOAT[((i>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FLOAT CosTab(int i)
	{
		return TrigFLOAT[(((i+16384)>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FLOAT SinFloat(FLOAT F)
	{
		return SinTab((INT)((F*65536.0f)/(2.0f*PI)));
	}
	FLOAT CosFloat(FLOAT F)
	{
		return CosTab((INT)((F*65536.0f)/(2.0f*PI)));
	}

	// Constructor.
	FGlobalMath();

private:
	// Tables.
	FLOAT  TrigFLOAT		[NUM_ANGLES];
	FLOAT  SqrtFLOAT		[NUM_SQRTS];
	FLOAT  LightSqrtFLOAT	[NUM_SQRTS];
};

inline INT ReduceAngle(INT Angle)
{
	return Angle & FGlobalMath::ANGLE_MASK;
};

/*-----------------------------------------------------------------------------
	Floating point constants.
-----------------------------------------------------------------------------*/

/*
 * Lengths of normalized vectors (These are half their maximum values
 * to assure that dot products with normalized vectors don't overflow).
 */
#define FLOAT_NORMAL_THRESH				(0.0001f)

//
// Magic numbers for numerical precision.
//
#define THRESH_POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.002f)	/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
													/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
													/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL		(0.02f)		/* Vectors are parallel if dot product varies less than this */

/*
 * FVerticesEqual
 */

inline UBOOL FVerticesEqual(const FVector& V1, const FVector& V2)
{
	if(Abs(V1.X - V2.X) > THRESH_POINTS_ARE_SAME * 4.0f)
		return 0;

	if(Abs(V1.Y - V2.Y) > THRESH_POINTS_ARE_SAME * 4.0f)
		return 0;

	if(Abs(V1.Z - V2.Z) > THRESH_POINTS_ARE_SAME * 4.0f)
		return 0;

	return 1;
}

/*-----------------------------------------------------------------------------
	FVector transformation.
-----------------------------------------------------------------------------*/

/*
 * Transformations in optimized assembler format.
 * An adaption of Michael Abrash' optimal transformation code.
 */
#if ASM
inline void ASMTransformPoint(const FCoords &Coords, const FVector &InVector, FVector &OutVector)
{
	// FCoords is a structure of 4 vectors: Origin, X, Y, Z
	//				 	  x  y  z
	// FVector	Origin;   0  4  8
	// FVector	XAxis;   12 16 20
	// FVector  YAxis;   24 28 32
	// FVector  ZAxis;   36 40 44
	//
	//	task:	Temp = (InVector - Coords.Origin);
	//			Outvector.X = (Temp | Coords.XAxis);
	//			Outvector.Y = (Temp | Coords.YAxis);
	//			Outvector.Z = (Temp | Coords.ZAxis);
	//
	// About 33 cycles on a Pentium.
	//
	__asm{
		mov     esi,[InVector]
		mov     edx,[Coords]
		mov     edi,[OutVector]

		// get source
		fld     dword ptr [esi+0]
		fld     dword ptr [esi+4]
		fld     dword ptr [esi+8] // z y x
		fxch    st(2)     // xyz

		// subtract origin
		fsub    dword ptr [edx + 0]  // xyz
		fxch    st(1)
		fsub	dword ptr [edx + 4]  // yxz
		fxch    st(2)
		fsub	dword ptr [edx + 8]  // zxy
		fxch    st(1)        // X Z Y

		// triplicate X for  transforming
		fld     st(0)	// X X   Z Y
        fmul    dword ptr [edx+12]     // Xx X Z Y
        fld     st(1)   // X Xx X  Z Y
        fmul    dword ptr [edx+24]   // Xy Xx X  Z Y
		fxch    st(2)
		fmul    dword ptr [edx+36]  // Xz Xx Xy  Z  Y
		fxch    st(4)     // Y  Xx Xy  Z  Xz

		fld     st(0)			// Y Y    Xx Xy Z Xz
		fmul    dword ptr [edx+16]
		fld     st(1) 			// Y Yx Y    Xx Xy Z Xz
        fmul    dword ptr [edx+28]
		fxch    st(2)			// Y  Yx Yy   Xx Xy Z Xz
		fmul    dword ptr [edx+40]	 // Yz Yx Yy   Xx Xy Z Xz
		fxch    st(1)			// Yx Yz Yy   Xx Xy Z Xz

        faddp   st(3),st(0)	  // Yz Yy  XxYx   Xy Z  Xz
        faddp   st(5),st(0)   // Yy  XxYx   Xy Z  XzYz
        faddp   st(2),st(0)   // XxYx  XyYy Z  XzYz
		fxch    st(2)         // Z     XyYy XxYx XzYz

		fld     st(0)         //  Z  Z     XyYy XxYx XzYz
		fmul    dword ptr [edx+20]
		fld     st(1)         //  Z  Zx Z  XyYy XxYx XzYz
        fmul    dword ptr [edx+32]
		fxch    st(2)         //  Z  Zx Zy
		fmul    dword ptr [edx+44]	  //  Zz Zx Zy XyYy XxYx XzYz
		fxch    st(1)         //  Zx Zz Zy XyYy XxYx XzYz

		faddp   st(4),st(0)   //  Zz Zy XyYy  XxYxZx  XzYz
		faddp   st(4),st(0)	  //  Zy XyYy     XxYxZx  XzYzZz
		faddp   st(1),st(0)   //  XyYyZy      XxYxZx  XzYzZz
		fxch    st(1)		  //  Xx+Xx+Zx   Xy+Yy+Zy  Xz+Yz+Zz

		fstp    dword ptr [edi+0]
        fstp    dword ptr [edi+4]
        fstp    dword ptr [edi+8]
	}
}

inline void ASMTransformVector(const FCoords &Coords, const FVector &InVector, FVector &OutVector)
{
	__asm{
		mov     esi,[InVector]
		mov     edx,[Coords]
		mov     edi,[OutVector]

		// get source
		fld     dword ptr [esi+0]
		fld     dword ptr [esi+4]
		fxch    st(1)
		fld     dword ptr [esi+8] // z x y
		fxch    st(1)             // x z y

		// triplicate X for  transforming
		fld     st(0)	// X X   Z Y
        fmul    dword ptr [edx+12]     // Xx X Z Y
        fld     st(1)   // X Xx X  Z Y
        fmul    dword ptr [edx+24]   // Xy Xx X  Z Y
		fxch    st(2)
		fmul    dword ptr [edx+36]  // Xz Xx Xy  Z  Y
		fxch    st(4)     // Y  Xx Xy  Z  Xz

		fld     st(0)			// Y Y    Xx Xy Z Xz
		fmul    dword ptr [edx+16]
		fld     st(1) 			// Y Yx Y    Xx Xy Z Xz
        fmul    dword ptr [edx+28]
		fxch    st(2)			// Y  Yx Yy   Xx Xy Z Xz
		fmul    dword ptr [edx+40]	 // Yz Yx Yy   Xx Xy Z Xz
		fxch    st(1)			// Yx Yz Yy   Xx Xy Z Xz

        faddp   st(3),st(0)	  // Yz Yy  XxYx   Xy Z  Xz
        faddp   st(5),st(0)   // Yy  XxYx   Xy Z  XzYz
        faddp   st(2),st(0)   // XxYx  XyYy Z  XzYz
		fxch    st(2)         // Z     XyYy XxYx XzYz

		fld     st(0)         //  Z  Z     XyYy XxYx XzYz
		fmul    dword ptr [edx+20]
		fld     st(1)         //  Z  Zx Z  XyYy XxYx XzYz
        fmul    dword ptr [edx+32]
		fxch    st(2)         //  Z  Zx Zy
		fmul    dword ptr [edx+44]	  //  Zz Zx Zy XyYy XxYx XzYz
		fxch    st(1)         //  Zx Zz Zy XyYy XxYx XzYz

		faddp   st(4),st(0)   //  Zz Zy XyYy  XxYxZx  XzYz
		faddp   st(4),st(0)	  //  Zy XyYy     XxYxZx  XzYzZz
		faddp   st(1),st(0)   //  XyYyZy      XxYxZx  XzYzZz
		fxch    st(1)		  //  Xx+Xx+Zx   Xy+Yy+Zy  Xz+Yz+Zz

		fstp    dword ptr [edi+0]
        fstp    dword ptr [edi+4]
        fstp    dword ptr [edi+8]
	}
}
#endif

/*
 * Transform a point by a coordinate system, moving
 * it by the coordinate system's origin if nonzero.
 */
inline FVector FVector::TransformPointBy(const FCoords &Coords) const{
#if ASM
	FVector Temp;
	ASMTransformPoint(Coords, *this, Temp);
	return Temp;
#else
	FVector Temp = *this - Coords.Origin;
	return FVector(	Temp | Coords.XAxis, Temp | Coords.YAxis, Temp | Coords.ZAxis);
#endif
}

/*
 * Transform a directional vector by a coordinate system.
 * Ignore's the coordinate system's origin.
 */
inline FVector FVector::TransformVectorBy(const FCoords &Coords) const{
#if ASM
	FVector Temp;
	ASMTransformVector(Coords, *this, Temp);
	return Temp;
#else
	return FVector(	*this | Coords.XAxis, *this | Coords.YAxis, *this | Coords.ZAxis);
#endif
}

inline FVector FVector::TransformVectorByTranspose(const FCoords &Coords) const{
	return FVector
	(
		X * Coords.XAxis.X + Y * Coords.YAxis.X + Z * Coords.ZAxis.X,
		X * Coords.XAxis.Y + Y * Coords.YAxis.Y + Z * Coords.ZAxis.Y,
		X * Coords.XAxis.Z + Y * Coords.YAxis.Z + Z * Coords.ZAxis.Z
	);
}


// Apply 'pivot' transform: First rotate, then add the translation.
inline FVector FVector::PivotTransform(const FCoords& Coords) const{
	return Coords.Origin + FVector(*this | Coords.XAxis, *this | Coords.YAxis, *this | Coords.ZAxis);
}

// Mirror a vector about a normal vector.
inline FVector FVector::MirrorByVector(const FVector& MirrorNormal) const{
	return *this - MirrorNormal * (2.0f * (*this | MirrorNormal));
}

// Mirror a vector about a plane.
inline FVector FVector::MirrorByPlane(const FPlane& Plane) const{
	return *this - Plane * (2.0f * Plane.PlaneDot(*this));
}

// Rotate around Axis (assumes Axis.Size() == 1)
inline FVector FVector::RotateAngleAxis(const INT Angle, const FVector& Axis) const{
	FLOAT S		= GMath.SinTab(Angle);
	FLOAT C		= GMath.CosTab(Angle);

	FLOAT XX	= Axis.X * Axis.X;
	FLOAT YY	= Axis.Y * Axis.Y;
	FLOAT ZZ	= Axis.Z * Axis.Z;

	FLOAT XY	= Axis.X * Axis.Y;
	FLOAT YZ	= Axis.Y * Axis.Z;
	FLOAT ZX	= Axis.Z * Axis.X;

	FLOAT XS	= Axis.X * S;
	FLOAT YS	= Axis.Y * S;
	FLOAT ZS	= Axis.Z * S;

	FLOAT OMC	= 1.0f - C;

	return FVector(
		(OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z,
		(OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z,
		(OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z
		);
}


/*-----------------------------------------------------------------------------
	FVector friends.
-----------------------------------------------------------------------------*/

/*
 * Compare two points and see if they're the same, using a threshold.
 * Uses fast distance approximation.
 */
inline bool FPointsAreSame(const FVector &P, const FVector &Q)
{
	FLOAT Temp;
	Temp=P.X-Q.X;
	if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
	{
		Temp=P.Y-Q.Y;
		if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
		{
			Temp=P.Z-Q.Z;

			if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
				return true;
		}
	}
	return false;
}

/*
 * Compare two points and see if they're the same, using a threshold.
 * Uses fast distance approximation.
 */
inline bool FPointsAreNear(const FVector &Point1, const FVector &Point2, FLOAT Dist)
{
	FLOAT Temp;
	Temp = (Point1.X - Point2.X); if(Abs(Temp) >= Dist) return false;
	Temp = (Point1.Y - Point2.Y); if(Abs(Temp) >= Dist) return false;
	Temp = (Point1.Z - Point2.Z); if(Abs(Temp) >= Dist) return false;
	return true;
}

/*
 * Calculate the signed distance (in the direction of the normal) between
 * a point and a plane.
 */
inline FLOAT FPointPlaneDist
(
	const FVector &Point,
	const FVector &PlaneBase,
	const FVector &PlaneNormal
){
	return (Point - PlaneBase) | PlaneNormal;
}

// Euclidean distance between two points.
inline FLOAT FDist(const FVector &V1, const FVector &V2)
{
	return appSqrt(Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z));
}

// Squared distance between two points.
inline FLOAT FDistSquared(const FVector &V1, const FVector &V2)
{
	return Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z);
}

// See if two normal vectors (or plane normals) are nearly parallel.
inline int FParallel(const FVector &Normal1, const FVector &Normal2)
{
	FLOAT NormalDot = Normal1 | Normal2;
	return (Abs (NormalDot - 1.0f) <= THRESH_VECTORS_ARE_PARALLEL);
}

// See if two planes are coplanar.
inline int FCoplanar(const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2)
{
	if      (!FParallel(Normal1,Normal2)) return 0;
	else if (FPointPlaneDist (Base2,Base1,Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
}

// Triple product of three vectors.
inline FLOAT FTriple(const FVector& X, const FVector& Y, const FVector& Z)
{
	return
	(	(X.X * (Y.Y * Z.Z - Y.Z * Z.Y))
	+	(X.Y * (Y.Z * Z.X - Y.X * Z.Z))
	+	(X.Z * (Y.X * Z.Y - Y.Y * Z.X)));
}

/*-----------------------------------------------------------------------------
	FCoords functions.
-----------------------------------------------------------------------------*/

/*
 * Return this coordinate system's transpose.
 *
 * If the coordinate system is orthogonal, this is equivalent to its inverse.
 */
inline FCoords FCoords::Transpose() const{
	return FCoords
	(
		-Origin.TransformVectorBy(*this),
		FVector(XAxis.X, YAxis.X, ZAxis.X),
		FVector(XAxis.Y, YAxis.Y, ZAxis.Y),
		FVector(XAxis.Z, YAxis.Z, ZAxis.Z)
	);
}

// Mirror the coordinates about a normal vector.
inline FCoords FCoords::MirrorByVector(const FVector& MirrorNormal) const{
	return FCoords
	(
		Origin.MirrorByVector(MirrorNormal),
		XAxis .MirrorByVector(MirrorNormal),
		YAxis .MirrorByVector(MirrorNormal),
		ZAxis .MirrorByVector(MirrorNormal)
	);
}

// Mirror the coordinates about a plane.
inline FCoords FCoords::MirrorByPlane(const FPlane& Plane) const{
	return FCoords
	(
		Origin.MirrorByPlane (Plane),
		XAxis .MirrorByVector(Plane),
		YAxis .MirrorByVector(Plane),
		ZAxis .MirrorByVector(Plane)
	);
}

/*-----------------------------------------------------------------------------
	FCoords operators.
-----------------------------------------------------------------------------*/

// Transform this coordinate system by another coordinate system.
inline FCoords& FCoords::operator*=(const FCoords& TransformCoords)
{
	//!! Proper solution:
	//Origin = Origin.TransformPointBy(TransformCoords.Inverse().Transpose());
	// Fast solution assuming orthogonal coordinate system:
	Origin = Origin.TransformPointBy (TransformCoords);
	XAxis  = XAxis .TransformVectorBy(TransformCoords);
	YAxis  = YAxis .TransformVectorBy(TransformCoords);
	ZAxis  = ZAxis .TransformVectorBy(TransformCoords);
	return *this;
}
inline FCoords FCoords::operator*(const FCoords &TransformCoords) const{
	return FCoords(*this) *= TransformCoords;
}

// Transform this coordinate system by a pitch-yaw-roll rotation.
inline FCoords& FCoords::operator*=(const FRotator &Rot)
{
	// Apply yaw rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+GMath.CosTab(Rot.Yaw), +GMath.SinTab(Rot.Yaw), +0.0f),
		FVector(-GMath.SinTab(Rot.Yaw), +GMath.CosTab(Rot.Yaw), +0.0f),
		FVector(+0.0f, +0.0f, +1.0f)
	);

	// Apply pitch rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+GMath.CosTab(Rot.Pitch), +0.0f, +GMath.SinTab(Rot.Pitch)),
		FVector(+0.0f, +1.0f, +0.0f),
		FVector(-GMath.SinTab(Rot.Pitch), +0.0f, +GMath.CosTab(Rot.Pitch))
	);

	// Apply roll rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+1.0f, +0.0f, +0.0f),
		FVector(+0.0f, +GMath.CosTab(Rot.Roll), -GMath.SinTab(Rot.Roll)),
		FVector(+0.0f, +GMath.SinTab(Rot.Roll), +GMath.CosTab(Rot.Roll))
	);
	return *this;
}
inline FCoords FCoords::operator*(const FRotator &Rot) const{
	return FCoords(*this) *= Rot;
}

inline FCoords& FCoords::operator*=(const FVector &Point)
{
	Origin -= Point;
	return *this;
}
inline FCoords FCoords::operator*(const FVector &Point) const{
	return FCoords(*this) *= Point;
}

// Detransform this coordinate system by a pitch-yaw-roll rotation.
inline FCoords& FCoords::operator/=(const FRotator &Rot)
{
	// Apply inverse roll rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+1.0f, -0.0f, +0.0f),
		FVector(-0.0f, +GMath.CosTab(Rot.Roll), +GMath.SinTab(Rot.Roll)),
		FVector(+0.0f, -GMath.SinTab(Rot.Roll), +GMath.CosTab(Rot.Roll))
	);

	// Apply inverse pitch rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+GMath.CosTab(Rot.Pitch), +0.0f, -GMath.SinTab(Rot.Pitch)),
		FVector(+0.0f, +1.0f, -0.0f),
		FVector(+GMath.SinTab(Rot.Pitch), +0.0f, +GMath.CosTab(Rot.Pitch))
	);

	// Apply inverse yaw rotation.
	*this *= FCoords
	(
		FVector(0.0f, 0.0f, 0.0f),
		FVector(+GMath.CosTab(Rot.Yaw), -GMath.SinTab(Rot.Yaw), -0.0f),
		FVector(+GMath.SinTab(Rot.Yaw), +GMath.CosTab(Rot.Yaw), +0.0f),
		FVector(-0.0f, +0.0f, +1.0f)
	);
	return *this;
}
inline FCoords FCoords::operator/(const FRotator &Rot) const{
	return FCoords(*this) /= Rot;
}

inline FCoords& FCoords::operator/=(const FVector &Point)
{
	Origin += Point;
	return *this;
}
inline FCoords FCoords::operator/(const FVector &Point) const{
	return FCoords(*this) /= Point;
}

/*
 * Transform this coordinate system by a scale.
 *
 * Note: Will return coordinate system of opposite handedness if
 * Scale.X*Scale.Y*Scale.Z is negative.
 */
inline FCoords& FCoords::operator*=(const FScale &Scale)
{
	// Apply sheering.
	FLOAT   Sheer      = FSheerSnap(Scale.SheerRate);
	FCoords TempCoords = GMath.UnitCoords;
	switch(Scale.SheerAxis)
	{
		case SHEER_XY:
			TempCoords.XAxis.Y = Sheer;
			break;
		case SHEER_XZ:
			TempCoords.XAxis.Z = Sheer;
			break;
		case SHEER_YX:
			TempCoords.YAxis.X = Sheer;
			break;
		case SHEER_YZ:
			TempCoords.YAxis.Z = Sheer;
			break;
		case SHEER_ZX:
			TempCoords.ZAxis.X = Sheer;
			break;
		case SHEER_ZY:
			TempCoords.ZAxis.Y = Sheer;
			break;
		default:
			break;
	}
	*this *= TempCoords;

	// Apply scaling.
	XAxis    *= Scale.Scale;
	YAxis    *= Scale.Scale;
	ZAxis    *= Scale.Scale;
	Origin.X /= Scale.Scale.X;
	Origin.Y /= Scale.Scale.Y;
	Origin.Z /= Scale.Scale.Z;

	return *this;
}
inline FCoords FCoords::operator*(const FScale &Scale) const{
	return FCoords(*this) *= Scale;
}

// Detransform a coordinate system by a scale.
inline FCoords& FCoords::operator/=(const FScale &Scale)
{
	// Deapply scaling.
	XAxis    /= Scale.Scale;
	YAxis    /= Scale.Scale;
	ZAxis    /= Scale.Scale;
	Origin.X *= Scale.Scale.X;
	Origin.Y *= Scale.Scale.Y;
	Origin.Z *= Scale.Scale.Z;

	// Deapply sheering.
	FCoords TempCoords(GMath.UnitCoords);
	FLOAT Sheer = FSheerSnap(Scale.SheerRate);
	switch(Scale.SheerAxis)
	{
		case SHEER_XY:
			TempCoords.XAxis.Y = -Sheer;
			break;
		case SHEER_XZ:
			TempCoords.XAxis.Z = -Sheer;
			break;
		case SHEER_YX:
			TempCoords.YAxis.X = -Sheer;
			break;
		case SHEER_YZ:
			TempCoords.YAxis.Z = -Sheer;
			break;
		case SHEER_ZX:
			TempCoords.ZAxis.X = -Sheer;
			break;
		case SHEER_ZY:
			TempCoords.ZAxis.Y = -Sheer;
			break;
		default: // SHEER_NONE
			break;
	}
	*this *= TempCoords;

	return *this;
}
inline FCoords FCoords::operator/(const FScale &Scale) const{
	return FCoords(*this) /= Scale;
}

/*-----------------------------------------------------------------------------
	Random numbers.
-----------------------------------------------------------------------------*/

const int FPSignBitMask = ~(1 << 31); // sjs

// Compute pushout of a box from a plane.
inline FLOAT FBoxPushOut(const FVector & Normal, const FVector & Size)
{
    float dx = Normal.X*Size.X;
    float dy = Normal.Y*Size.Y;
    float dz = Normal.Z*Size.Z;
    *(int*)(&dx) &= FPSignBitMask;
    *(int*)(&dy) &= FPSignBitMask;
    *(int*)(&dz) &= FPSignBitMask;
    return dx+dy+dz;
}


//Returns uniformly distributed random unit vector.
inline FVector VRand()
{
	FVector Result;

	do{
		// Check random vectors in the unit sphere so result is statistically uniform.
		Result.X = appFrand()*2 - 1;
		Result.Y = appFrand()*2 - 1;
		Result.Z = appFrand()*2 - 1;
	}while(Result.SizeSquared() > 1.0f);

	return Result.UnsafeNormal();
}

// quick and dirty random numbers (> 10x faster than rand() and 100x worse!)
extern CORE_API unsigned long qRandSeed;
const float INV_MAX_QUICK_RAND = 1.0f/0xffff;

inline void qSeedRand(unsigned int inSeed)
{
	qRandSeed = inSeed;
}

inline unsigned int qRand()
{
   qRandSeed = (qRandSeed * 196314165) + 907633515;
   return (qRandSeed >> 17);
}

inline float qFRand()
{
   qRandSeed = (qRandSeed * 196314165) + 907633515;

   return (float)(qRandSeed>>16) * INV_MAX_QUICK_RAND;
}

/*-----------------------------------------------------------------------------
	Texturing.
-----------------------------------------------------------------------------*/


// Returns UV texture coordinates for the specified vertex.
inline void FVectorsToTexCoords(FVector InVtx, FVector InPolyBase, FVector InTextureU, FVector InTextureV, FLOAT InMaterialUSize, FLOAT InMaterialVSize, FLOAT* InU, FLOAT* InV)
{
	*InU = ((InVtx - InPolyBase) | InTextureU) / InMaterialUSize;
	*InV = ((InVtx - InPolyBase) | InTextureV) / InMaterialVSize;
}

/*
 * Accepts a triangle (XYZ and UV values for each point) and returns a poly base and UV vectors
 *
 * NOTE : the UV coords should be scaled by the texture size
 */
inline void FTexCoordsToVectors(FVector V0, FVector UV0, FVector V1, FVector UV1, FVector V2, FVector UV2, FVector* InBaseResult, FVector* InUResult, FVector* InVResult)
{
	guard(FTexCoordsToVectors);

	// Create polygon normal.
	FVector PN = FVector((V0-V1) ^ (V2-V0));
	PN = PN.SafeNormal();

	// Fudge UV's to make sure no infinities creep into UV vector math, whenever we detect identical U or V's.
	if((UV0.X == UV1.X) || (UV2.X == UV1.X) || (UV2.X == UV0.X) ||
		(UV0.Y == UV1.Y) || (UV2.Y == UV1.Y) || (UV2.Y == UV0.Y))
		{
		UV1 += FVector(0.004173f,0.004123f,0.0f);
		UV2 += FVector(0.003173f,0.003123f,0.0f);
	}

	//
	// Solve the equations to find our texture U/V vectors 'TU' and 'TV' by stacking them
	// into a 3x3 matrix , one for  u(t) = TU dot (x(t)-x(o) + u(o) and one for v(t)=  TV dot (.... ,
	// then the third assumes we're perpendicular to the normal.
	//
	FCoords TexEqu;
	TexEqu.XAxis = FVector(	V1.X - V0.X, V1.Y - V0.Y, V1.Z - V0.Z);
	TexEqu.YAxis = FVector(V2.X - V0.X, V2.Y - V0.Y, V2.Z - V0.Z);
	TexEqu.ZAxis = FVector(PN.X,        PN.Y,        PN.Z       );
	TexEqu.Origin =FVector(0.0f, 0.0f, 0.0f);
	TexEqu = TexEqu.Inverse();

	FVector UResult(UV1.X-UV0.X, UV2.X-UV0.X, 0.0f);
	FVector TUResult = UResult.TransformVectorBy(TexEqu);

	FVector VResult(UV1.Y-UV0.Y, UV2.Y-UV0.Y, 0.0f);
	FVector TVResult = VResult.TransformVectorBy(TexEqu);

	//
	// Adjust the BASE to account for U0 and V0 automatically, and force it into the same plane.
	//
	FCoords BaseEqu;
	BaseEqu.XAxis = TUResult;
	BaseEqu.YAxis = TVResult;
	BaseEqu.ZAxis = FVector(PN.X, PN.Y, PN.Z);
	BaseEqu.Origin = FVector(0.0f, 0.0f, 0.0f);

	FVector BResult = FVector(UV0.X - (TUResult|V0), UV0.Y - (TVResult|V0),  0.0f);

	*InBaseResult = - 1.0f *  BResult.TransformVectorBy(BaseEqu.Inverse());
	*InUResult = TUResult;
	*InVResult = TVResult;

	unguard;
}

// Projects a texture coordinate system onto a plane.
inline void FProjectTextureToPlane(FVector& Base,FVector& X,FVector& Y,FPlane Plane)
{
	guard(FTexCoordsProjectToPlane);

	// Calculate a vector perpendicular to the texture(the texture normal).
	// Moving the texture base along this vector doesn't affect texture mapping.

	FVector	Z = (X ^ Y).SafeNormal();

	// Calculate the ratio of distance along the plane normal to distance along the texture normal.

	FLOAT	Ratio = 1.0f / (Z | Plane);

	// Project each component of the texture coordinate system onto the plane.

	Base = Base - Z * Plane.PlaneDot(Base) * Ratio;
	X = X - Z * (X | Plane) * Ratio;
	Y = Y - Z * (Y | Plane) * Ratio;

	unguard;
}

/*-----------------------------------------------------------------------------
	Advanced geometry.
-----------------------------------------------------------------------------*/

/*
 * Find the intersection of an infinite line (defined by two points) and a plane.
 *
 * Assumes that the line and plane do indeed intersect; you must
 * make sure they're not parallel before calling.
 */
inline FVector FLinePlaneIntersection
(
	const FVector &Point1,
	const FVector &Point2,
	const FVector &PlaneOrigin,
	const FVector &PlaneNormal
){
	return
		Point1
	+	(Point2-Point1)
	*	(((PlaneOrigin - Point1)|PlaneNormal) / ((Point2 - Point1)|PlaneNormal));
}
inline FVector FLinePlaneIntersection
(
	const FVector &Point1,
	const FVector &Point2,
	const FPlane  &Plane
){
	return
		Point1
	+	(Point2-Point1)
	*	((Plane.W - (Point1|Plane))/((Point2 - Point1)|Plane));
}

// Determines whether a point is inside a box.
inline UBOOL FPointBoxIntersection
(
	const FVector&	Point,
	const FBox&		Box
){
	if(Point.X >= Box.Min.X && Point.X <= Box.Max.X &&
	   Point.Y >= Box.Min.Y && Point.Y <= Box.Max.Y &&
	   Point.Z >= Box.Min.Z && Point.Z <= Box.Max.Z)
		return 1;
	else
		return 0;
}

#define BOX_SIDE_THRESHOLD	0.1f

// Determines whether a line intersects a box.
inline UBOOL FLineBoxIntersection
(
	const FBox&		Box,
	const FVector&	Start,
	const FVector&	End,
	const FVector&	Direction,
	const FVector&	OneOverDirection
){
	FVector	Time;
	UBOOL	Inside = 1;

	if(Start.X < Box.Min.X)
	{
		if(Direction.X <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.X = (Box.Min.X - Start.X) * OneOverDirection.X;
		}
	}
	else if(Start.X > Box.Max.X)
	{
		if(Direction.X >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.X = (Box.Max.X - Start.X) * OneOverDirection.X;
		}
	}
	else
		Time.X = 0.0f;

	if(Start.Y < Box.Min.Y)
	{
		if(Direction.Y <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Y = (Box.Min.Y - Start.Y) * OneOverDirection.Y;
		}
	}
	else if(Start.Y > Box.Max.Y)
	{
		if(Direction.Y >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Y = (Box.Max.Y - Start.Y) * OneOverDirection.Y;
		}
	}
	else
		Time.Y = 0.0f;

	if(Start.Z < Box.Min.Z)
	{
		if(Direction.Z <= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Z = (Box.Min.Z - Start.Z) * OneOverDirection.Z;
		}
	}
	else if(Start.Z > Box.Max.Z)
	{
		if(Direction.Z >= 0.0f)
			return 0;
		else
		{
			Inside = 0;
			Time.Z = (Box.Max.Z - Start.Z) * OneOverDirection.Z;
		}
	}
	else
		Time.Z = 0.0f;

	if(Inside)
		return 1;
	else
	{
		FLOAT	MaxTime = Max(Time.X,Max(Time.Y,Time.Z));

		if(MaxTime >= 0.0f && MaxTime <= 1.0f)
		{
			FVector	Hit = Start + Direction * MaxTime;

			if(	Hit.X > Box.Min.X - BOX_SIDE_THRESHOLD && Hit.X < Box.Max.X + BOX_SIDE_THRESHOLD &&
				Hit.Y > Box.Min.Y - BOX_SIDE_THRESHOLD && Hit.Y < Box.Max.Y + BOX_SIDE_THRESHOLD &&
				Hit.Z > Box.Min.Z - BOX_SIDE_THRESHOLD && Hit.Z < Box.Max.Z + BOX_SIDE_THRESHOLD)
				return 1;
		}

		return 0;
	}
}

CORE_API UBOOL FLineExtentBoxIntersection(const FBox& inBox,
								 const FVector& Start,
								 const FVector& End,
								 const FVector& Extent,
								 FVector& HitLocation,
								 FVector& HitNormal,
								 FLOAT& HitTime);

// Determines whether a line intersects a sphere.
inline UBOOL FLineSphereIntersection(FVector Start,FVector Dir,FLOAT Length,FVector Origin,FLOAT Radius)
{
	FVector	EO = Start - Origin;
	FLOAT	v = (Dir | (Origin - Start)),
			disc = Radius * Radius - ((EO | EO) - v * v);

	if(disc >= 0.0f)
	{
		FLOAT	Time = (v - appSqrt(disc)) / Length;

		if(Time >= 0.0f && Time <= 1.0f)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

/*-----------------------------------------------------------------------------
	FPlane functions.
-----------------------------------------------------------------------------*/

/*
 * Compute intersection point of three planes.
 * Returns true if valid, false if infinite.
 */
inline bool FIntersectPlanes3(FVector& I, const FPlane& P1, const FPlane& P2, const FPlane& P3)
{
	guard(FIntersectPlanes3);

	// Compute determinant, the triple product P1|(P2^P3)==(P1^P2)|P3.
	FLOAT Det = (P1 ^ P2) | P3;
	if(Square(Det) < Square(0.001f))
	{
		// Degenerate.
		I = FVector(0,0,0);
		return false;
	}
	else
	{
		// Compute the intersection point, guaranteed valid if determinant is nonzero.
		I = (P1.W*(P2^P3) + P2.W*(P3^P1) + P3.W*(P1^P2)) / Det;
	}
	return true;
	unguard;
}

/*
 * Compute intersection point and direction of line joining two planes.
 * Returns true if valid, false if infinite.
 */
inline UBOOL FIntersectPlanes2(FVector& I, FVector& D, const FPlane& P1, const FPlane& P2)
{
	guard(FIntersectPlanes2);

	// Compute line direction, perpendicular to both plane normals.
	D = P1 ^ P2;
	FLOAT DD = D.SizeSquared();
	if(DD < Square(0.001f))
	{
		// Parallel or nearly parallel planes.
		D = I = FVector(0,0,0);
		return false;
	}
	else
	{
		// Compute intersection.
		I = (P1.W*(P2^D) + P2.W*(D^P1)) / DD;
		D.Normalize();
		return true;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	FRotator functions.
-----------------------------------------------------------------------------*/

// Convert a rotation into a vector facing in its direction.
inline FVector FRotator::Vector()
{
	return (GMath.UnitCoords / *this).XAxis;
}

/*-----------------------------------------------------------------------------
	FQuat.
-----------------------------------------------------------------------------*/

/*
 * floating point quaternion.
 */
class CORE_API FQuat {
	public:
	// Variables.
	FLOAT X,Y,Z,W;
	// X,Y,Z, W also doubles as the Axis/Angle format.

	// Constructors.
	FQuat(){}

	FQuat(FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InA)
	:	X(InX), Y(InY), Z(InZ), W(InA){}

	// Binary operators.
	FQuat operator+(const FQuat& Q) const{
		return FQuat(X + Q.X, Y + Q.Y, Z + Q.Z, W + Q.W);
	}

	FQuat operator-(const FQuat& Q) const{
		return FQuat(X - Q.X, Y - Q.Y, Z - Q.Z, W - Q.W);
	}

	FQuat operator*(const FQuat& Q) const{
		return FQuat(
			W*Q.X + X*Q.W + Y*Q.Z - Z*Q.Y,
			W*Q.Y - X*Q.Z + Y*Q.W + Z*Q.X,
			W*Q.Z + X*Q.Y - Y*Q.X + Z*Q.W,
			W*Q.W - X*Q.X - Y*Q.Y - Z*Q.Z
			);
	}

	FQuat operator*(const FLOAT& Scale) const{
		return FQuat(Scale*X, Scale*Y, Scale*Z, Scale*W);
	}

	// Unary operators.
	FQuat operator-() const{
		return FQuat(X, Y, Z, -W);
	}

    // Misc operators
	UBOOL operator!=(const FQuat& Q) const{
		return X!=Q.X || Y!=Q.Y || Z!=Q.Z || W!=Q.W;
	}

	UBOOL Normalize()
	{
		//
		FLOAT SquareSum = (FLOAT)(X*X+Y*Y+Z*Z+W*W);
		if(SquareSum >= DELTA)
		{
			FLOAT Scale = 1.0f/(FLOAT)appSqrt(SquareSum);
			X *= Scale;
			Y *= Scale;
			Z *= Scale;
			W *= Scale;
			return true;
		}
		else {
			X = 0.0f;
			Y = 0.0f;
			Z = 0.1f;
			W = 0.0f;
			return false;
		}
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar, FQuat& F)
	{
		return Ar << F.X << F.Y << F.Z << F.W;
	}

	//Warning : assumes normalized quaternions.
	FQuat FQuatToAngAxis()
	{
		FLOAT scale = (FLOAT)appSin(W);
		FQuat A;

		if (scale >= DELTA)
		{
			A.X = Z / scale;
			A.Y = Y / scale;
			A.Z = Z / scale;
			A.W = (2.0f * appAcos (W));
			// Degrees: A.W = ((FLOAT)appAcos(W) * 360.0f) * INV_PI;
		}
		else {
			A.X = 0.0f;
			A.Y = 0.0f;
			A.Z = 1.0f;
			A.W = 0.0f;
		}

		return A;
	};

	//Angle-Axis to Quaternion. No normalized axis assumed.
	FQuat AngAxisToFQuat()
	{
		FLOAT scale = X*X + Y*Y + Z*Z;
		FQuat Q;

		if (scale >= DELTA)
		{
			FLOAT invscale = 1.0f /(FLOAT)appSqrt(scale);
			Q.X = X * invscale;
			Q.Y = Y * invscale;
			Q.Z = Z * invscale;
			Q.W = appCos(W * 0.5f); //Radians assumed.
		}
		else
		{
			Q.X = 0.0f;
			Q.Y = 0.0f;
			Q.Z = 1.0f;
			Q.W = 0.0f;
		}
		return Q;
	}

	FVector RotateVector(FVector v)
	{
		// (q.W*q.W-qv.qv)v + 2(qv.v)qv + 2 q.W (qv x v)

		FVector qv(X, Y, Z);
		FVector vOut = 2.0f * W * (qv ^ v);
		vOut += ((W * W) - (qv | qv)) * v;
		vOut += (2.0f * (qv | v)) * qv;

		return vOut;
	}
};

// Dot product of axes to get cos of angle  #Warning some people use .W component here too !
inline FLOAT FQuatDot(const FQuat& Q1,const FQuat& Q2)
{
	return(Q1.X*Q2.X + Q1.Y*Q2.Y + Q1.Z*Q2.Z);
};

// Error measure (angle) between two quaternions, ranged [0..1]
inline FLOAT FQuatError(FQuat& Q1,FQuat& Q2)
{
	// Returns the hypersphere-angle between two quaternions; alignment shouldn't matter, though
	// normalized input is expected.
	FLOAT cosom = Q1.X*Q2.X + Q1.Y*Q2.Y + Q1.Z*Q2.Z + Q1.W*Q2.W;
	return (Abs(cosom) < 0.9999999f) ? appAcos(cosom)*(1.0f/PI) : 0.0f;
}

// Ensure quat1 points to same side of the hypersphere as quat2
inline void AlignFQuatWith(FQuat &quat1, const FQuat &quat2)
{
	FLOAT Minus  = Square(quat1.X-quat2.X) + Square(quat1.Y-quat2.Y) + Square(quat1.Z-quat2.Z) + Square(quat1.W-quat2.W);
	FLOAT Plus   = Square(quat1.X+quat2.X) + Square(quat1.Y+quat2.Y) + Square(quat1.Z+quat2.Z) + Square(quat1.W+quat2.W);

	if (Minus > Plus)
	{
		quat1.X = - quat1.X;
		quat1.Y = - quat1.Y;
		quat1.Z = - quat1.Z;
		quat1.W = - quat1.W;
	}
}

// No-frills spherical interpolation. Assumes aligned quaternions, and the output is not normalized.
inline FQuat SlerpQuat(const FQuat &quat1,const FQuat &quat2, float slerp)
{
	FQuat result;
	float omega,cosom,sininv,scale0,scale1;

	// Get cosine of angle betweel quats.
	cosom = quat1.X * quat2.X +
			quat1.Y * quat2.Y +
			quat1.Z * quat2.Z +
			quat1.W * quat2.W;

	if(cosom < 0.99999999f)
	{
		omega = appAcos(cosom);
		sininv = 1.0f/appSin(omega);
		scale0 = appSin((1.0f - slerp) * omega) * sininv;
		scale1 = appSin(slerp * omega) * sininv;

		result.X = scale0 * quat1.X + scale1 * quat2.X;
		result.Y = scale0 * quat1.Y + scale1 * quat2.Y;
		result.Z = scale0 * quat1.Z + scale1 * quat2.Z;
		result.W = scale0 * quat1.W + scale1 * quat2.W;
		return result;
	}
	else
	{
		return quat1;
	}

}

/*-----------------------------------------------------------------------------
	FMatrix classes.
-----------------------------------------------------------------------------*/

/*
 * Floating point 4x4 matrix
 */
class FMatrix{
public:

	CORE_API static FMatrix Identity;

	// Variables.
	FLOAT M[4][4];

	// Constructors.

	FMatrix(){}

	FMatrix(FPlane InX, FPlane InY, FPlane InZ, FPlane InW)
	{
		M[0][0] = InX.X; M[0][1] = InX.Y;  M[0][2] = InX.Z;  M[0][3] = InX.W;
		M[1][0] = InY.X; M[1][1] = InY.Y;  M[1][2] = InY.Z;  M[1][3] = InY.W;
		M[2][0] = InZ.X; M[2][1] = InZ.Y;  M[2][2] = InZ.Z;  M[2][3] = InZ.W;
		M[3][0] = InW.X; M[3][1] = InW.Y;  M[3][2] = InW.Z;  M[3][3] = InW.W;
	}

	// Destructor.

	~FMatrix(){}

	void SetIdentity()
	{
		M[0][0] = 1; M[0][1] = 0;  M[0][2] = 0;  M[0][3] = 0;
		M[1][0] = 0; M[1][1] = 1;  M[1][2] = 0;  M[1][3] = 0;
		M[2][0] = 0; M[2][1] = 0;  M[2][2] = 1;  M[2][3] = 0;
		M[3][0] = 0; M[3][1] = 0;  M[3][2] = 0;  M[3][3] = 1;
	}

	FPlane* Rows()
	{
		return reinterpret_cast<FPlane*>(M);
	}

	//Concatenation operator.
	FMatrix operator*(FMatrix Other) const{
		FMatrix	Result;

		Result.M[0][0] = M[0][0] * Other.M[0][0] + M[0][1] * Other.M[1][0] + M[0][2] * Other.M[2][0] + M[0][3] * Other.M[3][0];
		Result.M[0][1] = M[0][0] * Other.M[0][1] + M[0][1] * Other.M[1][1] + M[0][2] * Other.M[2][1] + M[0][3] * Other.M[3][1];
		Result.M[0][2] = M[0][0] * Other.M[0][2] + M[0][1] * Other.M[1][2] + M[0][2] * Other.M[2][2] + M[0][3] * Other.M[3][2];
		Result.M[0][3] = M[0][0] * Other.M[0][3] + M[0][1] * Other.M[1][3] + M[0][2] * Other.M[2][3] + M[0][3] * Other.M[3][3];

		Result.M[1][0] = M[1][0] * Other.M[0][0] + M[1][1] * Other.M[1][0] + M[1][2] * Other.M[2][0] + M[1][3] * Other.M[3][0];
		Result.M[1][1] = M[1][0] * Other.M[0][1] + M[1][1] * Other.M[1][1] + M[1][2] * Other.M[2][1] + M[1][3] * Other.M[3][1];
		Result.M[1][2] = M[1][0] * Other.M[0][2] + M[1][1] * Other.M[1][2] + M[1][2] * Other.M[2][2] + M[1][3] * Other.M[3][2];
		Result.M[1][3] = M[1][0] * Other.M[0][3] + M[1][1] * Other.M[1][3] + M[1][2] * Other.M[2][3] + M[1][3] * Other.M[3][3];

		Result.M[2][0] = M[2][0] * Other.M[0][0] + M[2][1] * Other.M[1][0] + M[2][2] * Other.M[2][0] + M[2][3] * Other.M[3][0];
		Result.M[2][1] = M[2][0] * Other.M[0][1] + M[2][1] * Other.M[1][1] + M[2][2] * Other.M[2][1] + M[2][3] * Other.M[3][1];
		Result.M[2][2] = M[2][0] * Other.M[0][2] + M[2][1] * Other.M[1][2] + M[2][2] * Other.M[2][2] + M[2][3] * Other.M[3][2];
		Result.M[2][3] = M[2][0] * Other.M[0][3] + M[2][1] * Other.M[1][3] + M[2][2] * Other.M[2][3] + M[2][3] * Other.M[3][3];

		Result.M[3][0] = M[3][0] * Other.M[0][0] + M[3][1] * Other.M[1][0] + M[3][2] * Other.M[2][0] + M[3][3] * Other.M[3][0];
		Result.M[3][1] = M[3][0] * Other.M[0][1] + M[3][1] * Other.M[1][1] + M[3][2] * Other.M[2][1] + M[3][3] * Other.M[3][1];
		Result.M[3][2] = M[3][0] * Other.M[0][2] + M[3][1] * Other.M[1][2] + M[3][2] * Other.M[2][2] + M[3][3] * Other.M[3][2];
		Result.M[3][3] = M[3][0] * Other.M[0][3] + M[3][1] * Other.M[1][3] + M[3][2] * Other.M[2][3] + M[3][3] * Other.M[3][3];

		return Result;
	}

	void operator*=(FMatrix Other)
	{
		FMatrix Result;
		Result.M[0][0] = M[0][0] * Other.M[0][0] + M[0][1] * Other.M[1][0] + M[0][2] * Other.M[2][0] + M[0][3] * Other.M[3][0];
		Result.M[0][1] = M[0][0] * Other.M[0][1] + M[0][1] * Other.M[1][1] + M[0][2] * Other.M[2][1] + M[0][3] * Other.M[3][1];
		Result.M[0][2] = M[0][0] * Other.M[0][2] + M[0][1] * Other.M[1][2] + M[0][2] * Other.M[2][2] + M[0][3] * Other.M[3][2];
		Result.M[0][3] = M[0][0] * Other.M[0][3] + M[0][1] * Other.M[1][3] + M[0][2] * Other.M[2][3] + M[0][3] * Other.M[3][3];

		Result.M[1][0] = M[1][0] * Other.M[0][0] + M[1][1] * Other.M[1][0] + M[1][2] * Other.M[2][0] + M[1][3] * Other.M[3][0];
		Result.M[1][1] = M[1][0] * Other.M[0][1] + M[1][1] * Other.M[1][1] + M[1][2] * Other.M[2][1] + M[1][3] * Other.M[3][1];
		Result.M[1][2] = M[1][0] * Other.M[0][2] + M[1][1] * Other.M[1][2] + M[1][2] * Other.M[2][2] + M[1][3] * Other.M[3][2];
		Result.M[1][3] = M[1][0] * Other.M[0][3] + M[1][1] * Other.M[1][3] + M[1][2] * Other.M[2][3] + M[1][3] * Other.M[3][3];

		Result.M[2][0] = M[2][0] * Other.M[0][0] + M[2][1] * Other.M[1][0] + M[2][2] * Other.M[2][0] + M[2][3] * Other.M[3][0];
		Result.M[2][1] = M[2][0] * Other.M[0][1] + M[2][1] * Other.M[1][1] + M[2][2] * Other.M[2][1] + M[2][3] * Other.M[3][1];
		Result.M[2][2] = M[2][0] * Other.M[0][2] + M[2][1] * Other.M[1][2] + M[2][2] * Other.M[2][2] + M[2][3] * Other.M[3][2];
		Result.M[2][3] = M[2][0] * Other.M[0][3] + M[2][1] * Other.M[1][3] + M[2][2] * Other.M[2][3] + M[2][3] * Other.M[3][3];

		Result.M[3][0] = M[3][0] * Other.M[0][0] + M[3][1] * Other.M[1][0] + M[3][2] * Other.M[2][0] + M[3][3] * Other.M[3][0];
		Result.M[3][1] = M[3][0] * Other.M[0][1] + M[3][1] * Other.M[1][1] + M[3][2] * Other.M[2][1] + M[3][3] * Other.M[3][1];
		Result.M[3][2] = M[3][0] * Other.M[0][2] + M[3][1] * Other.M[1][2] + M[3][2] * Other.M[2][2] + M[3][3] * Other.M[3][2];
		Result.M[3][3] = M[3][0] * Other.M[0][3] + M[3][1] * Other.M[1][3] + M[3][2] * Other.M[2][3] + M[3][3] * Other.M[3][3];
		*this = Result;
	}

	// Comparison operators.

	inline UBOOL operator==(FMatrix& Other) const{
		for(INT X = 0;X < 4;X++)
			for(INT Y = 0;Y < 4;Y++)
				if(M[X][Y] != Other.M[X][Y])
					return 0;

		return 1;
	}

	inline UBOOL operator!=(FMatrix& Other) const{
		return !(*this == Other);
	}

	// Homogeneous transform.

	FPlane TransformFPlane(const FPlane &P) const{
		FPlane Result;

#if !_DEBUG
		__asm{
			// Setup.

			mov esi, P
			mov edx, [this]
			lea edi, Result

			fld dword ptr [esi + 0]		//	X
			fmul dword ptr [edx + 0]	//	Xx
			fld dword ptr [esi + 0]		//	X		Xx
			fmul dword ptr [edx + 4]	//	Xy		Xx
			fld dword ptr [esi + 0]		//	X		Xy		Xx
			fmul dword ptr [edx + 8]	//	Xz		Xy		Xx
			fld dword ptr [esi + 0]		//	X		Xz		Xy		Xx
			fmul dword ptr [edx + 12]	//	Xw		Xz		Xy		Xx

			fld dword ptr [esi + 4]		//	Y		Xw		Xz		Xy		Xx
			fmul dword ptr [edx + 16]	//	Yx		Xw		Xz		Xy		Xx
			fld dword ptr [esi + 4]		//	Y		Yx		Xw		Xz		Xy		Xx
			fmul dword ptr [edx + 20]	//	Yy		Yx		Xw		Xz		Xy		Xx
			fld dword ptr [esi + 4]		//	Y		Yy		Yx		Xw		Xz		Xy		Xx
			fmul dword ptr [edx + 24]	//	Yz		Yy		Yx		Xw		Xz		Xy		Xx
			fld dword ptr [esi + 4]		//	Y		Yz		Yy		Yx		Xw		Xz		Xy		Xx
			fmul dword ptr [edx + 28]	//	Yw		Yz		Yy		Yx		Xw		Xz		Xy		Xx

			fxch st(3)					//	Yx		Yz		Yy		Yw		Xw		Xz		Xy		Xx
			faddp st(7), st(0)			//	Yz		Yy		Yw		Xw		Xz		Xy		XYx
			faddp st(4), st(0)			//	Yy		Yw		Xw		XYz		Xy		XYx
			faddp st(4), st(0)			//	Yw		Xw		XYz		XYy		XYx
			faddp st(1), st(0)			//	XYw		XYz		XYy		XYx

			fld dword ptr [esi + 8]		//	Z		XYw		XYz		XYy		XYx
			fmul dword ptr [edx + 32]	//	Zx		XYw		XYz		XYy		XYx
			fld dword ptr [esi + 8]		//	Z		Zx		XYw		XYz		XYy		XYx
			fmul dword ptr [edx + 36]	//	Zy		Zx		XYw		XYz		XYy		XYx
			fld dword ptr [esi + 8]		//	Z		Zy		Zx		XYw		XYz		XYy		XYx
			fmul dword ptr [edx + 40]	//	Zz		Zy		Zx		XYw		XYz		XYy		XYx
			fld dword ptr [esi + 8]		//	Z		Zz		Zy		Zx		XYw		XYz		XYy		XYx
			fmul dword ptr [edx + 44]	//	Zw		Zz		Zy		Zx		XYw		XYz		XYy		XYx

			fxch st(3)					//	Zx		Zz		Zy		Zw		XYw		XYz		XYy		XYx
			faddp st(7), st(0)			//	Zz		Zy		Zw		XYw		XYz		XYy		XYZx
			faddp st(4), st(0)			//	Zy		Zw		XYw		XYZz	XYy		XYZx
			faddp st(4), st(0)			//	Zw		XYw		XYZz	XYZy	XYZx
			faddp st(1), st(0)			//	XYZw	XYZz	XYZy	XYZx

			fld dword ptr [esi + 12]	//	W		XYZw	XYZz	XYZy	XYZx
			fmul dword ptr [edx + 48]	//	Wx		XYZw	XYZz	XYZy	XYZx
			fld dword ptr [esi + 12]	//	W		Wx		XYZw	XYZz	XYZy	XYZx
			fmul dword ptr [edx + 52]	//	Wy		Wx		XYZw	XYZz	XYZy	XYZx
			fld dword ptr [esi + 12]	//	W		Wy		Wx		XYZw	XYZz	XYZy	XYZx
			fmul dword ptr [edx + 56]	//	Wz		Wy		Wx		XYZw	XYZz	XYZy	XYZx
			fld dword ptr [esi + 12]	//	W		Wz		Wy		Wx		XYZw	XYZz	XYZy	XYZx
			fmul dword ptr [edx + 60]	//	Ww		Wz		Wy		Wx		XYZw	XYZz	XYZy	XYZx

			fxch st(3)					//	Wx		Wz		Wy		Ww		XYZw	XYZz	XYZy	XYZx
			faddp st(7), st(0)			//	Wz		Wy		Ww		XYZw	XYZz	XYZy	XYZWx
			faddp st(4), st(0)			//	Wy		Ww		XYZw	XYZWz	XYZy	XYZWx
			faddp st(4), st(0)			//	Ww		XYZw	XYZWz	XYZWy	XYZWx
			faddp st(1), st(0)			//	XYZWw	XYZWz	XYZWy	XYZWx

			fxch st(3)					//	XYZWx	XYZWz	XYZWy	XYZWw
			fstp dword ptr [edi + 0]	//	XYZWz	XYZWy	XYZWw
			fxch st(1)					//	XYZWy	XYZWz	XYZWw
			fstp dword ptr [edi + 4]	//	XYZWz	XYZWw
			fstp dword ptr [edi + 8]	//	XYZWw
			fstp dword ptr [edi + 12]
		}
#else
		Result.X = P.X * M[0][0] + P.Y * M[1][0] + P.Z * M[2][0] + P.W * M[3][0];
		Result.Y = P.X * M[0][1] + P.Y * M[1][1] + P.Z * M[2][1] + P.W * M[3][1];
		Result.Z = P.X * M[0][2] + P.Y * M[1][2] + P.Z * M[2][2] + P.W * M[3][2];
		Result.W = P.X * M[0][3] + P.Y * M[1][3] + P.Z * M[2][3] + P.W * M[3][3];
#endif

		return Result;
	}

	//Regular transform.
	FVector TransformFVector(const FVector &V) const{
		return TransformFPlane(FPlane(V.X,V.Y,V.Z,1.0f));
	}

	//Normal transform.
	FPlane TransformNormal(const FVector& V) const{
		return TransformFPlane(FPlane(V.X,V.Y,V.Z,0.0f));
	}

	//Transpose.
	FMatrix Transpose() const{
		FMatrix	Result;

		Result.M[0][0] = M[0][0];
		Result.M[0][1] = M[1][0];
		Result.M[0][2] = M[2][0];
		Result.M[0][3] = M[3][0];

		Result.M[1][0] = M[0][1];
		Result.M[1][1] = M[1][1];
		Result.M[1][2] = M[2][1];
		Result.M[1][3] = M[3][1];

		Result.M[2][0] = M[0][2];
		Result.M[2][1] = M[1][2];
		Result.M[2][2] = M[2][2];
		Result.M[2][3] = M[3][2];

		Result.M[3][0] = M[0][3];
		Result.M[3][1] = M[1][3];
		Result.M[3][2] = M[2][3];
		Result.M[3][3] = M[3][3];

		return Result;
	}

	//Determinant.
	inline FLOAT Determinant() const{
		return	M[0][0] * (
					M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
					M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
					M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
					) -
				M[1][0] * (
					M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
					M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
					M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
					) +
				M[2][0] * (
					M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
					M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
					M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
					) -
				M[3][0] * (
					M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
					M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
					M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
					);
	}

	//Inverse.
	FMatrix Inverse()
	{
		FMatrix Result;
		FLOAT	Det = Determinant();

		if(Det == 0.0f)
			return FMatrix::Identity;

		FLOAT	RDet = 1.0f / Det;

		Result.M[0][0] = RDet * (
				M[1][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
				M[2][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
				M[3][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
				);
		Result.M[0][1] = -RDet * (
				M[0][1] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
				M[2][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
				);
		Result.M[0][2] = RDet * (
				M[0][1] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
				M[1][1] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				);
		Result.M[0][3] = -RDet * (
				M[0][1] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
				M[1][1] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
				M[2][1] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				);

		Result.M[1][0] = -RDet * (
				M[1][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
				M[2][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) +
				M[3][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2])
				);
		Result.M[1][1] = RDet * (
				M[0][0] * (M[2][2] * M[3][3] - M[2][3] * M[3][2]) -
				M[2][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2])
				);
		Result.M[1][2] = -RDet * (
				M[0][0] * (M[1][2] * M[3][3] - M[1][3] * M[3][2]) -
				M[1][0] * (M[0][2] * M[3][3] - M[0][3] * M[3][2]) +
				M[3][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				);
		Result.M[1][3] = RDet * (
				M[0][0] * (M[1][2] * M[2][3] - M[1][3] * M[2][2]) -
				M[1][0] * (M[0][2] * M[2][3] - M[0][3] * M[2][2]) +
				M[2][0] * (M[0][2] * M[1][3] - M[0][3] * M[1][2])
				);

		Result.M[2][0] = RDet * (
				M[1][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
				M[2][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) +
				M[3][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1])
				);
		Result.M[2][1] = -RDet * (
				M[0][0] * (M[2][1] * M[3][3] - M[2][3] * M[3][1]) -
				M[2][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
				M[3][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1])
				);
		Result.M[2][2] = RDet * (
				M[0][0] * (M[1][1] * M[3][3] - M[1][3] * M[3][1]) -
				M[1][0] * (M[0][1] * M[3][3] - M[0][3] * M[3][1]) +
				M[3][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
				);
		Result.M[2][3] = -RDet * (
				M[0][0] * (M[1][1] * M[2][3] - M[1][3] * M[2][1]) -
				M[1][0] * (M[0][1] * M[2][3] - M[0][3] * M[2][1]) +
				M[2][0] * (M[0][1] * M[1][3] - M[0][3] * M[1][1])
				);

		Result.M[3][0] = -RDet * (
				M[1][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
				M[2][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) +
				M[3][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1])
				);
		Result.M[3][1] = RDet * (
				M[0][0] * (M[2][1] * M[3][2] - M[2][2] * M[3][1]) -
				M[2][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
				M[3][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1])
				);
		Result.M[3][2] = -RDet * (
				M[0][0] * (M[1][1] * M[3][2] - M[1][2] * M[3][1]) -
				M[1][0] * (M[0][1] * M[3][2] - M[0][2] * M[3][1]) +
				M[3][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
				);
		Result.M[3][3] = RDet * (
				M[0][0] * (M[1][1] * M[2][2] - M[1][2] * M[2][1]) -
				M[1][0] * (M[0][1] * M[2][2] - M[0][2] * M[2][1]) +
				M[2][0] * (M[0][1] * M[1][2] - M[0][2] * M[1][1])
				);

		return Result;
	}

	FMatrix TransposeAdjoint() const{
		FMatrix ta;

		ta.M[0][0] = this->M[1][1] * this->M[2][2] - this->M[1][2] * this->M[2][1];
		ta.M[0][1] = this->M[1][2] * this->M[2][0] - this->M[1][0] * this->M[2][2];
		ta.M[0][2] = this->M[1][0] * this->M[2][1] - this->M[1][1] * this->M[2][0];
		ta.M[0][3] = 0.0f;

		ta.M[1][0] = this->M[2][1] * this->M[0][2] - this->M[2][2] * this->M[0][1];
		ta.M[1][1] = this->M[2][2] * this->M[0][0] - this->M[2][0] * this->M[0][2];
		ta.M[1][2] = this->M[2][0] * this->M[0][1] - this->M[2][1] * this->M[0][0];
		ta.M[1][3] = 0.0f;

		ta.M[2][0] = this->M[0][1] * this->M[1][2] - this->M[0][2] * this->M[1][1];
		ta.M[2][1] = this->M[0][2] * this->M[1][0] - this->M[0][0] * this->M[1][2];
		ta.M[2][2] = this->M[0][0] * this->M[1][1] - this->M[0][1] * this->M[1][0];
		ta.M[2][3] = 0.0f;

		ta.M[3][0] = 0.0f;
		ta.M[3][1] = 0.0f;
		ta.M[3][2] = 0.0f;
		ta.M[3][1] = 1.0f;

		return ta;
	}

	// Conversions.

	FCoords Coords()
	{
		FCoords	Result;

		Result.XAxis = FVector(M[0][0],M[1][0],M[2][0]);
		Result.YAxis = FVector(M[0][1],M[1][1],M[2][1]);
		Result.ZAxis = FVector(M[0][2],M[1][2],M[2][2]);
		Result.Origin = FVector(M[3][0],M[3][1],M[3][2]);

		return Result;
	}

	//Serializer.
	friend FArchive& operator<<(FArchive& Ar,FMatrix& M)
	{
		return Ar <<
			M.M[0][0] << M.M[0][1] << M.M[0][2] << M.M[0][3] <<
			M.M[1][0] << M.M[1][1] << M.M[1][2] << M.M[1][3] <<
			M.M[2][0] << M.M[2][1] << M.M[2][2] << M.M[2][3] <<
			M.M[3][0] << M.M[3][1] << M.M[3][2] << M.M[3][3];
	}
};

// Matrix operations.

class FPerspectiveMatrix : public FMatrix{
public:

	FPerspectiveMatrix(float FOVX, float FOVY, float MultFOVX, float MultFOVY, float MinZ, float MaxZ) :
	  FMatrix(
			FPlane(MultFOVX / appTan(FOVX),		0.0f,							0.0f,							0.0f),
			FPlane(0.0f,						MultFOVY / appTan(FOVY),		0.0f,							0.0f),
			FPlane(0.0f,						0.0f,							MaxZ / (MaxZ - MinZ),			1.0f),
			FPlane(0.0f,						0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f))
			{
	}

	FPerspectiveMatrix(float FOV, float Width, float Height, float MinZ, float MaxZ) :
	  FMatrix(
			FPlane(1.0f / appTan(FOV),	0.0f,							0.0f,							0.0f),
			FPlane(0.0f,				Width / appTan(FOV) / Height,	0.0f,							0.0f),
			FPlane(0.0f,				0.0f,							MaxZ / (MaxZ - MinZ),			1.0f),
			FPlane(0.0f,				0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f))
			{
	}
};

class FOrthoMatrix : public FMatrix{
public:

	FOrthoMatrix(float Width,float Height,float ZScale,float ZOffset) :
		FMatrix(
			FPlane(1.0f / Width,	0.0f,			0.0f,				0.0f),
			FPlane(0.0f,			1.0f / Height,	0.0f,				0.0f),
			FPlane(0.0f,			0.0f,			ZScale,				0.0f),
			FPlane(0.0f,			0.0f,			ZOffset * ZScale,	1.0f))
			{
	}
};

class FTranslationMatrix : public FMatrix{
public:

	FTranslationMatrix(FVector Delta) :
		FMatrix(
			FPlane(1.0f,	0.0f,	0.0f,	0.0f),
			FPlane(0.0f,	1.0f,	0.0f,	0.0f),
			FPlane(0.0f,	0.0f,	1.0f,	0.0f),
			FPlane(Delta.X,	Delta.Y,Delta.Z,1.0f))
			{
	}
};

class FRotationMatrix : public FMatrix{
public:

#if 0
	FRotationMatrix(FRotator Rot) :
	  FMatrix(
			FMatrix(	// Roll
				FPlane(1.0f,					0.0f,					0.0f,						0.0f),
				FPlane(0.0f,					+GMath.CosTab(Rot.Roll),-GMath.SinTab(Rot.Roll),	0.0f),
				FPlane(0.0f,					+GMath.SinTab(Rot.Roll),+GMath.CosTab(Rot.Roll),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f)) *
			FMatrix(	// Pitch
				FPlane(+GMath.CosTab(Rot.Pitch),0.0f,					+GMath.SinTab(Rot.Pitch),	0.0f),
				FPlane(0.0f,					1.0f,					0.0f,						0.0f),
				FPlane(-GMath.SinTab(Rot.Pitch),0.0f,					+GMath.CosTab(Rot.Pitch),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f)) *
			FMatrix(	// Yaw
				FPlane(+GMath.CosTab(Rot.Yaw),	+GMath.SinTab(Rot.Yaw), 0.0f,	0.0f),
				FPlane(-GMath.SinTab(Rot.Yaw),	+GMath.CosTab(Rot.Yaw), 0.0f,	0.0f),
				FPlane(0.0f,					0.0f,					1.0f,	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,	1.0f))
			)
	  {
	  }
#else
	FRotationMatrix(FRotator Rot)
	{
		FLOAT	SR	= GMath.SinTab(Rot.Roll),
				SP	= GMath.SinTab(Rot.Pitch),
				SY	= GMath.SinTab(Rot.Yaw),
				CR	= GMath.CosTab(Rot.Roll),
				CP	= GMath.CosTab(Rot.Pitch),
				CY	= GMath.CosTab(Rot.Yaw);

		M[0][0]	= CP * CY;
		M[0][1]	= CP * SY;
		M[0][2]	= SP;
		M[0][3]	= 0.0f;

		M[1][0]	= SR * SP * CY - CR * SY;
		M[1][1]	= SR * SP * SY + CR * CY;
		M[1][2]	= - SR * CP;
		M[1][3]	= 0.0f;

		M[2][0]	= -(CR * SP * CY + SR * SY);
		M[2][1]	= CY * SR - CR * SP * SY;
		M[2][2]	= CR * CP;
		M[2][3]	= 0.0f;

		M[3][0]	= 0.0f;
		M[3][1]	= 0.0f;
		M[3][2]	= 0.0f;
		M[3][3]	= 1.0f;
	}
#endif
};

class FInverseRotationMatrix : public FMatrix{
public:

	FInverseRotationMatrix(FRotator Rot) :
		FMatrix(
			FMatrix(	// Yaw
				FPlane(+GMath.CosTab(-Rot.Yaw),	+GMath.SinTab(-Rot.Yaw), 0.0f,	0.0f),
				FPlane(-GMath.SinTab(-Rot.Yaw),	+GMath.CosTab(-Rot.Yaw), 0.0f,	0.0f),
				FPlane(0.0f,					0.0f,					1.0f,	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,	1.0f)) *
			FMatrix(	// Pitch
				FPlane(+GMath.CosTab(-Rot.Pitch),0.0f,					+GMath.SinTab(-Rot.Pitch),	0.0f),
				FPlane(0.0f,					1.0f,					0.0f,						0.0f),
				FPlane(-GMath.SinTab(-Rot.Pitch),0.0f,					+GMath.CosTab(-Rot.Pitch),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f)) *
			FMatrix(	// Roll
				FPlane(1.0f,					0.0f,					0.0f,						0.0f),
				FPlane(0.0f,					+GMath.CosTab(-Rot.Roll),-GMath.SinTab(-Rot.Roll),	0.0f),
				FPlane(0.0f,					+GMath.SinTab(-Rot.Roll),+GMath.CosTab(-Rot.Roll),	0.0f),
				FPlane(0.0f,					0.0f,					0.0f,						1.0f))
			)
		{
	}
};

class FQuaternionMatrix : public FMatrix{
public:

	FQuaternionMatrix(FQuat Q)
	{
		FLOAT wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		x2 = Q.X + Q.X;  y2 = Q.Y + Q.Y;  z2 = Q.Z + Q.Z;
		xx = Q.X * x2;   xy = Q.X * y2;   xz = Q.X * z2;
		yy = Q.Y * y2;   yz = Q.Y * z2;   zz = Q.Z * z2;
		wx = Q.W * x2;   wy = Q.W * y2;   wz = Q.W * z2;

		M[0][0] = 1.0f - (yy + zz);
		M[1][0] = xy - wz;
		M[2][0] = xz + wy;
		M[3][0] = 0.0f;

		M[0][1] = xy + wz;
		M[1][1] = 1.0f - (xx + zz);
		M[2][1] = yz - wx;
		M[3][1] = 0.0f;

		M[0][2] = xz - wy;
		M[1][2] = yz + wx;
		M[2][2] = 1.0f - (xx + yy);
		M[3][2] = 0.0f;

		M[0][3] = 0.0f;
		M[1][3] = 0.0f;
		M[2][3] = 0.0f;
		M[3][3] = 1.0f;
	}
};

class FScaleMatrix : public FMatrix{
public:

	FScaleMatrix(FVector Scale) :
		FMatrix(
			FPlane(Scale.X,	0.0f,		0.0f,		0.0f),
			FPlane(0.0f,	Scale.Y,	0.0f,		0.0f),
			FPlane(0.0f,	0.0f,		Scale.Z,	0.0f),
			FPlane(0.0f,	0.0f,		0.0f,		1.0f))
			{
	}
};


/*
 * Transform a (rotation) matrix into a Quaternion.
 */
class FMatrixQuaternion : public FQuat{
public:
	FMatrixQuaternion(FMatrix M)
	{
		// Trace.
		FLOAT Trace = M.M[0][0] + M.M[1][1] + M.M[2][2] + 1.0f;
		// Calculate directly for positive trace.
		if(Trace > 0.0f)
		{
			 FLOAT S = 0.5f / appSqrt(Trace);
			 W = 0.25f / S;
			 X = (M.M[1][2] - M.M[2][1]) * S;
			 Y = (M.M[2][0] - M.M[0][2]) * S;
			 Z = (M.M[0][1] - M.M[1][0]) * S;
			 return;
		}
		// Or determine the major diagonal element.
		if((M.M[0][0] > M.M[1][1]) &&  (M.M[0][0] > M.M[2][2]))
		{
			FLOAT SZ = 0.5f/appSqrt(1.0f + M.M[0][0] - M.M[1][1] - M.M[2][2]);
			X = 0.5f * SZ;
			Y = (M.M[1][0] + M.M[0][1]) * SZ;
			Z = (M.M[2][0] + M.M[0][2]) * SZ;
			W = (M.M[2][1] + M.M[1][2]) * SZ;
		}
		else if(M.M[1][1] > M.M[2][2])
		{
			FLOAT SZ = 0.5f/appSqrt(1.0f + M.M[1][1] - M.M[0][0] - M.M[2][2]);
			X = (M.M[1][0] + M.M[0][1]) * SZ;
			Y = 0.5f * SZ;
			Z = (M.M[2][1] + M.M[1][2]) * SZ;
			W = (M.M[2][0] + M.M[0][2]) * SZ;
		}
		else {
			FLOAT SZ = 0.5f/appSqrt(1.0f + M.M[2][2] - M.M[0][0] - M.M[1][1]);
			X = (M.M[2][0] + M.M[0][2]) * SZ;
			Y = (M.M[2][1] + M.M[1][2]) * SZ;
			Z = 0.5f * SZ;
			W = (M.M[1][0] + M.M[0][1]) * SZ;
		}
	}
};

/*
 * Transform a (rotation) FCoords into a Quaternion.
 */
class FCoordsQuaternion : public FQuat{
public:
	FCoordsQuaternion(FCoords C)
	{
		// Trace.
		FLOAT Trace = C.XAxis.X + C.YAxis.Y + C.ZAxis.Z + 1.0f;
		// Calculate directly for positive trace.
		if(Trace > 0.0f)
		{
			 FLOAT S = 0.5f / appSqrt(Trace);
			 W = 0.25f / S;
			 X = (C.ZAxis.Y - C.YAxis.Z) * S;
			 Y = (C.XAxis.Z - C.ZAxis.X) * S;
			 Z = (C.YAxis.X - C.XAxis.Y) * S;
			 return;
		}
		// Or determine the major diagonal element.
		if((C.XAxis.X > C.YAxis.Y) &&  (C.XAxis.X > C.ZAxis.Z))
		{
			FLOAT SZ = 0.5f/appSqrt(1.0f + C.XAxis.X - C.YAxis.Y - C.ZAxis.Z);
			X = 0.5f * SZ;
			Y = (C.XAxis.Y + C.YAxis.X) * SZ;
			Z = (C.XAxis.Z + C.ZAxis.X) * SZ;
			W = (C.YAxis.Z + C.ZAxis.Y) * SZ;
		}
		else if(C.YAxis.Y > C.ZAxis.Z)
		{
			FLOAT SZ = 0.5f/appSqrt(1.0f + C.YAxis.Y - C.XAxis.X - C.ZAxis.Z);
			X = (C.XAxis.Y + C.YAxis.X) * SZ;
			Y = 0.5f * SZ;
			Z = (C.YAxis.Z + C.ZAxis.Y) * SZ;
			W = (C.XAxis.Z + C.ZAxis.X) * SZ;
		}
		else {
			FLOAT SZ = 0.5f/appSqrt(1.0f + C.ZAxis.Z - C.XAxis.X - C.YAxis.Y);
			X = (C.XAxis.Z + C.ZAxis.X) * SZ;
			Y = (C.YAxis.Z + C.ZAxis.Y) * SZ;
			Z = 0.5f * SZ;
			W = (C.XAxis.Y + C.YAxis.X) * SZ;
		}
	}
};


class FQuaternionCoords : public FCoords{
public:
	FQuaternionCoords(FQuat Q)
	{
		FLOAT wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

		x2 = Q.X + Q.X;  y2 = Q.Y + Q.Y;  z2 = Q.Z + Q.Z;
		xx = Q.X * x2;   xy = Q.X * y2;   xz = Q.X * z2;
		yy = Q.Y * y2;   yz = Q.Y * z2;   zz = Q.Z * z2;
		wx = Q.W * x2;   wy = Q.W * y2;   wz = Q.W * z2;

		XAxis.X = 1.0f - (yy + zz);
		XAxis.Y = xy - wz;
		XAxis.Z = xz + wy;

		YAxis.X = xy + wz;
		YAxis.Y = 1.0f - (xx + zz);
		YAxis.Z = yz - wx;

		ZAxis.X = xz - wy;
		ZAxis.Y = yz + wx;
		ZAxis.Z = 1.0f - (xx + yy);

		Origin.X = 0.0f;
		Origin.Y = 0.0f;
		Origin.Z = 0.0f;
	}
};



/*
	FBox::TransformBy
*/
inline FBox FBox::TransformBy(const FMatrix& M) const{
	FBox	NewBox(0);

	for(INT X = 0;X < 2;X++)
		for(INT Y = 0;Y < 2;Y++)
			for(INT Z = 0;Z < 2;Z++)
				NewBox += M.TransformFVector(FVector(GetExtrema(X).X,GetExtrema(Y).Y,GetExtrema(Z).Z));

	return NewBox;
}

/*-----------------------------------------------------------------------------
	FPlane implementation.
-----------------------------------------------------------------------------*/

/*
 * Transform a point by a coordinate system, moving
 * it by the coordinate system's origin if nonzero.
 */
inline FPlane FPlane::TransformPlaneByOrtho(const FCoords &Coords) const{
	FVector Normal = TransformVectorBy(Coords);
	return FPlane(Normal, W - (Coords.Origin.TransformVectorBy(Coords) | Normal));
}

inline FPlane FPlane::TransformBy(const FCoords &Coords) const{
	return FPlane((*this * W).TransformPointBy(Coords),TransformVectorBy(Coords).SafeNormal());
}

inline FPlane FPlane::TransformPlaneByOrtho(const FMatrix& M) const{
	FVector Normal = M.TransformFPlane(FPlane(X,Y,Z,0));
	return FPlane(Normal, W - (M.TransformFVector(FVector(0,0,0)) | Normal));
}

inline FPlane FPlane::TransformBy(const FMatrix& M) const{
	FMatrix tmpTA = M.TransposeAdjoint();
	return this->TransformByUsingAdjointT(M, tmpTA);
}

//You can optionally pass in the matrices transpose-adjoint, which save it recalculating it.
inline FPlane FPlane::TransformByUsingAdjointT(const FMatrix& M, const FMatrix& TA) const{
	FVector newNorm = TA.TransformNormal(*this).SafeNormal();

	if(M.Determinant() < 0.0f)
		newNorm *= -1;

	return FPlane(M.TransformFVector(*this * W), newNorm);
}

inline FSphere FSphere::TransformBy(const FMatrix& M) const{
	FSphere	Result;

	(FVector&)Result = M.TransformFVector(*this);

	FVector	XAxis(M.M[0][0],M.M[0][1],M.M[0][2]),
			YAxis(M.M[1][0],M.M[1][1],M.M[1][2]),
			ZAxis(M.M[2][0],M.M[2][1],M.M[2][2]);

	Result.W = appSqrt(MaxPositiveFloat(XAxis|XAxis,MaxPositiveFloat(YAxis|YAxis,ZAxis|ZAxis))) * W;

	return Result;
}

struct FCompressedPosition{
	FVector Location;
	FRotator Rotation;
	FVector Velocity;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
