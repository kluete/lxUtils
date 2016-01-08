// LX color

#pragma once

#include <cstdint>
#include <memory>
#include <algorithm>

#if LX_JUCE
	#include "JuceHeader.h"
#endif

#if LX_WX
	class wxColour;
#endif

namespace LX
{
using std::uint32_t;
using std::unique_ptr;

//---- rgb colors -------------------------------------------------------------

enum class RGB_COLOR : uint32_t
{
	BLACK		= 0x000000FFul,
	WHITE		= 0xFFFFFFFFul,
	NO_COLOR	= 0x00000000ul,
	TRANSPARENT	= 0x00000000ul,

	SOFT_BLACK	= 0x0A0A0AFFul,
	DARK_GREY	= 0x404040FFul,
	GREY		= 0x808080FFul,
	BRIGHT_GREY	= 0xA0A0AFFFul,
	MID_GREY	= 0xCCCCCFFFul,
	LIGHT_GREY	= 0xF4F4F6FFul,
	GTK_GREY	= 0xEEEEEFFFul,

	RED		= 0xFF0000FFul,
	DARK_RED	= 0xC00000FFul,
	BRIGHT_RED	= 0xFF1414FFul,
	CRIMSON_RED	= 0xDC143CFFul,		// (permanent insurance)
	LIGHT_RED	= 0xFFF0F0FFul,
	NIGHT_RED	= 0x800000FFul,
	
	GREEN		= 0x00FF00FFul,
	MID_GREEN	= 0x00A000FFul,
	DARK_GREEN	= 0x008000FFul,
	NIGHT_GREEN	= 0x005000FFul,
	LIGHT_GREEN	= 0x80FF80FFul,
	PALE_GREEN	= 0xA0FFA0FFul,
	WAFER_GREEN	= 0xC0FFC0FFul,
	BRIGHT_GREEN	= 0x14FF96FFul,

	BLUE		= 0x0000FFFFul,
	LIGHT_BLUE	= 0xECF0FFFFul,
	WAFER_BLUE	= 0xF8F8FFFFul,		// ("it's wafer-thin")
	DARK_BLUE	= 0x0000C0FFul,
	NIGHT_BLUE	= 0x000080FFul,
	MID_BLUE	= 0x8080FFFFul,
	BRIGHT_BLUE	= 0x1496FFFFul,
	PALE_BLUE	= 0x3686C9FFul,

	ORANGE		= 0xF0B000FFul,
	DARK_ORANGE	= 0x906000FFul,
	YELLOW		= 0xF0C000FFul,
	STRONG_YELLOW	= 0xFFF050FFul,
	ORANGY_YELLOW	= 0xFFC814FFul,
	BRIGHT_YELLOW	= 0xFFB414FFul,

	CYAN		= 0x00A0A0FFul,
	MID_CYAN	= 0x14FFFFFFul,
	BRIGHT_CYAN	= 0x58ACEBFFul,

	PURPLE		= 0x7C46E0FFul,
	DARK_PURPLE	= 0x800080FFul,
	NIGHT_PURPLE	= 0x500050FFul,
	PINK		= 0xFF00E7FFul,
	BRIGHTPINK	= 0xFF92E0FFul,
	
	KAKI		= 0x99C700FFul,
	BROWN		= 0xC75D00FFul,
	DARK_BROWN	= 0x632E00FFul,
	
	BAR_OUTLINE		= 0x898999fful,
	BAR_GRADIENT_TOP	= 0xfbfbfcfful,
	BAR_GRADIENT_BOTTOM	= 0xd4d4dffful,
	BAR_MEDIAN		= 0xe8e8effful,
	BAR_BEVEL_NORTH_WEST	= 0xfffffffful,
	BAR_BEVEL_SOUTH_EAST	= 0xc4c4cffful,
};

//---- Color ------------------------------------------------------------------

class Color
{
	static constexpr double	one_over_255 = 1.0 / 255.0;
	
