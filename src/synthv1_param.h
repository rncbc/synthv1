// synthv1_param.h
//
/****************************************************************************
   Copyright (C) 2012-2019, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#ifndef __synthv1_param_h
#define __synthv1_param_h

#include "synthv1.h"

#include <QString>

// forward decl.
class QDomElement;
class QDomDocument;


//-------------------------------------------------------------------------
// synthv1_param - decl.
//

namespace synthv1_param
{
	// Preset serialization methods.
	bool loadPreset(synthv1 *pSynth,
		const QString& sFilename);
	bool savePreset(synthv1 *pSynth,
		const QString& sFilename,
		bool bSymLink = false);

	// Tuning serialization methods.
	void loadTuning(synthv1 *pSynth,
		const QDomElement& eTuning);
	void saveTuning(synthv1 *pSynth,
		QDomDocument& doc, QDomElement& eTuning,
		bool bSymLink = false);

	// Default parameter name/value helpers.
	const char *paramName(synthv1::ParamIndex index);
	float paramDefaultValue(synthv1::ParamIndex index);
	float paramSafeValue(synthv1::ParamIndex index, float fValue);
	float paramValue(synthv1::ParamIndex index, float fScale);
	float paramScale(synthv1::ParamIndex index, float fValue);
	bool paramFloat(synthv1::ParamIndex index);

	// Load/save and convert canonical/absolute filename helpers.
	QString loadFilename(const QString& sFilename);
	QString saveFilename(const QString& sFilename, bool bSymLink);
};


#endif	// __synthv1_param_h

// end of synthv1_param.h
