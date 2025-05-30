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

#include "tapping.h"
#include "text.h"

namespace mu::engraving {
Tapping::Tapping(ChordRest* parent)
    : Articulation(parent, ElementType::TAPPING)
{
}

void Tapping::scanElements(void* data, void (*func)(void* data, EngravingItem*), bool all)
{
    if (m_text) {
        m_text->scanElements(data, func, all);
    }

    func(data, this);
}
}
