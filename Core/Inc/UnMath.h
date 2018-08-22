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
class FVector;
class FPlane;
class FCoords;
class FRotator;
class FScale;
class FGlobalMath;
class FMatrix;
class FQuat;

// Fixed point conversion.
inline INT Fix	(INT A)		{ return A << 16; };
inline INT Fix	(FLOAT A)	{ return appRound(A * 65536.0f); };
inline INT Unfix(INT A)		{ return A >> 16; };

// Constants.
#undef  PI
#define PI 					(3.1415926535897932f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)

// Magic numbers for numerical precision.
#define DELTA		(0.00001f)
#define SLERP_DELTA	(0.0001f)

/*-----------------------------------------------------------------------------
	Global functions.
-----------------------------------------------------------------------------*/

//
// Snap a value to the nearest grid multiple.
//
inline FLOAT FSnap(FLOAT Location, FLOAT Grid){
	if(Grid == 0.0f)
		return Location;

	return appFloor((Location + 0.5f*Grid) / Grid) * Grid;
}

//
// Internal sheer adjusting function so it snaps nicely at 0 and 45 degrees.
//
inline FLOAT FSheerSnap (FLOAT Sheer){
	if(Sheer < -0.65f)	return Sheer + 0.15f;
	if(Sheer > 0.65f)	return Sheer - 0.15f;
	if(Sheer < -0.55f)	return -0.50f;
	if(Sheer > 0.55f)	return 0.50f;
	if(Sheer < -0.05f)	return Sheer + 0.05f;
	if(Sheer > 0.05f)	return Sheer - 0.05f;

	return 0.0f;
}

//
// Find the closest power of 2 that is >= N.
//
inline DWORD FNextPowerOfTwo(DWORD N){
	if( N<= 0L		) return 0L;
	if( N<= 1L		) return 1L;
	if( N<= 2L		) return 2L;
	if( N<= 4L		) return 4L;
	if( N<= 8L		) return 8L;
	if( N<= 16L		) return 16L;
	if( N<= 32L		) return 32L;
	if( N<= 64L		) return 64L;
	if( N<= 128L	) return 128L;
	if( N<= 256L	) return 256L;
	if( N<= 512L	) return 512L;
	if( N<= 1024L	) return 1024L;
	if( N<= 2048L	) return 2048L;
	if( N<= 4096L	) return 4096L;
	if( N<= 8192L	) return 8192L;
	if( N<= 16384L	) return 16384L;
	if( N<= 32768L	) return 32768L;
	if( N<= 65536L	) return 65536L;

	return 0;
}

//
// Add to a word angle, constraining it within a min (not to cross)
// and a max (not to cross).  Accounts for funkyness of word angles.
// Assumes that angle is initially in the desired range.
//
inline _WORD FAddAngleConfined(INT Angle, INT Delta, INT MinThresh, INT MaxThresh){
	if(Delta < 0){
		if(Delta <= -0x10000L || Delta <= -(INT)((_WORD)(Angle - MinThresh)))
			return (_WORD)MinThresh;
	}else if(Delta > 0){
		if(Delta >= 0x10000L || Delta >= (INT)((_WORD)(MaxThresh - Angle)))
			return (_WORD)MaxThresh;
	}

	return (_WORD)(Angle + Delta);
}

//
// Eliminate all fractional precision from an angle.
//
INT ReduceAngle(INT Angle);

//
// Fast 32-bit float evaluations. 
// Warning: likely not portable, and useful on Pentium class processors only.
//

inline UBOOL IsSmallerPositiveFloat(float F1,float F2){
	return ((*(DWORD*)&F1) < (*(DWORD*)&F2));
}

inline FLOAT MinPositiveFloat(float F1, float F2){
	if((*(DWORD*)&F1) < (*(DWORD*)&F2))
		return F1;

	return F2;
}

//
// Warning: 0 and -0 have different binary representations.
//

inline UBOOL EqualPositiveFloat(float F1, float F2){
	return (*(DWORD*)&F1 == *(DWORD*)&F2);
}

inline UBOOL IsNegativeFloat(float F1){
	return ((*(DWORD*)&F1) >= (DWORD)0x80000000); // Detects sign bit.
}

inline FLOAT MaxPositiveFloat(float F1, float F2){
	if((*(DWORD*)&F1) < (*(DWORD*)&F2))
		return F2;

	return F1;
}

// Clamp F0 between F1 and F2, all positive assumed.
inline FLOAT ClampPositiveFloat(float F0, float F1, float F2){
	if((*(DWORD*)&F0) < (*(DWORD*)&F1))
		return F1;

	if((*(DWORD*)&F0) > (*(DWORD*)&F2))
		return F2;
	
	return F0;
}

// Clamp any float F0 between zero and positive float Range
#define ClipFloatFromZero(F0, Range)\{\
	if ((*(DWORD*)&F0) >= (DWORD)0x80000000) F0 = 0.0f;\
	else if	((*(DWORD*)&F0) > (*(DWORD*)&Range)) F0 = Range;\
}

/*-----------------------------------------------------------------------------
	FVector.
-----------------------------------------------------------------------------*/

// Information associated with a floating point vector, describing its
// status as a point in a rendering context.
enum EVectorFlags{
	FVF_OutXMin		= 0x04,	// Outcode rejection, off left hand side of screen.
	FVF_OutXMax		= 0x08,	// Outcode rejection, off right hand side of screen.
	FVF_OutYMin		= 0x10,	// Outcode rejection, off top of screen.
	FVF_OutYMax		= 0x20,	// Outcode rejection, off bottom of screen.
	FVF_OutNear		= 0x40, // Near clipping plane.
	FVF_OutFar		= 0x80, // Far clipping plane.
	FVF_OutReject	= (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax), // Outcode rejectable.
	FVF_OutSkip		= (FVF_OutXMin | FVF_OutXMax | FVF_OutYMin | FVF_OutYMax), // Outcode clippable.
};

#define FVECTOR_ALIGNMENT DEFAULT_ALIGNMENT
class CORE_API FVector{
public:
	// Variables.
	FLOAT X,Y,Z;

	// Constructors.
	FVector(){}
	FVector(FLOAT InX, FLOAT InY, FLOAT InZ) : X(InX), Y(InY), Z(InZ){}
	FVector(FLOAT InF) : X(InF), Y(InF), Z(InF){}

	// Binary math operators.
	FVector operator^(const FVector& V) const{
		return FVector(
			Y * V.Z - Z * V.Y,
			Z * V.X - X * V.Z,
			X * V.Y - Y * V.X
		);
	}

	FLOAT operator|(const FVector& V) const{
		return X * V.X + Y * V.Y + Z * V.Z;
	}

