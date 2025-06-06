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

#include "../../idlib/precompiled.h"
#pragma hdrstop

#include "AAS_local.h"
#ifdef MOD_BOTS // cusTom3 - aas extensions - define BotAASBuild for use
#include "../bots/BotAASBuild.h"
#endif

/*
============
idAAS::Alloc
============
*/
idAAS *idAAS::Alloc(void)
{
	return new idAASLocal;
}

/*
============
idAAS::idAAS
============
*/
idAAS::~idAAS(void)
{
}

/*
============
idAASLocal::idAASLocal
============
*/
idAASLocal::idAASLocal(void)
{
	file = NULL;
#ifdef MOD_BOTS	// cusTom3 - aas extensions - what a mess, this should probably be in Init - TODO: look at it later
	botAASBuilder = NULL;
#endif
}

/*
============
idAASLocal::~idAASLocal
============
*/
idAASLocal::~idAASLocal(void)
{
	Shutdown();
#ifdef MOD_BOTS	// cusTom3 - aas extensions - what a mess, this should probably be in shut down, TODO: look at it later
    delete botAASBuilder;
#endif
}

/*
============
idAASLocal::Init
============
*/
bool idAASLocal::Init(const idStr &mapName, unsigned int mapFileCRC)
{
	if (file && mapName.Icmp(file->GetName()) == 0 && mapFileCRC == file->GetCRC()) {
		common->Printf("Keeping %s\n", file->GetName());
		RemoveAllObstacles();
	} else {
		Shutdown();

		file = AASFileManager->LoadAAS(mapName, mapFileCRC);

		if (!file) {
			common->DWarning("Couldn't load AAS file: '%s'", mapName.c_str());
			return false;
		}

#ifdef MOD_BOTS // cusTom3 - aas extensions
		if(BOT_ENABLED()) {
			// TODO: don't need a builder unless it is a 48, but Init's for now, look at later
			// if class changing is added models could change, would have to handle that here
            idStr ext;
            mapName.ExtractFileExtension(ext);
			if (ext.Find( BOT_AAS, false ) >= 0) {
                if(!botAASBuilder)
                    botAASBuilder = new BotAASBuild();
                botAASBuilder->Init( this );
				botAASBuilder->AddReachabilities();
			}
        }
#endif // TODO: save the new information out to a file so it doesn't have to be processed each map load
		SetupRouting();
	}

	return true;
}

/*
============
idAASLocal::Shutdown
============
*/
void idAASLocal::Shutdown(void)
{
	if (file) {
#ifdef MOD_BOTS // cusTom3 - aas extensions
		if(botAASBuilder) {
            botAASBuilder->FreeAAS();
		}
#endif // TODO: save the new information out to a file so it doesn't have to be processed each map load
		ShutdownRouting();
		RemoveAllObstacles();
		AASFileManager->FreeAAS(file);
		file = NULL;
	}
}

/*
============
idAASLocal::Stats
============
*/
void idAASLocal::Stats(void) const
{
	if (!file) {
		return;
	}

	common->Printf("[%s]\n", file->GetName());
	file->PrintInfo();
	RoutingStats();
}

/*
============
idAASLocal::GetSettings
============
*/
const idAASSettings *idAASLocal::GetSettings(void) const
{
	if (!file) {
		return NULL;
	}

	return &file->GetSettings();
}

/*
============
idAASLocal::PointAreaNum
============
*/
int idAASLocal::PointAreaNum(const idVec3 &origin) const
{
	if (!file) {
		return 0;
	}

	return file->PointAreaNum(origin);
}

/*
============
idAASLocal::PointReachableAreaNum
============
*/
int idAASLocal::PointReachableAreaNum(const idVec3 &origin, const idBounds &searchBounds, const int areaFlags) const
{
	if (!file) {
		return 0;
	}

	return file->PointReachableAreaNum(origin, searchBounds, areaFlags, TFL_INVALID);
}

/*
============
idAASLocal::BoundsReachableAreaNum
============
*/
int idAASLocal::BoundsReachableAreaNum(const idBounds &bounds, const int areaFlags) const
{
	if (!file) {
		return 0;
	}

	return file->BoundsReachableAreaNum(bounds, areaFlags, TFL_INVALID);
}

/*
============
idAASLocal::PushPointIntoAreaNum
============
*/
void idAASLocal::PushPointIntoAreaNum(int areaNum, idVec3 &origin) const
{
	if (!file) {
		return;
	}

	file->PushPointIntoAreaNum(areaNum, origin);
}

/*
============
idAASLocal::AreaCenter
============
*/
idVec3 idAASLocal::AreaCenter(int areaNum) const
{
	if (!file) {
		return vec3_origin;
	}

	return file->GetArea(areaNum).center;
}

/*
============
idAASLocal::AreaFlags
============
*/
int idAASLocal::AreaFlags(int areaNum) const
{
	if (!file) {
		return 0;
	}

	return file->GetArea(areaNum).flags;
}

/*
============
idAASLocal::AreaTravelFlags
============
*/
int idAASLocal::AreaTravelFlags(int areaNum) const
{
	if (!file) {
		return 0;
	}

	return file->GetArea(areaNum).travelFlags;
}

/*
============
idAASLocal::Trace
============
*/
bool idAASLocal::Trace(aasTrace_t &trace, const idVec3 &start, const idVec3 &end) const
{
	if (!file) {
		trace.fraction = 0.0f;
		trace.lastAreaNum = 0;
		trace.numAreas = 0;
		return true;
	}

	return file->Trace(trace, start, end);
}

/*
============
idAASLocal::GetPlane
============
*/
const idPlane &idAASLocal::GetPlane(int planeNum) const
{
	if (!file) {
		static idPlane dummy;
		return dummy;
	}

	return file->GetPlane(planeNum);
}

/*
============
idAASLocal::GetEdgeVertexNumbers
============
*/
void idAASLocal::GetEdgeVertexNumbers(int edgeNum, int verts[2]) const
{
	if (!file) {
		verts[0] = verts[1] = 0;
		return;
	}

	const int *v = file->GetEdge(abs(edgeNum)).vertexNum;

	verts[0] = v[INTSIGNBITSET(edgeNum)];

	verts[1] = v[INTSIGNBITNOTSET(edgeNum)];
}

/*
============
idAASLocal::GetEdge
============
*/
void idAASLocal::GetEdge(int edgeNum, idVec3 &start, idVec3 &end) const
{
	if (!file) {
		start.Zero();
		end.Zero();
		return;
	}

	const int *v = file->GetEdge(abs(edgeNum)).vertexNum;

	start = file->GetVertex(v[INTSIGNBITSET(edgeNum)]);

	end = file->GetVertex(v[INTSIGNBITNOTSET(edgeNum)]);
}
