#pragma once

// Fixed-point type
union fixed
{
	struct
	{
		u8 FractionalPart;
		int WholePart : 24;
	};
	s32 RawValue;

	constexpr fixed() : RawValue(0) {}

	constexpr fixed(int Int) : FractionalPart(0), WholePart(Int) {}

	constexpr fixed(int Whole, int Fraction) : FractionalPart(Fraction), WholePart(Whole) {}

	constexpr fixed(double Double) : RawValue((int)(Double * (1 << 8))) {}

	inline void operator+=(fixed Other)
	{
		RawValue += Other.RawValue;
	}

	inline void operator-=(fixed Other)
	{
		RawValue -= Other.RawValue;
	}

	inline void operator+=(int Other)
	{
		WholePart += Other;
	}

	inline void operator=(int Other)
	{
		WholePart = Other;
		FractionalPart = 0;
	}

	inline void operator*=(fixed Other)
	{
		RawValue = (RawValue * Other.RawValue) >> 8;
	}

	inline b32 operator==(fixed Other)
	{
		b32 Result = RawValue == Other.RawValue;
		return Result;
	}

	inline b32 operator!=(fixed Other)
	{
		b32 Result = !(*this == Other);
		return Result;
	}
};

static constexpr fixed SQRT_2 = fixed(1.4142135623730950488016887242097);
static constexpr fixed INV_SQRT_2 = fixed(0.70710678118654752440084436210485);

inline constexpr fixed operator+(fixed A, fixed B)
{
	fixed Result;
	Result.RawValue = A.RawValue + B.RawValue;
	return Result;
}

inline constexpr fixed operator-(fixed A, fixed B)
{
	fixed Result;
	Result.RawValue = A.RawValue - B.RawValue;
	return Result;
}

inline constexpr fixed operator*(fixed A, fixed B)
{
	fixed Result;
	Result.RawValue = (A.RawValue * B.RawValue) >> 8;
	return Result;
}

inline constexpr fixed operator+(fixed A, int Int)
{
	fixed Result = A;
	Result.WholePart += Int;
	return Result;
}

inline constexpr fixed operator-(fixed A, int Int)
{
	fixed Result = A;
	Result.WholePart -= Int;
	return Result;
}

inline constexpr fixed operator*(fixed A, int Int)
{
	fixed Result = A;
	Result.WholePart *= Int;
	return Result;
}

inline constexpr fixed operator*(int Int, const fixed B)
{
	fixed Result = B;
	Result.WholePart *= Int;
	return Result;
}

inline constexpr int Round(fixed A)
{
	s32 RawHalf = (1 >> 9);
	s32 RawFraction = (A.FractionalPart << 8);

	if (RawFraction >= RawHalf)
	{
		return A.WholePart + 1;
	}
	return A.WholePart;
}

inline constexpr b32 operator>(fixed A, fixed B)
{
	return A.RawValue > B.RawValue;
}

inline constexpr b32 operator>=(fixed A, fixed B)
{
	return A.RawValue >= B.RawValue;
}

inline constexpr b32 operator<=(fixed A, fixed B)
{
	return A.RawValue <= B.RawValue;
}

inline constexpr b32 operator>(fixed A, int Int)
{
	if (A.WholePart > Int)
	{
		return true;
	}
	else if (A.WholePart == Int)
	{
		return A.FractionalPart > 0;
	}
	return false;
}

inline constexpr b32 operator==(fixed A, int Int)
{
	b32 Result = A.WholePart == Int && A.FractionalPart == 0;
	return Result;
}

inline constexpr fixed operator-(fixed A)
{
	fixed Result;
	Result.RawValue = -A.RawValue;
	return Result;
}

inline constexpr b32 operator<(fixed A, fixed B)
{
	return A.RawValue < B.RawValue;
}

inline constexpr b32 operator<(fixed A, int Int)
{
	b32 Result = A < fixed(Int);
	return Result;
}

inline constexpr fixed operator>>(fixed A, int Int)
{
	fixed Result;
	Result.RawValue = A.RawValue >> Int;
	return Result;
}

inline constexpr fixed FMin(fixed A, fixed B)
{
	return A < B ? A : B;
}

