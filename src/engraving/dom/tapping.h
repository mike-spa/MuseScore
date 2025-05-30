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

#include "engraving/types/types.h"
#include "articulation.h"

namespace mu::engraving {
class Tapping : public Articulation
{
    OBJECT_ALLOCATOR(engraving, Tapping)
    DECLARE_CLASSOF(ElementType::TAPPING)

public:
    struct LayoutData : public Articulation::LayoutData
    {
    };
    DECLARE_LAYOUTDATA_METHODS(Tapping)

    TappingHand hand() const { return m_hand; }
    void setHand(TappingHand h) { m_hand = h; }

    Text* text() const { return m_text; }
    void setText(Text* text) { m_text = text; }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

protected:
    friend class mu::engraving::Factory;
    Tapping(ChordRest* parent);

private:
    TappingHand m_hand = TappingHand::INVALID;
    Text* m_text = nullptr;
};
} // namespace mu::engraving