	static constexpr
	double	clamp01(const double v) noexcept
	{
		// return std::max(0.0, std::min(1.0, v));		// g++ 4.9.1 says "not a constexpr"
		return (v < 0.0) ? 0.0 : ((v > 1.0) ? 1.0 : v);
	}
	
	static constexpr
	int	comp(const uint32_t rgba32, const int sh) noexcept
	{
		return (rgba32 >> (sh * 8)) & 0xff;
	}
	
public:
	// ctors
	explicit constexpr
	Color(const double r_, const double g_, const double b_, const double a_ = 1.0) noexcept
		: m_r(clamp01(r_)), m_g(clamp01(g_)), m_b(clamp01(b_)), m_a(clamp01(a_))		// (auto-clamped)
	{
	}
	
	explicit constexpr
	Color(const int r_, const int g_, const int b_, const int a_ = 255) noexcept
		: Color(r_ * one_over_255, g_ * one_over_255, b_ * one_over_255, a_ * one_over_255)
	{
	}
	
	constexpr
	Color(const RGB_COLOR rgba_enum) noexcept
		: Color(FromRGBA32(static_cast<uint32_t>(rgba_enum)))
	{
	}
	
	static constexpr
	Color	FromRGBA32(const uint32_t rgba32) noexcept
	{
		#if 0
			// g++ 4.9.1 doesn't support full c++14 constexpr ? (works fine in clang)
			const int	ri(comp(rgba32, 3));
			const int	gi(comp(rgba32, 2));
			const int	bi(comp(rgba32, 1));
			const int	ai(comp(rgba32, 0));
			
			return Color(ri, gi, bi, ai);
		#else
			return Color(	comp(rgba32, 3),
					comp(rgba32, 2),
					comp(rgba32, 1),
					comp(rgba32, 0));
					
		#endif
	}
	
	// re-enable vanilla ctors
	Color(const Color&) = default;
	Color& operator=(const Color&) = default;
	
	constexpr
	Color	Clamped(void) const noexcept
	{
		return Color(clamp01(m_r), clamp01(m_g), clamp01(m_b), clamp01(m_a));
	}

	double	r(void) const	{return m_r;}
	double	g(void) const	{return m_g;}
	double	b(void) const	{return m_b;}
	double	a(void) const	{return m_a;}
	
	uint32_t	ToRGBA32(void) const noexcept;
	
	bool	empty(void) const noexcept	{return (m_a <= 0.0);}
	
	bool	operator==(const Color &o) const noexcept
	{
		return (r() == o.r()) && (g() == o.g()) && (b() == o.b()) && (a() == o.a());			// add error tolerance?
	}
	
	bool	operator!=(const Color &o) const noexcept
	{
		return !operator==(o);
	}
	
	static constexpr
	Color	Grey(const double &lum)	noexcept
	{
		return Color(lum, lum, lum, 1.0);
	}
	
	Color	ToWhite(double fact) const;
	Color	Scale(double fact) const;
	Color	Scaled(double rf, double gf, double bf) const;
	Color	ChangeLightness(const double &lum_perc) const;
	Color	Mix(const Color &o, const double a) const;
	
	Color	with_r(const double &r_) const;
	Color	with_g(const double &g_) const;
	Color	with_b(const double &b_) const;
	Color	with_a(const double &a_) const;
	
	double	Norm(void) const;
	Color	Normalized(const double &scale) const;
	Color	Inverted(void) const;
	
	// wrappers
	Color	Brighter(const double scale) const	{return ChangeLightness(scale);}
	Color	Darker(const double scale) const	{return ChangeLightness(scale);}
	
	#if LX_WX
		static Color		FromWxColor(const wxColour &wxc);
		unique_ptr<wxColour>	ToWxColor(void) const;			// avoid full wx header include, isn't speed-critical so alloc ok
	#endif
	
	#if LX_JUCE
		static Color	FromJuceColor(const juce::Colour &jclr);
		juce::Colour	ToJuceColor(void) const;
		
		// automatic conversion
		operator juce::Colour() const	{return ToJuceColor();}
	#endif
	
private:
	
	double	m_r, m_g, m_b, m_a;
};

} // namespace LX

// nada mas