	friend FVector operator*(FLOAT Scale, const FVector& V){
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
		FLOAT RScale = 1.0f / Scale;

		return FVector(X * RScale, Y * RScale, Z * RScale);
	}

	FVector operator*(const FVector& V) const{
		return FVector(X * V.X, Y * V.Y, Z * V.Z);
	}

	// Binary comparison operators.
	UBOOL operator==(const FVector& V) const{
		return X == V.X && Y == V.Y && Z == V.Z;
	}

	UBOOL operator!=(const FVector& V) const{
		return X != V.X || Y != V.Y || Z != V.Z;
	}

	// Unary operators.
	FVector operator-() const{
		return FVector(-X, -Y, -Z);
	}

	// Assignment operators.
	FVector operator+=(const FVector& V){
		X += V.X;
		Y += V.Y;
		Z += V.Z;

		return *this;
	}

	FVector operator-=(const FVector& V){
		X -= V.X;
		Y -= V.Y;
		Z -= V.Z;

		return *this;
	}

	FVector operator*=(FLOAT Scale){
		X *= Scale;
		Y *= Scale;
		Z *= Scale;

		return *this;
	}

	FVector operator/=(FLOAT V){
		FLOAT RV = 1.0f / V;

		X *= RV;
		Y *= RV;
		Z *= RV;

		return *this;
	}

	FVector operator*=(const FVector& V){
		X *= V.X;
		Y *= V.Y;
		Z *= V.Z;

		return *this;
	}

	FVector operator/=(const FVector& V){
		X /= V.X;
		Y /= V.Y;
		Z /= V.Z;

		return *this;
	}

	// Simple functions.
	FLOAT Size() const{
		return appSqrt(X * X + Y * Y + Z * Z);
	}

	FLOAT SizeSquared() const{
		return X * X + Y * Y + Z * Z;
	}

	FLOAT Size2D() const {
		return appSqrt(X * X + Y * Y);
	}

	FLOAT SizeSquared2D() const {
		return X * X + Y * Y;
	}

	int IsNearlyZero() const{
		return Abs(X) < KINDA_SMALL_NUMBER &&
			   Abs(Y) < KINDA_SMALL_NUMBER &&
			   Abs(Z) < KINDA_SMALL_NUMBER;
	}

	bool IsZero() const{
		return X == 0.0f && Y == 0.0f && Z == 0.0f;
	}

	bool Normalize(){
		FLOAT SquareSum = X * X + Y * Y + Z * Z;

		if(SquareSum >= SMALL_NUMBER){
			FLOAT Scale = 1.0f / appSqrt(SquareSum);

			X *= Scale;
			Y *= Scale;
			Z *= Scale;

			return true;
		}

		return false;
	}

	FVector Projection() const{
		FLOAT RZ = 1.0f / Z;

		return FVector(X * RZ, Y * RZ, 1);
	}

	FVector UnsafeNormal() const{
		FLOAT Scale = 1.0f / appSqrt(X * X + Y * Y + Z * Z);

		return FVector(X * Scale, Y * Scale, Z * Scale);
	}

	FVector GridSnap(const FVector& Grid){
		return FVector(FSnap(X, Grid.X), FSnap(Y, Grid.Y), FSnap(Z, Grid.Z));
	}

	FVector BoundToCube(FLOAT Radius){
		return FVector(
			Clamp(X, -Radius,Radius),
			Clamp(Y, -Radius,Radius),
			Clamp(Z, -Radius,Radius)
		);
	}

	void AddBounded(const FVector& V, FLOAT Radius=MAXSWORD){
		*this = (*this + V).BoundToCube(Radius);
	}

	FLOAT& Component(INT Index){
		return (&X)[Index];
	}

	// Return a boolean that is based on the vector's direction.
	// When      V==(0,0,0) Booleanize(0)=1.
	// Otherwise Booleanize(V) <-> !Booleanize(!B).
	bool Booleanize(){
		return
			X >  0.0f ? 1 :
			X <  0.0f ? 0 :
			Y >  0.0f ? 1 :
			Y <  0.0f ? 0 :
			Z >= 0.0f ? 1 : 0;
	}

	// Transformation.
	FVector TransformVectorBy(const FCoords& Coords) const;
	FVector TransformPointBy(const FCoords& Coords) const;
	FVector MirrorByVector(const FVector& MirrorNormal) const;
	FVector MirrorByPlane(const FPlane& MirrorPlane) const;
	FVector PivotTransform(const FCoords& Coords) const;

	// Complicated functions.
	FRotator Rotation();
	void FindBestAxisVectors(FVector& Axis1, FVector& Axis2);
	FVector SafeNormal() const; //warning: Not inline because of compiler bug.

	// Friends.
	friend FLOAT FDist(const FVector& V1, const FVector& V2);
	friend FLOAT FDistSquared(const FVector& V1, const FVector& V2);
	friend UBOOL FPointsAreSame(const FVector& P, const FVector& Q);
	friend UBOOL FPointsAreNear(const FVector& Point1, const FVector& Point2, FLOAT Dist);
	friend FLOAT FPointPlaneDist(const FVector& Point, const FVector& PlaneBase, const FVector& PlaneNormal);
	friend FVector FLinePlaneIntersection(const FVector& Point1, const FVector& Point2, const FVector& PlaneOrigin, const FVector& PlaneNormal);
	friend FVector FLinePlaneIntersection(const FVector& Point1, const FVector& Point2, const FPlane& Plane);
	friend UBOOL FParallel(const FVector& Normal1, const FVector& Normal2);
	friend UBOOL FCoplanar(const FVector& Base1, const FVector& Normal1, const FVector& Base2, const FVector& Normal2);

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FVector& V){
		return Ar << V.X << V.Y << V.Z;
	}
};

// Used by the multiple vertex editing function to keep track of selected vertices.
class ABrush;
class CORE_API FVertexHit{
public:
	// Variables.
	ABrush* pBrush;
	INT PolyIndex;
	INT VertexIndex;

	// Constructors.
	FVertexHit(){
		pBrush = NULL;
		PolyIndex = VertexIndex = 0;
	}

	FVertexHit(ABrush* _pBrush, INT _PolyIndex, INT _VertexIndex){
		pBrush = _pBrush;
		PolyIndex = _PolyIndex;
		VertexIndex = _VertexIndex;
	}

	// Functions.
	bool operator==(const FVertexHit& V) const{
		return pBrush==V.pBrush && PolyIndex==V.PolyIndex && VertexIndex==V.VertexIndex;
	}
	bool operator!=(const FVertexHit& V) const{
		return pBrush!=V.pBrush || PolyIndex!=V.PolyIndex || VertexIndex!=V.VertexIndex;
	}
};

class FPoly;
class CORE_API FFaceDragHit{
public:
	FFaceDragHit(ABrush* InBrush, FPoly* InPoly){
		Brush = InBrush;
		Poly = InPoly;
	}

