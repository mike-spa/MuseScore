/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <vector>

#include "importtef.h"

namespace mu::iex::tabledit {
class MeasureHandler
{
public:
    int actualSize(const muse::vector<TefMeasure>& tefMeasures, const size_t idx) const;
    void calculate(const muse::vector<TefNote>& tefContents, const muse::vector<TefMeasure>& tefMeasures);
    int sumPreviousGaps(const size_t idx) const;
    size_t measureIndex(int tstart, const muse::vector<TefMeasure>& tefMeasures) const;
private:
    void dumpActualsAndSumGaps(const muse::vector<TefMeasure>& tefMeasures) const;
    void initializeMeasureStartsAndGaps(const muse::vector<TefMeasure>& tefMeasures);
    int offsetInMeasure(int tstart, const muse::vector<TefMeasure>& tefMeasures);
    void updateGapLeft(muse::vector<int>& gapLeft, const int position, const muse::vector<TefMeasure>& tefMeasures);
    void updateGapRight(muse::vector<int>& gapRight, const TefNote& note, const muse::vector<TefMeasure>& tefMeasures);
    void updateGaps(const muse::vector<TefNote>& tefContents, const muse::vector<TefMeasure>& tefMeasures);
    muse::vector<int> gapsLeft;
    muse::vector<int> gapsRight;
    muse::vector<int> nominalMeasureStarts;
};
} // namespace mu::iex::tabledit
