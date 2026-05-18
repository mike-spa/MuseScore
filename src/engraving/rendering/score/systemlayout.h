/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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

#include "layoutcontext.h"

#include "../../dom/measure.h"
#include "../../dom/segment.h"
#include "../../types/types.h"

namespace mu::engraving {
class BarLine;
class Bracket;
class BracketItem;
class Chord;
class Dynamic;
class Expression;
class FiguredBass;
class FretDiagram;
class Harmony;
class HarpPedalDiagram;
class Image;
class InstrumentChange;
class Measure;
class MeasureNumber;
class MMRest;
class MMRestRange;
class Parenthesis;
class RehearsalMark;
class Score;
class Segment;
class SkylineLine;
class Spanner;
class StaffText;
class Sticking;
class System;
class SystemText;
class TempoText;
class TimeSig;
}

namespace mu::engraving::rendering::score {
class SystemLayout
{
public:
    static System* collectSystem(LayoutContext& ctx);
    static void layoutSystemElements(System* system, LayoutContext& ctx);

    static void layoutSystem(System* system, LayoutContext& ctx, double xo1, bool isFirstSystem = false, bool firstSystemIndent = false);

    static void hideEmptyStaves(System* system, LayoutContext& ctx, bool isFirstSystem);
    static bool canChangeSysStaffVisibility(const System* system, const staff_idx_t staffIdx);

    static void layout2(System* system, LayoutContext& ctx);
    static void restoreLayout2(System* system, LayoutContext& ctx);
    static void setMeasureHeight(System* system, double height, const LayoutContext& ctx);

    static double minDistance(const System* top, const System* bottom, const LayoutContext& ctx);

    static void centerElementsBetweenStaves(const System* system);
    static void centerBigTimeSigsAcrossStaves(const System* system);

    static void updateSkylineForElement(EngravingItem* element, const System* system, double yMove);
    static void removeElementFromSkyline(EngravingItem* element, const System* system);

    static void layoutSystemLockIndicators(System* system, LayoutContext& ctx);

private:
    struct MeasureState
    {
        Measure* measure = nullptr;
        double measureWidth = 0.0;
        double measurePos = 0.0;
        std::map<EngravingItem*, PointF> elementPositions;
        bool curHeader = false;
        bool curTrailer = false;

        void clear()
        {
            measure = nullptr;
            measureWidth = 0.0;
            measurePos = 0.0;
            elementPositions.clear();
        }

        void restoreMeasure()
        {
            measure->mutldata()->setPosX(measurePos);
            measure->setWidth(measureWidth);
            for (auto pair : elementPositions) {
                pair.first->setPos(pair.second);
            }
        }
    };

    struct ElementsToLayout
    {
        System* system;
        muse::vector<Measure*> measures;
        muse::vector<Segment*> segments;

        muse::vector<ChordRest*> chordRests;
        muse::vector<Chord*> chords;
        muse::vector<BarLine*> barlines;
        muse::vector<TimeSig*> timeSigAboveStaves;

        muse::vector<MeasureNumber*> measureNumbers;
        muse::vector<MMRestRange*> mmrRanges;
        muse::vector<EngravingItem*> markersAndJumps;

        muse::vector<Sticking*> stickings;
        muse::vector<EngravingItem*> fermatasAndTremoloBars;
        muse::vector<FiguredBass*> figuredBass;
        muse::vector<Dynamic*> dynamics;
        muse::vector<Expression*> expressions;
        muse::vector<HarpPedalDiagram*> harpDiagrams;
        muse::vector<FretDiagram*> fretDiagrams;
        muse::vector<StaffText*> staffText;
        muse::vector<InstrumentChange*> instrChanges;
        muse::vector<SystemText*> systemText;
        muse::vector<EngravingItem*> playTechCapoStringTunTripletFeel;
        muse::vector<RehearsalMark*> rehMarks;
        muse::vector<TempoText*> tempoText;
        muse::vector<Image*> images;
        muse::vector<Parenthesis*> parenthesis;
        muse::vector<Harmony*> harmonies;
        muse::vector<PlayCountText*> playCountText;

        muse::vector<Spanner*> slurs;
        muse::vector<Spanner*> trills;
        muse::vector<Spanner*> hairpins;
        muse::vector<Spanner*> ottavas;
        muse::vector<Spanner*> pedal;
        muse::vector<Spanner*> voltas;
        muse::vector<Spanner*> tempoChangeLines;
        muse::vector<Spanner*> partialLyricsLines;
        muse::vector<Spanner*> allOtherSpanners;

        muse::vector<GuitarBend*> guitarBends;

        ElementsToLayout(System* s)
            : system(s) {}
    };

    static void collectElementsToLayout(Measure* measure, ElementsToLayout& elements, const LayoutContext& ctx);
    static void collectSpannersToLayout(ElementsToLayout& elements, const LayoutContext& ctx);

    static System* getNextSystem(LayoutContext& lc);
    static void createSkylines(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);
    static void processLines(System* system, LayoutContext& ctx, const muse::vector<Spanner*>& lines, bool align = false);
    static void layoutTies(Chord* ch, System* system, const Fraction& stick, LayoutContext& ctx);
    static void doLayoutTies(System* system, const muse::vector<Segment*>& sl, const Fraction& stick, const Fraction& etick,
                             LayoutContext& ctx);
    static void doLayoutNoteSpannersLinear(System* system, LayoutContext& ctx);
    static void layoutNoteAnchoredSpanners(System* system, Chord* chord);
    static void updateCrossBeams(System* system, LayoutContext& ctx);
    static bool measureHasCrossStuffOrModifiedBeams(const Measure* measure);
    static void restoreOldSystemLayout(System* system, LayoutContext& ctx);
    static void layoutTuplets(const muse::vector<ChordRest*>& chordRests, LayoutContext& ctx);

    static void layoutTiesAndBends(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static double minVertSpaceForCrossStaffBeams(System* system, staff_idx_t staffIdx1, staff_idx_t staffIdx2, LayoutContext& ctx);

    static bool elementShouldBeCenteredBetweenStaves(const EngravingItem* item, const System* system);
    static bool mmRestShouldBeCenteredBetweenStaves(const MMRest* mmRest, const System* system);
    static bool whammyBarShouldBeCenteredBetweenStaves(const WhammyBarSegment* wbar, const System* system);
    static bool elementHasAnotherStackedOutside(const EngravingItem* element, const Shape& elementShape, const SkylineLine& skylineLine);
    static void centerElementBetweenStaves(EngravingItem* element, const System* system);
    static void centerMMRestBetweenStaves(MMRest* mmRest, const System* system);

    static bool shouldBeJustified(System* system, double curSysWidth, double targetSystemWidth, LayoutContext& ctx);

    static void updateTimeSigAboveStavesXPos(System* system, LayoutContext& ctx);
    static void clearBigTimeSigNotShown(System* system, LayoutContext& ctx);

    static void layoutSticking(const muse::vector<Sticking*> stickings, System* system, LayoutContext& ctx);

    static void layoutLyrics(const ElementsToLayout& elements, LayoutContext& ctx);

    static void layoutVoltas(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static void layoutDynamicExpressionAndHairpins(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);

    static void layoutParenthesisAndBigTimeSigs(const ElementsToLayout& elementsToLayout);

    static void layoutHarmonies(const muse::vector<Harmony*> harmonies, System* system, LayoutContext& ctx);
    static void layoutFretDiagrams(const ElementsToLayout& elements, System* system, LayoutContext& ctx);

    static void alignRests(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);
    static void checkFullMeasureRestCollisions(const ElementsToLayout& elementsToLayout, LayoutContext& ctx);
};
}