	ABrush* Brush;
	FPoly* Poly;
};

/*-----------------------------------------------------------------------------
	FPlane.
-----------------------------------------------------------------------------*/

class CORE_API FPlane : public FVector{
public:
	// Variables.
	FLOAT W;

	// Constructors.
	FPlane(){}
	FPlane(const FPlane& P) : FVector(P), W(P.W){}
	FPlane(const FVector& V) : FVector(V), W(0){}
	FPlane(FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW) : FVector(InX, InY, InZ), W(InW){}
	FPlane(FVector InNormal, FLOAT InW) : FVector(InNormal), W(InW){}
	FPlane(FVector InBase, const FVector &InNormal) : FVector(InNormal), W(InBase | InNormal){}
	FPlane(FVector A, FVector B, FVector C) : FVector(((B-A)^(C-A)).SafeNormal()), W(A | ((B-A)^(C-A)).SafeNormal()){}

	// Functions.
	FLOAT PlaneDot(const FVector &P) const{
		return X * P.X + Y * P.Y + Z * P.Z - W;
	}

	FPlane Flip() const{
		return FPlane(-X, -Y, -Z, -W);
	}

	FPlane TransformPlaneByOrtho(const FCoords &Coords) const;

	bool operator==(const FPlane& V) const{
		return X == V.X && Y == V.Y && Z == V.Z && W == V.W;
	}

	bool operator!=(const FPlane& V) const{
		return X != V.X || Y != V.Y || Z != V.Z || W != V.W;
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FPlane &P){
		return Ar << (FVector&)P << P.W;
	}
};

/*-----------------------------------------------------------------------------
	FSphere.
-----------------------------------------------------------------------------*/

class CORE_API FSphere : public FPlane{
public:
	// Constructors.
	FSphere(){}
	FSphere(INT) : FPlane(0, 0, 0, 0){}
	FSphere(FVector V, FLOAT W) : FPlane(V, W){}
	FSphere(const FVector* Pts, INT Count);
	friend FArchive& operator<<(FArchive& Ar, FSphere& S){
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

// An axis along which sheering is performed.
enum ESheerAxis{
	SHEER_None	= 0,
	SHEER_XY	= 1,
	SHEER_XZ	= 2,
	SHEER_YX	= 3,
	SHEER_YZ	= 4,
	SHEER_ZX	= 5,
	SHEER_ZY	= 6,
};

//
// Scaling and sheering info associated with a brush.  This is 
// easily-manipulated information which is built into a transformation
// matrix later.
//
class CORE_API FScale {
public:
	// Variables.
	FVector	Scale;
	FLOAT	SheerRate;
	BYTE	SheerAxis; // From ESheerAxis

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FScale &S){
		return Ar << S.Scale << S.SheerRate << S.SheerAxis;
	}

	// Constructors.
	FScale(){}
	FScale(const FVector &InScale, FLOAT InSheerRate, ESheerAxis InSheerAxis) : Scale(InScale),
																				SheerRate(InSheerRate),
																				SheerAxis(InSheerAxis){}

	// Operators.
	bool operator==(const FScale &S) const{
		return Scale == S.Scale && SheerRate == S.SheerRate && SheerAxis == S.SheerAxis;
	}

	// Functions.
	FLOAT Orientation(){
		return Sgn(Scale.X * Scale.Y * Scale.Z);
	}
};

/*-----------------------------------------------------------------------------
	FCoords.
-----------------------------------------------------------------------------*/

//
// A coordinate system matrix.
//
class CORE_API FCoords{
public:
	FVector	Origin;
	FVector	XAxis;
	FVector YAxis;
	FVector ZAxis;

	// Constructors.
	FCoords(){}
	FCoords(const FVector &InOrigin) : Origin(InOrigin), XAxis(1,0,0), YAxis(0,1,0), ZAxis(0,0,1){}
	FCoords(const FVector &InOrigin, const FVector &InX, const FVector &InY, const FVector &InZ) : Origin(InOrigin),
																								   XAxis(InX),
																								   YAxis(InY),
																								   ZAxis(InZ){}

	// Functions.
	FCoords MirrorByVector(const FVector& MirrorNormal) const;
	FCoords MirrorByPlane(const FPlane& MirrorPlane) const;
	FCoords	Transpose() const;
	FCoords Inverse() const;
	FCoords PivotInverse() const;
	FCoords ApplyPivot(const FCoords& CoordsB) const;
	FRotator OrthoRotation() const;

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

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FCoords& F){
		return Ar << F.Origin << F.XAxis << F.YAxis << F.ZAxis;
	}
};

/*-----------------------------------------------------------------------------
	FModelCoords.
-----------------------------------------------------------------------------*/

//
// A model coordinate system, describing both the covariant and contravariant
// transformation matrices to transform points and normals by.
//
class CORE_API FModelCoords{
public:
	// Variables.
	FCoords PointXform;		// Coordinates to transform points by  (covariant).
	FCoords VectorXform;	// Coordinates to transform normals by (contravariant).

	// Constructors.
	FModelCoords(){}
	FModelCoords(const FCoords& InCovariant, const FCoords& InContravariant) : PointXform(InCovariant),
																			   VectorXform(InContravariant){}

	// Functions.
	FModelCoords Inverse(){
		return FModelCoords(VectorXform.Transpose(), PointXform.Transpose());
	}
};

/*-----------------------------------------------------------------------------
	FRotator.
-----------------------------------------------------------------------------*/

