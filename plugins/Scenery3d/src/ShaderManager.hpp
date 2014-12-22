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

#ifndef _SHADERMANAGER_HPP_
#define _SHADERMANAGER_HPP_
#include "OBJ.hpp"
#include "StelOpenGL.hpp"

#include <QMap>

class QOpenGLShaderProgram;

//! A structure for global shader parameters
struct GlobalShaderParameters
{
	bool pixelLighting;
	bool bump;
	bool shadows;
	bool shadowFilter;
	bool shadowFilterHQ;
};

//! A simple shader cache class that gives us the correct shader depending on desired configuration.
//! It also contains a uniform cache to avoid unnecessary glUniformLocation calls
class ShaderMgr
{
public:
	ShaderMgr();
	~ShaderMgr();

	//! Enum for OpenGL shader attribute locations
	enum ATTLOC
	{
		//! This is the OpenGL attribute location where vertex positions are mapped to
		ATTLOC_VERTEX,
		//! This is the OpenGL attribute location where vertex normals are mapped to
		ATTLOC_NORMAL,
		//! This is the OpenGL attribute location where vertex texture coordinates are mapped to
		ATTLOC_TEXTURE,
		//! This is the OpenGL attribute location where vertex tangents are mapped to
		ATTLOC_TANGENT,
		//! This is the OpenGL attribute location where vertex bitangents are mapped to
		ATTLOC_BITANGENT
	};

	//! Enum for OpenGL shader uniform locations (faster than accessing by string each time)
	enum UNIFORM
	{
		//! Defines the ModelView matrix
		UNIFORM_MAT_MODELVIEW,
		//! Defines the Projection matrix
		UNIFORM_MAT_PROJECTION,
		//! Defines the combined ModelViewProjection matrix, used if shader needs no separation of the 2 for a bit of a speedup
		UNIFORM_MAT_MVP,
		//! Defines the 3x3 Normal matrix (transpose of the inverse MV matrix)
		UNIFORM_MAT_NORMAL,
		//! The first shadow matrix
		UNIFORM_MAT_SHADOW0,
		UNIFORM_MAT_SHADOW1,
		UNIFORM_MAT_SHADOW2,
		UNIFORM_MAT_SHADOW3,

		//! Defines the Diffuse texture slot
		UNIFORM_TEX_DIFFUSE,
		//! Defines the Bump texture slot
		UNIFORM_TEX_BUMP,
		//! Defines the Height texture slot
		UNIFORM_TEX_HEIGHT,
		//! First shadow map
		UNIFORM_TEX_SHADOW0,
		UNIFORM_TEX_SHADOW1,
		UNIFORM_TEX_SHADOW2,
		UNIFORM_TEX_SHADOW3,

		//! Material ambient color
		UNIFORM_MTL_AMBIENT,
		//! Material diffuse color
		UNIFORM_MTL_DIFFUSE,
		//! Material specular color
		UNIFORM_MTL_SPECULAR,
		//! Material specular shininess (exponent)
		UNIFORM_MTL_SHININESS,
		//! Material global transparency
		UNIFORM_MTL_ALPHA,

		//! Light direction vector (world space)
		UNIFORM_LIGHT_DIRECTION,
		//! Light direction vector (view space)
		UNIFORM_LIGHT_DIRECTION_VIEW,
		//! Light ambient intensity
		UNIFORM_LIGHT_AMBIENT,
		//! Light diffuse intensity
		UNIFORM_LIGHT_DIFFUSE,

		//! Squared frustum splits (vec4)
		UNIFORM_VEC_SQUAREDSPLITS,
		//! Alpha test threshold
		UNIFORM_FLOAT_ALPHA_THRESH,
	};



	//! Returns a shader that supports the specified operations. Must be called within a GL context.
	inline QOpenGLShaderProgram* getShader(const GlobalShaderParameters &globals, const OBJ::Material *mat = NULL);

	//! Returns a shader that can only transform geometry, nothing else
	inline QOpenGLShaderProgram* getTransformShader();

	//! Returns the location of this uniform for this shader, or -1 if this uniform does not exist.
	//! This is cached to elimate the overhead of the glGet calls
	inline GLint uniformLocation(const QOpenGLShaderProgram* shad,UNIFORM uni);

