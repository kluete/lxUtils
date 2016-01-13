// lx color

#include <cmath>
#include <algorithm>

#if LX_WX
	#include "wx/colour.h"
#endif

#include "lx/color.h"

using namespace std;
using namespace LX;

uint32_t	Color::ToRGBA32(void) const noexcept
{
	const uint32_t	ri = m_r * 255.0;
	const uint32_t	gi = m_g * 255.0;
	const uint32_t	bi = m_b * 255.0;
	const uint32_t	ai = m_a * 255.0;
		
	return (ri << 24) | (gi << 16) | (bi << 8) | ai;
}

Color	Color::with_r(const double &r_) const
{
	return Color(r_, g(), b(), a());
}

Color	Color::with_g(const double &g_) const
{
	return Color(r(), g_, b(), a());
}

Color	Color::with_b(const double &b_) const
{
	return Color(r(), g(), b_, a());
}

Color	Color::with_a(const double &a_) const
{
	return Color(r(), g(), b(), a_);
}

Color	Color::ToWhite(double fact) const
{
	if (fact <= 0.0)	return Color(*this);			// identity
	if (fact >= 1.0)	return Color(1.0, 1.0, 1.0, a());	// white
	
	const double	dr = 1.0 - r();
	const double	dg = 1.0 - g();
	const double	db = 1.0 - b();
	
	return Color(	r() + (dr * fact),
			g() + (dg * fact),
			b() + (db * fact),
			a());
}

Color	Color::Inverted(void) const
{
	return Color(1 - m_r, 1 - m_g, 1 - m_b, 1.0);
}

Color	Color::Scale(double fact) const
{
	if (fact <= 0.0)	return Color(RGB_COLOR::BLACK);
	if (fact >= 1.0)	return Color(*this);	// identity
	
	return Color(r() * fact, g() * fact, b() *fact, a());
}

Color	Color::Scaled(double rf, double gf, double bf) const
{
	return Color(r() * rf, g() * gf, b() * bf, a());
}

Color	Color::ChangeLightness(const double &lum_perc) const
{
	if (lum_perc <= 100)	return Scale(lum_perc / 100.0);
	else			return ToWhite((lum_perc - 100.0) / 100.0);
}

double	Color::Norm(void) const
{
	return sqrt((r()*r()) + (g()*g()) + (b()*b()));
}

Color	Color::Normalized(const double &scale) const
{
	const double	norm = Norm();
	if (norm == 0.0)	return *this;
	
	const double	f = scale / norm;
	
	return Color(r() * f, g() * f, b() * f, 1.0);
}

Color	Color::Mix(const Color &o, const double mix) const
{
	const double	r2 = m_r + ((o.r() - m_r) * mix);
	const double	g2 = m_g + ((o.g() - m_g) * mix);
	const double	b2 = m_b + ((o.b() - m_b) * mix);
	const double	a2 = m_a + ((o.a() - m_a) * mix);
	
	return Color(r2, g2, b2, a2);
}

//---- JUCE glue --------------------------------------------------------------

#if LX_JUCE

	// static
	Color	Color::FromJuceColor(const juce::Colour &jclr)
	{
		const Color	clr(jclr.getRed(), jclr.getGreen(), jclr.getBlue(), jclr.getAlpha());
		
		return clr;
	}

	juce::Colour	Color::ToJuceColor(void) const
	{
		const uint8_t	ri = m_r * 255.0;
		const uint8_t	gi = m_g * 255.0;
		const uint8_t	bi = m_b * 255.0;
		const uint8_t	ai = m_a * 255.0;
		
		return juce::Colour(ri, gi, bi, ai);
	}

#endif // LX_JUCE

//---- wxWidgets glue ---------------------------------------------------------

#if LX_WX

	// static
	Color	Color::FromWxColor(const wxColour &wxc)
	{	
		const Color	clr(wxc.Red(), wxc.Green(), wxc.Blue(), 255);
	
		return clr.Clamped();
	}

	// return unique_ptr
	unique_ptr<wxColour>	Color::ToWxColor(void) const
	{
		return make_unique<wxColour>(r() * 255.0, g() * 255.0, b() * 255.0, a() * 255.0);
	}

#endif // LX_WX

// nada mas