inline constexpr fixed FMax(fixed A, fixed B)
{
	return A > B ? A : B;
}

inline constexpr fixed FClamp(fixed Value, fixed Min, fixed Max)
{
	fixed Result = Value;
	if (Value < Min)
	{
		Result = Min;
	}
	else if (Value > Max)
	{
		Result = Max;
	}
	return Result;
}

inline constexpr fixed Abs(fixed Value)
{
	if (Value < 0)
	{
		Value.RawValue = -Value.RawValue;
	}
	return Value;
}

struct v2
{
	fixed X;
	fixed Y;

	inline void operator+=(v2 Other)
	{
		X += Other.X;
		Y += Other.Y;
	}

	inline b32 operator==(v2 Other)
	{
		b32 Result = X == Other.X && Y == Other.Y;
		return Result;
	}

	inline b32 operator!=(v2 Other)
	{
		b32 Result = !(*this == Other);
		return Result;
	}

	inline void operator*=(fixed Num)
	{
		X *= Num;
		Y *= Num;
	}

	inline void operator-=(v2 Other)
	{
		X -= Other.X;
		Y -= Other.Y;
	}
};

inline constexpr v2 operator+(v2 A, v2 B)
{
	v2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

inline constexpr v2 operator-(v2 A, v2 B)
{
	v2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

inline constexpr fixed SqMagnitude(v2 V)
{
	return V.X * V.X + V.Y * V.Y;
}

inline constexpr v2 operator*(fixed A, v2 V)
{
	v2 Result;
	Result.X = V.X * A;
	Result.Y = V.Y * A;
	return Result;
}

inline constexpr v2 operator*(v2 V, int Int)
{
	v2 Result;
	Result.X = V.X * Int;
	Result.Y = V.Y * Int;
	return Result;
}

inline constexpr v2 operator>>(v2 V, const u32 Int)
{
	v2 Result;
	Result.X.RawValue = V.X.RawValue >> Int;
	Result.Y.RawValue = V.Y.RawValue >> Int;
	return Result;
}

inline constexpr v2 Lerp(v2 A, v2 B, fixed t)
{
	v2 Result = t * B + (1 - t) * A;
	return Result;	
}

struct lerp_data
{
	v2 StartPos;
	v2 TargetPos;
	fixed Step;
	fixed t;
	u32 TargetNumFrames;
	u32 FramesLerped;
};

union iv2
{
	struct
	{
		s32 X;
		s32 Y;
	};
	struct
	{
		s32 Width;
		s32 Height;
	};

	constexpr iv2() : X(0), Y(0) {}
	constexpr iv2(v2 V) : X(V.X.WholePart), Y(V.Y.WholePart) {}
	constexpr iv2(s32 InX, s32 InY) : X(InX), Y(InY) {}

	inline void operator+=(iv2 Other)
	{
		X += Other.X;
		Y += Other.Y;
	}

	inline void operator*=(iv2 Other)
	{
		X *= Other.X;
		Y *= Other.Y;
	}

	inline void operator*=(int Int)
	{
		X *= Int;
		Y *= Int;
	}

	inline b32 operator==(iv2 Other)
	{
		b32 Result = X == Other.X && Y == Other.Y;
		return Result;
	}

	inline b32 operator!=(iv2 Other)
	{
		b32 Result = !(*this == Other);
		return Result;
	}

	inline void operator/=(s32 Int)
	{
		X /= Int;
		Y /= Int;
	}
};

inline constexpr iv2 operator+(iv2 A, iv2 B)
{
	iv2 Result = {};
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}

inline constexpr iv2 operator-(iv2 A, iv2 B)
{
	iv2 Result;
	Result.X = A.X - B.X;
	Result.Y = A.Y - B.Y;
	return Result;
}

inline constexpr iv2 operator>>(iv2 V, s32 Int)
{
	iv2 Result;
	Result.X = V.X >> Int;
	Result.Y = V.Y >> Int;
	return Result;
}

inline constexpr v2 operator+(v2 A, iv2 B)
{
	v2 Result;
	Result.X = A.X + B.X;
	Result.Y = A.Y + B.Y;
	return Result;
}