	//! Clears the shaders that have been created by this manager. Must be called within a GL context.
	void clearCache();

private:
	typedef QMap<QString,UNIFORM> t_UniformStrings;
	static t_UniformStrings uniformStrings;

	//A Bitflag enum which contains features that shaders can implement, and which this manager can dynamically select
	enum FeatureFlags
	{
		//Transform-only shader (all flags off) (use for depth-only render)
		TRANSFORM	= 0,
		//The shader has some sort of color output
		SHADING		= (1<<0),
		//Per-pixel lighting
		PIXEL_LIGHTING  = (1<<1),
		//Shader applies shadows from shadow maps
		SHADOWS         = (1<<2),
		//Shader applies bump/normal maps
		BUMP            = (1<<3),
		//Shader applies height maps (in addition to bump map)
		HEIGHT          = (1<<3),
		//Shader applies alpha testing (w. fragment discard)
		ALPHATEST	= (1<<4),
		//Shader filters shadows
		SHADOW_FILTER	= (1<<5),
		//shader filters shadows (higher quality)
		SHADOW_FILTER_HQ = (1<<6),
		//uses ambient material
		MAT_AMBIENT	= (1<<7),
		//uses specular material
		MAT_SPECULAR	= (1<<8),
		//has diffuse texture
		MAT_DIFFUSETEX	= (1<<9),
	};

	typedef QMap<QString,FeatureFlags> t_FeatureFlagStrings;
	static t_FeatureFlagStrings featureFlagsStrings;

	QString getVShaderName(uint flags);
	QString getGShaderName(uint flags);
	QString getFShaderName(uint flags);
	QOpenGLShaderProgram* findOrLoadShader(uint flags);
	//! A simple shader preprocessor that can replace #defines
	QByteArray preprocessShader(const QString& fileName, uint flags);
	bool loadShader(QOpenGLShaderProgram& program, const QString& vShader, const QString& gShader, const QString& fShader, const uint flags);
	void buildUniformCache(QOpenGLShaderProgram& program);

	typedef QHash<uint,QOpenGLShaderProgram*> t_ShaderCache;
	t_ShaderCache m_shaderCache;

	typedef QHash<UNIFORM,GLuint> t_UniformCacheEntry;
	typedef QHash<const QOpenGLShaderProgram*, t_UniformCacheEntry> t_UniformCache;
	t_UniformCache m_uniformCache;
};

QOpenGLShaderProgram* ShaderMgr::getShader(const GlobalShaderParameters& globals,const OBJ::Material* mat)
{
	//Build bitflags from bools. Some stuff requires pixelLighting to be enabled, so check it too.
	uint flags = SHADING;
	if(globals.pixelLighting)            flags|= PIXEL_LIGHTING;
	if(globals.pixelLighting && globals.shadows) flags|= SHADOWS;
	if(globals.pixelLighting && globals.shadows && globals.shadowFilter) flags|= SHADOW_FILTER;
	if(globals.pixelLighting && globals.shadows && globals.shadowFilter && globals.shadowFilterHQ) flags|= SHADOW_FILTER_HQ;

	if(mat)
	{
		if(mat->illum>0)
			flags|= MAT_AMBIENT;
		if(mat->illum==4)
			flags|= ALPHATEST;
		if(mat->illum == OBJ::SPECULAR && globals.pixelLighting)
			flags|= MAT_SPECULAR;
		if(mat->texture)
			flags|= MAT_DIFFUSETEX;
		if(mat->bump_texture && globals.bump && globals.pixelLighting)
			flags|= BUMP;
		if(mat->height_texture && globals.bump && globals.pixelLighting)
			flags|= HEIGHT;
	}

	return findOrLoadShader(flags);
}

QOpenGLShaderProgram* ShaderMgr::getTransformShader()
{
	return findOrLoadShader(TRANSFORM);
}

GLint ShaderMgr::uniformLocation(const QOpenGLShaderProgram *shad, UNIFORM uni)
{
	t_UniformCache::iterator it = m_uniformCache.find(shad);
	if(it!=m_uniformCache.end())
	{
		return it.value().value(uni,-1);
	}
	return -1;
}

#endif