//
// Rotation.
//
class CORE_API FRotator{
public:
	// Variables.
	INT Pitch; // Looking up and down (0=Straight Ahead, +Up, -Down).
	INT Yaw;   // Rotating around (running in circles), 0=East, +North, -South.
	INT Roll;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FRotator& R){
		return Ar << R.Pitch << R.Yaw << R.Roll;
	}

	// Constructors.
	FRotator(){}
	FRotator(INT InPitch, INT InYaw, INT InRoll) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll){}
	FRotator(FLOAT Inf) : Pitch(Inf), Yaw(Inf), Roll(Inf){}

	// Binary arithmetic operators.
	FRotator operator+(const FRotator &R) const{
		return FRotator(Pitch + R.Pitch, Yaw + R.Yaw, Roll + R.Roll);
	}

	FRotator operator-(const FRotator &R) const{
		return FRotator(Pitch - R.Pitch, Yaw - R.Yaw, Roll - R.Roll);
	}

	FRotator operator*(FLOAT Scale) const{
		return FRotator(appRound(Pitch * Scale), appRound(Yaw * Scale), appRound(Roll * Scale));
	}

	friend FRotator operator*(FLOAT Scale, const FRotator &R){
		return FRotator(appRound(R.Pitch * Scale), appRound(R.Yaw * Scale), appRound(R.Roll * Scale));
	}

	FRotator operator*= (FLOAT Scale){
		Pitch = appRound(Pitch*Scale); Yaw = appRound(Yaw*Scale); Roll = appRound(Roll*Scale);
		return *this;
	}

	// Binary comparison operators.
	bool operator==(const FRotator &R) const{
		return Pitch == R.Pitch && Yaw == R.Yaw && Roll == R.Roll;
	}

	bool operator!=(const FRotator &V) const{
		return Pitch != V.Pitch || Yaw != V.Yaw || Roll != V.Roll;
	}

	// Assignment operators.
	FRotator operator+=(const FRotator &R){
		Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
		return *this;
	}

	FRotator operator-=(const FRotator &R){
		Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
		return *this;
	}

	// Functions.
	FRotator Reduce() const{
		return FRotator(ReduceAngle(Pitch), ReduceAngle(Yaw), ReduceAngle(Roll));
	}

	bool IsZero() const{
		return ((Pitch & 65535) == 0) && ((Yaw & 65535) == 0) && ((Roll & 65535) == 0);
	}

	FRotator Add(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll){
		Yaw   += DeltaYaw;
		Pitch += DeltaPitch;
		Roll  += DeltaRoll;
		return *this;
	}

	FRotator AddBounded(INT DeltaPitch, INT DeltaYaw, INT DeltaRoll){
		Yaw  += DeltaYaw;
		Pitch = FAddAngleConfined(Pitch, DeltaPitch, 192 * 0x100, 64 * 0x100);
		Roll  = FAddAngleConfined(Roll, DeltaRoll, 192 * 0x100, 64 * 0x100);
		return *this;
	}

	FRotator GridSnap(const FRotator &RotGrid){
		return FRotator(
			appRound(FSnap(Pitch,RotGrid.Pitch)),
			appRound(FSnap(Yaw,  RotGrid.Yaw)),
			appRound(FSnap(Roll, RotGrid.Roll))
		);
	}

	FVector Vector();
};

/*-----------------------------------------------------------------------------
	Bounds.
-----------------------------------------------------------------------------*/

//
// A rectangular minimum bounding volume.
//
class CORE_API FBox{
public:
	// Variables.
	FVector Min;
	FVector Max;
	BYTE IsValid;

	// Constructors.
	FBox(){}
	FBox(INT) : Min(0,0,0), Max(0,0,0), IsValid(0){}
	FBox(const FVector& InMin, const FVector& InMax) : Min(InMin), Max(InMax), IsValid(1){}
	FBox(const FVector* Points, INT Count);

	// Accessors.
	FVector& GetExtrema(int i){
		return (&Min)[i];
	}
	const FVector& GetExtrema(int i) const{
		return (&Min)[i];
	}

