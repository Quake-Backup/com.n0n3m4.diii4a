/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __DECLTABLE_H__
#define __DECLTABLE_H__

/*
===============================================================================

	tables are used to map a floating point input value to a floating point
	output value, with optional wrap / clamp and interpolation

===============================================================================
*/

class idDeclTable : public idDecl
{
	public:
		virtual size_t			Size(void) const;
		virtual const char 	*DefaultDefinition(void) const;
#ifdef _RAVEN
		virtual bool			Parse(const char *text, const int textLength, bool noCaching = false);
#else
		virtual bool			Parse(const char *text, const int textLength);
#endif
		virtual void			FreeData(void);

#ifdef _RAVEN
        float			GetMaxValue( void ) const { return( maxValue ); }
        float			GetMinValue( void ) const { return( minValue ); }
        // bdube: made virtual so it can be accessed in game
        virtual
#endif
#ifdef _HUMANHEAD
		virtual	//HUMANHEAD pdm: made virtual so it can be called from game code
#endif
		float					TableLookup(float index) const;

	private:
		bool					clamp;
		bool					snap;
		idList<float>			values;

#ifdef _RAVEN
// RAVEN BEGIN
// jscott: for BSE
	float					minValue;
	float					maxValue;
// RAVEN END
#endif
};

#endif /* !__DECLTABLE_H__ */
