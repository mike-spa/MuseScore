/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "layoutcontext.h"

namespace mu::engraving {
class Bracket;
class BracketItem;
class System;
}

namespace mu::engraving::rendering::score {
class SystemHeaderLayout
{
public:
    static double layoutBrackets(System* system, LayoutContext& ctx);
    static void addBrackets(System* system, Measure* measure, LayoutContext& ctx);
    static double totalBracketOffset(LayoutContext& ctx);

    static double instrumentNamesWidth(System* system, LayoutContext& ctx, bool isFirstSystem, double instrumentNameOffset);
    static void setInstrumentNamesVerticalPos(System* system, LayoutContext& ctx);
    static void setInstrumentNamesHorizontalPos(System* system, double maxNamesWidth, double instrumentNameOffset);
    static void setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick = { 0, 1 });
    static void setIndividualStaffNames(System* system, LayoutContext& ctx, bool longName, Fraction tick = { 0, 1 });

private:
    static Bracket* createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                  std::vector<Bracket*>& bl, Measure* measure);
};
}