	// Functions.
	FBox& operator+=(const FVector &Other){
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
	FBox& operator+=(const FBox& Other){
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

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FBox& Bound){
		return Ar << Bound.Min << Bound.Max << Bound.IsValid;
	}
};

/*-----------------------------------------------------------------------------
	FGlobalMath.
-----------------------------------------------------------------------------*/

//
// Global mathematics info.
//
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
	FLOAT Sqrt(int i){
		return SqrtFLOAT[i]; 
	}
	FLOAT SinTab(int i){
		return TrigFLOAT[((i>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FLOAT CosTab(int i){
		return TrigFLOAT[(((i+16384)>>ANGLE_SHIFT)&(NUM_ANGLES-1))];
	}
	FLOAT SinFloat(FLOAT F){
		return SinTab(appRound((F*65536.0f)/(2.0f*PI)));
	}
	FLOAT CosFloat(FLOAT F){
		return CosTab(appRound((F*65536.0f)/(2.0f*PI)));
	}

	// Constructor.
	FGlobalMath();

private:
	// Tables.
	FLOAT  TrigFLOAT		[NUM_ANGLES];
	FLOAT  SqrtFLOAT		[NUM_SQRTS];
	FLOAT  LightSqrtFLOAT	[NUM_SQRTS];
};

inline INT ReduceAngle(INT Angle){
	return Angle & FGlobalMath::ANGLE_MASK;
};

/*-----------------------------------------------------------------------------
	Floating point constants.
-----------------------------------------------------------------------------*/

//
// Lengths of normalized vectors (These are half their maximum values
// to assure that dot products with normalized vectors don't overflow).
//
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



/*-----------------------------------------------------------------------------
	FVector transformation.
-----------------------------------------------------------------------------*/

//
// Transformations in optimized assembler format.
// An adaption of Michael Abrash' optimal transformation code.
//
#if ASM
inline void ASMTransformPoint(const FCoords &Coords, const FVector &InVector, FVector &OutVector){
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
#elif ASMLINUX
inline void ASMTransformPoint(const FCoords &Coords, const FVector &InVector, FVector &OutVector){
	__asm__ __volatile__ ("
		# Get source.
		flds	0(%%esi);			# x
		flds	4(%%esi);			# y x
		flds	8(%%esi);			# z y x
		fxch	%%st(2);

		# Subtract origin.
		fsubs	0(%1);
		fxch	%%st(1);
		fsubs	4(%1);
		fxch	%%st(2);
		fsubs	8(%1);
		fxch	%%st(1);

		# Triplicate X for transforming.
		fld		%%st(0);
		fmuls	12(%1);
		fld		%%st(1);
		fmuls	24(%1);
		fxch	%%st(2);
		fmuls	36(%1);
		fxch	%%st(4);
		
		fld		%%st(0);
		fmuls	16(%1);
		fld		%%st(1);
		fmuls	28(%1);
		fxch	%%st(2);
		fmuls	40(%1);
		fxch	%%st(1);

		faddp	%%st(0),%%st(3);
		faddp	%%st(0),%%st(5);
		faddp	%%st(0),%%st(2);
		fxch	%%st(2);
		
		fld		%%st(0);
		fmuls	20(%1);
		fld		%%st(1);
		fmuls	32(%1);
		fxch	%%st(2);
		fmuls	44(%1);
		fxch	%%st(1);
		
		faddp	%%st(0),%%st(4);
		faddp	%%st(0),%%st(4);
		faddp	%%st(0),%%st(1);
		fxch	%%st(1);

		fstps	0(%%edi);
		fstps	4(%%edi);
		fstps	8(%%edi);
	"
	:
	:	"S" (&InVector),
		"q" (&Coords),
		"D" (&OutVector)
	: "memory"
	);
}
#endif

#if ASM
inline void ASMTransformVector(const FCoords &Coords, const FVector &InVector, FVector &OutVector){
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

#if ASMLINUX
inline void ASMTransformVector(const FCoords &Coords, const FVector &InVector, FVector &OutVector){
	asm volatile("
		# Get source.
		flds	0(%%esi);
		flds	4(%%esi);
		fxch	%%st(1);
		flds	8(%%esi);
		fxch	%%st(1);

		# Triplicate X for transforming.
		fld		%%st(0);
		fmuls	12(%1);
		fld		%%st(1);
		fmuls	24(%1);
		fxch	%%st(2);
		fmuls	36(%1);
		fxch	%%st(4);

		fld		%%st(0);
		fmuls	16(%1);
		fld		%%st(1);
		fmuls	28(%1);
		fxch	%%st(2);
		fmuls	40(%1);
		fxch	%%st(1);

		faddp	%%st(0),%%st(3);
		faddp	%%st(0),%%st(5);
		faddp	%%st(0),%%st(2);
		fxch	%%st(2);

		fld		%%st(0);
		fmuls	20(%1);
		fld		%%st(1);
		fmuls	32(%1);
		fxch	%%st(2);
		fmuls	44(%1);
		fxch	%%st(1);

		faddp	%%st(0),%%st(4);
		faddp	%%st(0),%%st(4);
		faddp	%%st(0),%%st(1);
		fxch	%%st(1);

		fstps	0(%%edi);
		fstps	4(%%edi);
		fstps	8(%%edi);
	"
	:
	: "S" (&InVector),
	  "q" (&Coords),
	  "D" (&OutVector)
	: "memory"
	);
}
#endif

//
// Transform a point by a coordinate system, moving
// it by the coordinate system's origin if nonzero.
//
inline FVector FVector::TransformPointBy(const FCoords &Coords) const{
#if ASM
	FVector Temp;
	ASMTransformPoint(Coords, *this, Temp);
	return Temp;
#elif ASMLINUX
	static FVector Temp;
	ASMTransformPoint(Coords, *this, Temp);
	return Temp;
#elif __PSX2_EE__
	FVector Result;
	FVector UnitVector = FVector(1.0f, 1.0f, 1.0f);
	asm volatile ("
		lqc2		vf1, 0x00(%0)
		lqc2		vf3, 0x00(%1)
		lqc2		vf4, 0x00(%2)
		lqc2		vf5, 0x00(%3)
		lqc2		vf6, 0x00(%4)
	"
	:
	: "r" (&UnitVector),
	  "r" (&Coords.Origin), "r" (&Coords.XAxis),
	  "r" (&Coords.YAxis), "r" (&Coords.ZAxis)
	);
	asm volatile ("
		/* Register allocation:
		 * vf0: 0, 0, 0, 1 constant vector
		 * vf1: 1, 1, 1, 1 unit vector
		 * vf3: Frame origin
		 * vf4: XAxis
		 * vf5: YAxis
		 * vf6: ZAxis
		 */

		/* Load 128bit values. */
		lqc2		vf2,  0x00(%1)

		/* Get subtracted vector */
		vsub.xyz	vf7xyz, vf2xyz, vf3xyz		/* vf6 <- Temp */

		/* Perform dot products */
		vmul.xyz	vf8xyz, vf7xyz, vf4xyz		/* vf8 <- Temp | XAxis */
		vaddax.x	ACCx, vf0x,	vf8x			/* ACCx = vf8.x */
		vmadday.x	ACCx, vf1x, vf8y			/* ACCx = vf8.x + vf8.y */
		vmaddaz.x	ACCx, vf1x, vf8z			/* ACCx = vf8.x + vf8.y + vf8.z */

		vmul.xyz	vf8xyz, vf7xyz, vf5xyz		/* vf8 <- Temp | YAxis */
		vaddax.y	ACCy, vf0y,	vf8x			/* ACCy = vf8.x */
		vmadday.y	ACCy, vf1y, vf8y			/* ACCy = vf8.x + vf8.y */
		vmaddaz.y	ACCy, vf1y, vf8z			/* ACCy = vf8.x + vf8.y + vf8.z */
			
		vmul.xyz	vf8xyz, vf7xyz, vf6xyz		/* vf8 <- Temp | ZAxis */
		vaddax.z	ACCz, vf0z,	vf8x			/* ACCy = vf8.x */
		vmadday.z	ACCz, vf1z, vf8y			/* ACCy = vf8.x + vf8.y */
		vmaddaz.z	ACCz, vf1z, vf8z			/* ACCy = vf8.x + vf8.y + vf8.z */

		vmadd		vf9, vf0, vf0				/* vf9 = ACC + 0 * 0 */
		sqc2		vf9, 0x00(%0)				/* Result.Point <- ACC */
	"
	:
	: "r" (&Result), "r" (this)
	: "memory"
	);
	return Result;
#else
	FVector Temp = *this - Coords.Origin;
	return FVector(	Temp | Coords.XAxis, Temp | Coords.YAxis, Temp | Coords.ZAxis);
#endif
}

//
// Transform a directional vector by a coordinate system.
// Ignore's the coordinate system's origin.
//
inline FVector FVector::TransformVectorBy(const FCoords &Coords) const{
#if ASM
	FVector Temp;
	ASMTransformVector(Coords, *this, Temp);
	return Temp;
#elif ASMLINUX
	FVector Temp;
	ASMTransformVector(Coords, *this, Temp);
	return Temp;
#elif __PSX2_EE__
	FVector Result;
	FVector UnitVector = FVector(1.0f, 1.0f, 1.0f);
	asm volatile ("
		lqc2		vf1, 0x00(%0)
		lqc2		vf4, 0x00(%1)
		lqc2		vf5, 0x00(%2)
		lqc2		vf6, 0x00(%3)
	"
	:
	: "r" (&UnitVector), "r" (&Coords.XAxis),
	  "r" (&Coords.YAxis), "r" (&Coords.ZAxis)
	);
	asm volatile ("
		/* Register allocation:
		 * vf0: 0, 0, 0, 1 constant vector
		 * vf1: 1, 1, 1, 1 unit vector
		 * vf4: XAxis
		 * vf5: YAxis
		 * vf6: ZAxis
		 */

		/* Load 128bit values. */
		lqc2		vf7,  0x00(%1)

		/* Perform dot products */
		vmul.xyz	vf8xyz, vf7xyz, vf4xyz		/* vf8 <- Temp | XAxis */
		vaddax.x	ACCx, vf0x,	vf8x			/* ACCx = vf8.x */
		vmadday.x	ACCx, vf1x, vf8y			/* ACCx = vf8.x + vf8.y */
		vmaddaz.x	ACCx, vf1x, vf8z			/* ACCx = vf8.x + vf8.y + vf8.z */

		vmul.xyz	vf8xyz, vf7xyz, vf5xyz		/* vf8 <- Temp | YAxis */
		vaddax.y	ACCy, vf0y,	vf8x			/* ACCy = vf8.x */
		vmadday.y	ACCy, vf1y, vf8y			/* ACCy = vf8.x + vf8.y */
		vmaddaz.y	ACCy, vf1y, vf8z			/* ACCy = vf8.x + vf8.y + vf8.z */
			
		vmul.xyz	vf8xyz, vf7xyz, vf6xyz		/* vf8 <- Temp | ZAxis */
		vaddax.z	ACCz, vf0z,	vf8x			/* ACCy = vf8.x */
		vmadday.z	ACCz, vf1z, vf8y			/* ACCy = vf8.x + vf8.y */
		vmaddaz.z	ACCz, vf1z, vf8z			/* ACCy = vf8.x + vf8.y + vf8.z */

		vmadd		vf9, vf0, vf0				/* vf9 = ACC + 0 * 0 */
		sqc2		vf9, 0x00(%0)				/* Result.Point <- ACC */
	"
	:
	: "r" (&Result), "r" (this)
	: "memory"
	);
	return Result;
#else
	return FVector(	*this | Coords.XAxis, *this | Coords.YAxis, *this | Coords.ZAxis);
#endif
}


// Apply 'pivot' transform: First rotate, then add the translation.
// TODO: convert to assembly !
inline FVector FVector::PivotTransform(const FCoords& Coords) const{
	return Coords.Origin + FVector(*this | Coords.XAxis, *this | Coords.YAxis, *this | Coords.ZAxis);
}

//
// Mirror a vector about a normal vector.
//
inline FVector FVector::MirrorByVector(const FVector& MirrorNormal) const{
	return *this - MirrorNormal * (2.0f * (*this | MirrorNormal));
}

//
// Mirror a vector about a plane.
//
inline FVector FVector::MirrorByPlane(const FPlane& Plane) const{
	return *this - Plane * (2.0f * Plane.PlaneDot(*this));
}

/*-----------------------------------------------------------------------------
	FVector friends.
-----------------------------------------------------------------------------*/

//
// Compare two points and see if they're the same, using a threshold.
// Returns 1=yes, 0=no.  Uses fast distance approximation.
//
inline int FPointsAreSame(const FVector &P, const FVector &Q){
	FLOAT Temp;
	Temp=P.X-Q.X;
	if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME)){
		Temp=P.Y-Q.Y;
		if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
		{
			Temp=P.Z-Q.Z;
			if((Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME))
			{
				return 1;
			}
		}
	}
	return 0;
}

//
// Compare two points and see if they're the same, using a threshold.
// Returns 1=yes, 0=no.  Uses fast distance approximation.
//
inline int FPointsAreNear(const FVector &Point1, const FVector &Point2, FLOAT Dist){
	FLOAT Temp;
	Temp=(Point1.X - Point2.X); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.Y - Point2.Y); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.Z - Point2.Z); if (Abs(Temp)>=Dist) return 0;
	return 1;
}

//
// Calculate the signed distance (in the direction of the normal) between
// a point and a plane.
//
inline FLOAT FPointPlaneDist
(
	const FVector &Point,
	const FVector &PlaneBase,
	const FVector &PlaneNormal
){
	return (Point - PlaneBase) | PlaneNormal;
}

//
// Euclidean distance between two points.
//
inline FLOAT FDist(const FVector &V1, const FVector &V2){
	return appSqrt(Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z));
}

//
// Squared distance between two points.
//
inline FLOAT FDistSquared(const FVector &V1, const FVector &V2){
	return Square(V2.X-V1.X) + Square(V2.Y-V1.Y) + Square(V2.Z-V1.Z);
}

//
// See if two normal vectors (or plane normals) are nearly parallel.
//
inline int FParallel(const FVector &Normal1, const FVector &Normal2){
	FLOAT NormalDot = Normal1 | Normal2;
	return (Abs (NormalDot - 1.0f) <= THRESH_VECTORS_ARE_PARALLEL);
}

//
// See if two planes are coplanar.
//
inline int FCoplanar(const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2){
	if      (!FParallel(Normal1,Normal2)) return 0;
	else if (FPointPlaneDist (Base2,Base1,Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
}

//
// Triple product of three vectors.
//
inline FLOAT FTriple(const FVector& X, const FVector& Y, const FVector& Z){
	return
	(	(X.X * (Y.Y * Z.Z - Y.Z * Z.Y))
	+	(X.Y * (Y.Z * Z.X - Y.X * Z.Z))
	+	(X.Z * (Y.X * Z.Y - Y.Y * Z.X)));
}

/*-----------------------------------------------------------------------------
	FPlane implementation.
-----------------------------------------------------------------------------*/

//
// Transform a point by a coordinate system, moving
// it by the coordinate system's origin if nonzero.
//
inline FPlane FPlane::TransformPlaneByOrtho(const FCoords &Coords) const{
	FVector Normal(*this | Coords.XAxis, *this | Coords.YAxis, *this | Coords.ZAxis);
	return FPlane(Normal, W - (Coords.Origin.TransformVectorBy(Coords) | Normal));
}

/*-----------------------------------------------------------------------------
	FCoords functions.
-----------------------------------------------------------------------------*/

//
// Return this coordinate system's transpose.
// If the coordinate system is orthogonal, this is equivalent to its inverse.
//
inline FCoords FCoords::Transpose() const{
	return FCoords
	(
		-Origin.TransformVectorBy(*this),
		FVector(XAxis.X, YAxis.X, ZAxis.X),
		FVector(XAxis.Y, YAxis.Y, ZAxis.Y),
		FVector(XAxis.Z, YAxis.Z, ZAxis.Z)
	);
}

//
// Mirror the coordinates about a normal vector.
//
inline FCoords FCoords::MirrorByVector(const FVector& MirrorNormal) const{
	return FCoords
	(
		Origin.MirrorByVector(MirrorNormal),
		XAxis .MirrorByVector(MirrorNormal),
		YAxis .MirrorByVector(MirrorNormal),
		ZAxis .MirrorByVector(MirrorNormal)
	);
}

//
// Mirror the coordinates about a plane.
//
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

//
// Transform this coordinate system by another coordinate system.
//
inline FCoords& FCoords::operator*=(const FCoords& TransformCoords){
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

//
// Transform this coordinate system by a pitch-yaw-roll rotation.
//
inline FCoords& FCoords::operator*=(const FRotator &Rot){
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

inline FCoords& FCoords::operator*=(const FVector &Point){
	Origin -= Point;
	return *this;
}
inline FCoords FCoords::operator*(const FVector &Point) const{
	return FCoords(*this) *= Point;
}

//
// Detransform this coordinate system by a pitch-yaw-roll rotation.
//
inline FCoords& FCoords::operator/=(const FRotator &Rot){
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

inline FCoords& FCoords::operator/=(const FVector &Point){
	Origin += Point;
	return *this;
}
inline FCoords FCoords::operator/(const FVector &Point) const{
	return FCoords(*this) /= Point;
}

//
// Transform this coordinate system by a scale.
// Note: Will return coordinate system of opposite handedness if
// Scale.X*Scale.Y*Scale.Z is negative.
//
inline FCoords& FCoords::operator*=(const FScale &Scale){
	// Apply sheering.
	FLOAT   Sheer      = FSheerSnap(Scale.SheerRate);
	FCoords TempCoords = GMath.UnitCoords;
	switch(Scale.SheerAxis){
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

//
// Detransform a coordinate system by a scale.
//
inline FCoords& FCoords::operator/=(const FScale &Scale){
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
	switch(Scale.SheerAxis){
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

//
// Compute pushout of a box from a plane.
//
inline FLOAT FBoxPushOut(FVector Normal, FVector Size){
	return Abs(Normal.X*Size.X) + Abs(Normal.Y*Size.Y) + Abs(Normal.Z*Size.Z);
}

//
// Return a uniformly distributed random unit vector.
//
inline FVector VRand(){
	FVector Result;
	do{
		// Check random vectors in the unit sphere so result is statistically uniform.
		Result.X = appFrand()*2 - 1;
		Result.Y = appFrand()*2 - 1;
		Result.Z = appFrand()*2 - 1;
	} while(Result.SizeSquared() > 1.0f);
	return Result.UnsafeNormal();
}

#if 1 //Math added by Legend on 4/12/2000
//
// RandomSpreadVector
// By Paul Du Bois, Infinite Machine
//
// Return a random unit vector within a cone of spread_degrees.
// (Distribution is such that there is no bunching near the axis.)
//
// - Create an FRotator with pitch < spread_degrees/2, yaw = 0, roll = random.
//   The tricky bit is getting the probability distribution of pitch
//   correct.  If it's flat, particles will tend to cluster around the pole.
//
// - For a given pitch phi, the probability p(phi) should be proportional to
//   the surface area of the slice of sphere defined by that pitch and a
//   random roll.  This is 2 pi (r sin phi) (r d phi) =~ sin phi
//
// - To map a flat distribution (FRand) to our new p(phi), we first find the
//   CDF (cumulative distribution fn) which is basically the integral of
//   p(phi) over its domain, normalized to have a total area of 1.  Smaller
//   spreads are modeled by limiting the domain of p(phi) and normalizing
//   the CDF for the smaller domain.  The CDF is always monotonically
//   non-decreasing and has range [0,1].
//
// - The insight is that the inverse of the CDF is exactly what we need to
//   convert a flat distribution to our p(phi).  aCDF: [0,1] -> angle.
//   (insert handwaving argument)
//
// - CDF(x) = integral(0->x) sin phi d phi
//          = -cos(x) - -cos(0)
//          = K * (1 - cos(x)) (K is normalizing factor to make CDF(domain_max)=1)
//        K = 1/max_cos_val   max_cos_val = (1 - cos(max pitch))
//
// - aCDF(y) = acos(1-y/K) = acos(1-y*max_cos_val)
//
inline FVector RandomSpreadVector(FLOAT spread_degrees){
    FLOAT max_pitch = Clamp(spread_degrees * (PI / 180.0f / 2.0f),0.0f,180.0f);
    FLOAT K = 1.0f - appCos(max_pitch);
    FLOAT pitch = appAcos(1.0f - appFrand()*K);  // this is the aCDF
    FLOAT rand_roll = appFrand() * (2.0f * PI);
    FLOAT radius = appSin(pitch);
    return FVector(appCos(pitch),radius*appSin(rand_roll),radius*appCos(rand_roll));
}
#endif

/*-----------------------------------------------------------------------------
	Advanced geometry.
-----------------------------------------------------------------------------*/

//
// Find the intersection of an infinite line (defined by two points) and
// a plane.  Assumes that the line and plane do indeed intersect; you must
// make sure they're not parallel before calling.
//
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

/*-----------------------------------------------------------------------------
	FPlane functions.
-----------------------------------------------------------------------------*/

//
// Compute intersection point of three planes.
// Return 1 if valid, 0 if infinite.
//
inline UBOOL FIntersectPlanes3(FVector& I, const FPlane& P1, const FPlane& P2, const FPlane& P3){
	guard(FIntersectPlanes3);

	// Compute determinant, the triple product P1|(P2^P3)==(P1^P2)|P3.
	FLOAT Det = (P1 ^ P2) | P3;
	if(Square(Det) < Square(0.001f)){
		// Degenerate.
		I = FVector(0,0,0);
		return 0;
	}
	else{
		// Compute the intersection point, guaranteed valid if determinant is nonzero.
		I = (P1.W*(P2^P3) + P2.W*(P3^P1) + P3.W*(P1^P2)) / Det;
	}
	return 1;
	unguard;
}

//
// Compute intersection point and direction of line joining two planes.
// Return 1 if valid, 0 if infinite.
//
inline UBOOL FIntersectPlanes2(FVector& I, FVector& D, const FPlane& P1, const FPlane& P2){
	guard(FIntersectPlanes2);

	// Compute line direction, perpendicular to both plane normals.
	D = P1 ^ P2;
	FLOAT DD = D.SizeSquared();
	if(DD < Square(0.001f)){
		// Parallel or nearly parallel planes.
		D = I = FVector(0,0,0);
		return 0;
	}
	else{
		// Compute intersection.
		I = (P1.W*(P2^D) + P2.W*(D^P1)) / DD;
		D.Normalize();
		return 1;
	}
	unguard;
}

/*-----------------------------------------------------------------------------
	FRotator functions.
-----------------------------------------------------------------------------*/

//
// Convert a rotation into a vector facing in its direction.
//
inline FVector FRotator::Vector(){
	return (GMath.UnitCoords / *this).XAxis;
}


/*-----------------------------------------------------------------------------
	FMatrix.          
-----------------------------------------------------------------------------*/
// Floating point 4 x 4  (4 x 3)  KNI-friendly matrix
class CORE_API FMatrix{
public:

	// Variables.
	FPlane XPlane; // each plane [x,y,z,w] is a *column* in the matrix.
	FPlane YPlane;
	FPlane ZPlane;
	FPlane WPlane;

	FLOAT& M(INT i,INT j) {return ((FLOAT*)&XPlane)[i*4+j];}
	const FLOAT& M(INT i,INT j) const {return ((FLOAT*)&XPlane)[i*4+j];}

	// Constructors.
	FMatrix(){}
	FMatrix(FPlane InX, FPlane InY, FPlane InZ)
	:	XPlane(InX), YPlane(InY), ZPlane(InZ), WPlane(0,0,0,0){}
	FMatrix(FPlane InX, FPlane InY, FPlane InZ, FPlane InW)
	:	XPlane(InX), YPlane(InY), ZPlane(InZ), WPlane(InW){}


	// Regular transform
	FVector TransformFVector(const FVector &V) const{
		FVector FV;

		FV.X = V.X * M(0,0) + V.Y * M(0,1) + V.Z * M(0,2) + M(0,3);
		FV.Y = V.X * M(1,0) + V.Y * M(1,1) + V.Z * M(1,2) + M(1,3);
		FV.Z = V.X * M(2,0) + V.Y * M(2,1) + V.Z * M(2,2) + M(2,3);

		return FV;
	}

	// Homogeneous transform
	FPlane TransformFPlane(const FPlane &P) const{
		FPlane FP;

		FP.X = P.X * M(0,0) + P.Y * M(0,1) + P.Z * M(0,2) + M(0,3);
		FP.Y = P.X * M(1,0) + P.Y * M(1,1) + P.Z * M(1,2) + M(1,3);
		FP.Z = P.X * M(2,0) + P.Y * M(2,1) + P.Z * M(2,2) + M(2,3);
		FP.W = P.X * M(3,0) + P.Y * M(3,1) + P.Z * M(3,2) + M(3,3);

		return FP;
	}

	FQuat FMatrixToFQuat();

	// Combine transforms binary operation MxN
	friend FMatrix CombineTransforms(const FMatrix& M, const FMatrix& N);
	friend FMatrix FMatrixFromFCoords(const FCoords& FC);
	friend FCoords FCoordsFromFMatrix(const FMatrix& FM);

};

FMatrix CombineTransforms(const FMatrix& M, const FMatrix& N);

// Conversions for Unreal1 coordinate system class.

inline FMatrix FMatrixFromFCoords(const FCoords& FC) {
	FMatrix M;
	M.XPlane = FPlane(FC.XAxis.X, FC.XAxis.Y, FC.XAxis.Z, FC.Origin.X);
	M.YPlane = FPlane(FC.YAxis.X, FC.YAxis.Y, FC.YAxis.Z, FC.Origin.Y);
	M.ZPlane = FPlane(FC.ZAxis.X, FC.ZAxis.Y, FC.ZAxis.Z, FC.Origin.Z);
	M.WPlane = FPlane(0.0f,        0.0f,        0.0f,        1.0f        );
	return M;
}

inline FCoords FCoordsFromFMatrix(const FMatrix& FM){
	FCoords C;
	C.Origin = FVector(FM.XPlane.W, FM.YPlane.W, FM.ZPlane.W);
	C.XAxis  = FVector(FM.XPlane.X, FM.XPlane.Y, FM.XPlane.Z);
	C.YAxis  = FVector(FM.YPlane.X, FM.YPlane.Y, FM.YPlane.Z);
	C.ZAxis  = FVector(FM.ZPlane.X, FM.ZPlane.Y, FM.ZPlane.Z);
	return C;
}



/*-----------------------------------------------------------------------------
	FQuat.          
-----------------------------------------------------------------------------*/

// floating point quaternion.
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
			X*Q.X - Y*Q.Y - Z*Q.Z - W*Q.W, 
			X*Q.Y + Y*Q.X + Z*Q.W - W*Q.Z, 
			X*Q.Z - Y*Q.W + Z*Q.X + W*Q.Y, 
			X*Q.W + Y*Q.Z - Z*Q.Y + W*Q.X
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
	
	UBOOL Normalize(){
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
		else 
		{	
			X = 0.0f;
			Y = 0.0f;
			Z = 0.1f;
			W = 0.0f;
			return false;
		}
	}

	// Serializer.
	friend FArchive& operator<<(FArchive& Ar, FQuat& F){
		return Ar << F.X << F.Y << F.Z << F.W;
	}

	FMatrix FQuatToFMatrix();

	// Warning : assumes normalized quaternions.
	FQuat FQuatToAngAxis(){
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
		else 
		{
			A.X = 0.0f;
			A.Y = 0.0f;
			A.Z = 1.0f;
			A.W = 0.0f; 
		}

		return A;
	};

	//
	// Angle-Axis to Quaternion. No normalized axis assumed.
	//
	FQuat AngAxisToFQuat(){
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
};


// Dot product of axes to get cos of angle  #Warning some people use .W component here too !
inline FLOAT FQuatDot(const FQuat& Q1,const FQuat& Q2){
	return(Q1.X*Q2.X + Q1.Y*Q2.Y + Q1.Z*Q2.Z);
};

// Error measure (angle) between two quaternions, ranged [0..1]
inline FLOAT FQuatError(FQuat& Q1,FQuat& Q2){
	// Returns the hypersphere-angle between two quaternions; alignment shouldn't matter, though 
	// normalized input is expected.
	FLOAT cosom = Q1.X*Q2.X + Q1.Y*Q2.Y + Q1.Z*Q2.Z + Q1.W*Q2.W;
	return (Abs(cosom) < 0.9999999f) ? appAcos(cosom)*(1.0f/PI) : 0.0f;
}

// Ensure quat1 points to same side of the hypersphere as quat2
inline void AlignFQuatWith(FQuat &quat1, const FQuat &quat2){
	FLOAT Minus  = Square(quat1.X-quat2.X) + Square(quat1.Y-quat2.Y) + Square(quat1.Z-quat2.Z) + Square(quat1.W-quat2.W);
	FLOAT Plus   = Square(quat1.X+quat2.X) + Square(quat1.Y+quat2.Y) + Square(quat1.Z+quat2.Z) + Square(quat1.W+quat2.W);

	if (Minus > Plus){
		quat1.X = - quat1.X;
		quat1.Y = - quat1.Y;
		quat1.Z = - quat1.Z;
		quat1.W = - quat1.W;
	}
}

// No-frills spherical interpolation. Assumes aligned quaternions, and the output is not normalized.
inline FQuat SlerpQuat(const FQuat &quat1,const FQuat &quat2, float slerp){
	FQuat result;
	float omega,cosom,sininv,scale0,scale1;

	// Get cosine of angle betweel quats.
	cosom = quat1.X * quat2.X +
			quat1.Y * quat2.Y +
			quat1.Z * quat2.Z +
			quat1.W * quat2.W;

	if(cosom < 0.99999999f){	
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
	else{
		return quat1;
	}
	
}

/*
*	New Stuff for Republic Commando
*/

struct FCompressedPosition{
	FVector Location;
	FRotator Rotation;
	FVector Velocity;
};

// Used to generate random values between Min and Max
struct FRange{
	FLOAT Min;
	FLOAT Max;
};

// Vector of Ranges
struct FRangeVector{
	FRange X;
	FRange Y;
	FRange Z;
};

struct FIntBox{
    int X1, Y1, X2, Y2;
};

struct FFloatBox{
    float X1, Y1, X2, Y2;
};

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
