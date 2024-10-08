/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "dom/score.h"
#include "global/types/string.h"
#include "engravingerrors.h"

using namespace mu::engraving;
namespace mu::iex::musicxml {
Err importMusicXmlfromBuffer(Score* score, const muse::String&, const muse::ByteArray& data);
Err importCompressedMusicXml(MasterScore* score, const String& name, bool forceMode);
Err importMusicXml(MasterScore* score, const String& name, bool forceMode);
}
