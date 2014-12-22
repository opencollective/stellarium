/*
 * Stellarium Scenery3d Plug-in
 *
 * Copyright (C) 2014 Simon Parzer, Peter Neubauer, Georg Zotti, Andrei Borza, Florian Schaukowitsch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
 
 
/*
This is the fragment shader for vertex lighting, which does not need to do many things
*/
 
#version 120

#define MAT_DIFFUSETEX 1
#define ALPHATEST 1

#if MAT_DIFFUSETEX
uniform sampler2D u_texDiffuse;
#endif

#if ALPHATEST
uniform float u_fAlphaThresh;
#endif

varying vec2 v_texcoord;
varying vec4 v_illumination;

void main(void)
{
#if MAT_DIFFUSETEX
	vec4 texVal = texture2D(u_texDiffuse,v_texcoord);
#if ALPHATEST
	if(texVal.a < u_fAlphaThresh)
		discard;
#endif
	gl_FragColor = v_illumination * texVal;
#else
	gl_FragColor = v_illumination;
#endif
}