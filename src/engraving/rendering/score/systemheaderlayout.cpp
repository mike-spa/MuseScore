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

#include "systemheaderlayout.h"

#include "systemlayout.h"
#include "tlayout.h"

#include "dom/bracket.h"
#include "dom/factory.h"
#include "dom/part.h"
#include "dom/system.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

double SystemHeaderLayout::layoutBrackets(System* system, LayoutContext& ctx)
{
    size_t nstaves = system->staves().size();
    size_t columns = system->getBracketsColumnsCount();

    std::vector<double> bracketWidth(columns, 0.0);

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (size_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                Bracket* b = createBracket(system, ctx, bi, i, static_cast<int>(staffIdx), bl, system->firstMeasure());
                if (b != nullptr) {
                    b->mutldata()->bracketHeight.set_value(3.5 * b->spatium() * 2); // dummy
                    TLayout::layoutBracket(b, b->mutldata(), ctx.conf());
                    bracketWidth[i] = std::max(bracketWidth[i], b->ldata()->bracketWidth());
                }
            }
        }
    }

    for (Bracket* b : bl) {
        delete b;
    }

    double totalBracketWidth = 0.0;

    if (!system->brackets().empty()) {
        for (double w : bracketWidth) {
            totalBracketWidth += w;
        }
    }

    return totalBracketWidth;
}

Bracket* SystemHeaderLayout::createBracket(System* system, LayoutContext& ctx, BracketItem* bi, size_t column, staff_idx_t staffIdx,
                                           std::vector<Bracket*>& bl, Measure* measure)
{
    if (!measure) {
        return nullptr;
    }

    size_t nstaves = system->staves().size();
    staff_idx_t firstStaff = staffIdx;
    staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
    if (lastStaff >= nstaves) {
        lastStaff = nstaves - 1;
    }

    for (; firstStaff <= lastStaff; ++firstStaff) {
        if (system->staff(firstStaff)->show()) {
            break;
        }
    }
    for (; lastStaff >= firstStaff; --lastStaff) {
        if (system->staff(lastStaff)->show()) {
            break;
        }
    }
    size_t span = lastStaff - firstStaff + 1;
    //
    // do not show bracket if it only spans one
    // system due to some invisible staves
    //
    if (span > 1
        || (bi->bracketSpan() == span)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() != BracketType::SQUARE)
        || (span == 1 && ctx.conf().styleB(Sid::alwaysShowSquareBracketsWhenEmptyStavesAreHidden)
            && bi->bracketType() == BracketType::SQUARE)) {
        //
        // this bracket is visible
        //
        Bracket* b = 0;
        track_idx_t track = staffIdx * VOICES;
        for (size_t k = 0; k < bl.size(); ++k) {
            if (bl[k]->track() == track && bl[k]->column() == column && bl[k]->bracketType() == bi->bracketType()
                && bl[k]->measure() == measure) {
                b = muse::takeAt(bl, k);
                break;
            }
        }
        if (b == 0) {
            b = Factory::createBracket(ctx.mutDom().dummyParent());
            b->setBracketItem(bi);
            b->setGenerated(true);
            b->setTrack(track);
            b->setMeasure(measure);
        }
        system->add(b);

        if (bi->selected()) {
            bool needSelect = true;

            std::vector<EngravingItem*> brackets = ctx.selection().elements(ElementType::BRACKET);
            for (const EngravingItem* element : brackets) {
                if (toBracket(element)->bracketItem() == bi) {
                    needSelect = false;
                    break;
                }
            }

            if (needSelect) {
                ctx.select(b, SelectType::ADD);
            }
        }

        b->setStaffSpan(firstStaff, lastStaff);

        return b;
    }

    return nullptr;
}

void SystemHeaderLayout::addBrackets(System* system, Measure* measure, LayoutContext& ctx)
{
    if (system->staves().empty()) {                 // ignore vbox
        return;
    }

    size_t nstaves = system->staves().size();

    //---------------------------------------------------
    //  find x position of staves
    //    create brackets
    //---------------------------------------------------

    size_t columns = system->getBracketsColumnsCount();

    std::vector<Bracket*> bl;
    bl.swap(system->brackets());

    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* s = ctx.dom().staff(staffIdx);
        for (size_t i = 0; i < columns; ++i) {
            for (auto bi : s->brackets()) {
                if (bi->column() != i || bi->bracketType() == BracketType::NO_BRACKET) {
                    continue;
                }
                createBracket(system, ctx, bi, i, staffIdx, bl, measure);
            }
        }
        if (!system->staff(staffIdx)->show()) {
            continue;
        }
    }

    //---------------------------------------------------
    //  layout brackets
    //---------------------------------------------------
    SystemLayout::layoutBracketsVertical(system, ctx);

    system->setBracketsXPosition(measure->x());

    muse::join(system->brackets(), bl);
}

double SystemHeaderLayout::totalBracketOffset(LayoutContext& ctx)
{
    if (ctx.state().totalBracketsWidth() >= 0) {
        return ctx.state().totalBracketsWidth();
    }

    size_t columns = 0;
    for (const Staff* staff : ctx.dom().staves()) {
        for (const BracketItem* bi : staff->brackets()) {
            columns = std::max(columns, bi->column() + 1);
        }
    }

    size_t nstaves = ctx.dom().nstaves();
    std::vector < double > bracketWidth(nstaves, 0.0);
    for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
        const Staff* staff = ctx.dom().staff(staffIdx);
        for (auto bi : staff->brackets()) {
            if (bi->bracketType() == BracketType::NO_BRACKET || !bi->visible()) {
                continue;
            }

            //! This logic is partially copied from System::createBracket.
            //! Of course, we don't need to worry about invisible staves,
            //! but we do need to worry about brackets that span past the
            //! last staff.
            staff_idx_t firstStaff = staffIdx;
            staff_idx_t lastStaff = staffIdx + bi->bracketSpan() - 1;
            if (lastStaff >= nstaves) {
                lastStaff = nstaves - 1;
            }

            for (; firstStaff <= lastStaff; ++firstStaff) {
                if (ctx.dom().staff(firstStaff)->show()) {
                    break;
                }
            }
            for (; lastStaff >= firstStaff; --lastStaff) {
                if (ctx.dom().staff(lastStaff)->show()) {
                    break;
                }
            }

            size_t span = lastStaff - firstStaff + 1;
            if (span > 1
                || (bi->bracketSpan() == span)
                || (span == 1 && ctx.conf().styleB(Sid::alwaysShowBracketsWhenEmptyStavesAreHidden))) {
                Bracket* dummyBr = Factory::createBracket(ctx.mutDom().dummyParent(), /*isAccessibleEnabled=*/ false);
                dummyBr->setBracketItem(bi);
                dummyBr->setStaffSpan(firstStaff, lastStaff);
                dummyBr->mutldata()->bracketHeight.set_value(3.5 * dummyBr->spatium() * 2); // default
                TLayout::layoutBracket(dummyBr, dummyBr->mutldata(), ctx.conf());
                for (staff_idx_t stfIdx = firstStaff; stfIdx <= lastStaff; ++stfIdx) {
                    bracketWidth[stfIdx] += dummyBr->ldata()->bracketWidth();
                }
                delete dummyBr;
            }
        }
    }

    double totalBracketsWidth = 0.0;
    for (double w : bracketWidth) {
        totalBracketsWidth = std::max(totalBracketsWidth, w);
    }
    ctx.mutState().setTotalBracketsWidth(totalBracketsWidth);

    return totalBracketsWidth;
}

double SystemHeaderLayout::instrumentNamesWidth(System* system, LayoutContext& ctx, bool isFirstSystem, double instrumentNameOffset)
{
    double namesWidth = 0.0;

    std::set<Part*> partsWithIndividualStaffNames;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (!ctx.dom().staff(staffIdx)->show()) {
            // We know that the staff is hidden in the entire score so safe to skip
            continue;
        }

        const SysStaff* staff = system->staff(staffIdx);
        if (!staff || (isFirstSystem && !staff->show())) {
            continue;
        }

        if (InstrumentName* name = staff->individualStaffName) {
            TLayout::layoutInstrumentName(name, name->mutldata());
            namesWidth = std::max(namesWidth, name->width());

            partsWithIndividualStaffNames.insert(ctx.dom().staff(staffIdx)->part());
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        if (!ctx.dom().staff(staffIdx)->part()->show()) {
            // We know that the part is hidden in the entire score so safe to skip
            continue;
        }

        const SysStaff* staff = system->staff(staffIdx);

        if (InstrumentName* name = staff->instrumentName) {
            TLayout::layoutInstrumentName(name, name->mutldata());
            namesWidth = std::max(namesWidth, name->width());

            Part* part = ctx.dom().staff(staffIdx)->part();
            if (muse::contains(partsWithIndividualStaffNames, part) && name->position() == AlignH::RIGHT) {
                if (isFirstSystem) {
                    std::vector<staff_idx_t> visibleStaves = system->visibleStavesOfPart(part);
                    size_t visibleStavesCount = visibleStaves.size();
                    if (visibleStavesCount % 2) {
                        staff_idx_t midStaff = visibleStaves[visibleStavesCount / 2];
                        if (InstrumentName* staffName = system->staff(midStaff)->individualStaffName) {
                            namesWidth = std::max(namesWidth, staffName->width() + name->width() + instrumentNameOffset);
                        }
                    }
                } else {
                    staff_idx_t startStaff = system->firstSysStaffOfPart(part);
                    staff_idx_t endStaff = startStaff + part->nstaves();
                    for (staff_idx_t idx = startStaff; idx < endStaff; ++idx) {
                        if (InstrumentName* staffName = system->staff(idx)->individualStaffName) {
                            namesWidth = std::max(namesWidth, staffName->width() + name->width() + instrumentNameOffset);
                        }
                    }
                }
            }
        }
    }

    return namesWidth;
}

void SystemHeaderLayout::setInstrumentNamesVerticalPos(System* system, LayoutContext& ctx)
{
    std::set<const Part*> partsWithIndividualStaffNames;

    for (staff_idx_t staffIdx = 0; staffIdx < ctx.dom().nstaves(); ++staffIdx) {
        SysStaff* s = system->staff(staffIdx);
        if (InstrumentName* individualName = s->individualStaffName) {
            const RectF& staffBBox = s->bbox();
            const RectF& nameBBox = individualName->ldata()->bbox();
            individualName->mutldata()->setPosY(0.5 * (staffBBox.top() + staffBBox.bottom() - nameBBox.bottom() - nameBBox.top()));
            partsWithIndividualStaffNames.insert(ctx.dom().staff(staffIdx)->part());
        }
    }

    staff_idx_t staffIdx = 0;
    for (const Part* p : ctx.dom().parts()) {
        size_t nstaves = p->nstaves();

        SysStaff* s = system->staff(staffIdx);
        InstrumentName* t = s->instrumentName;
        if (!t || t->effectiveStaffIdx() == muse::nidx) {
            staffIdx += nstaves;
            continue;
        }

        const RectF& bbox = t->ldata()->bbox();
        double yCenter = 0.5 * (bbox.bottom() + bbox.top());

        std::vector<staff_idx_t> visibleStavesOfPart = system->visibleStavesOfPart(p);
        size_t visibleStavesCount = visibleStavesOfPart.size();
        DO_ASSERT(visibleStavesCount > 0);

        double y1 = 0;
        double y2 = 0;

        if (!muse::contains(partsWithIndividualStaffNames, p)) {
            SysStaff* topSt = system->staff(visibleStavesOfPart.front());
            SysStaff* bottomSt = system->staff(visibleStavesOfPart.back());
            y1 = topSt->bbox().top();
            y2 = bottomSt->bbox().bottom();
            t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
        } else {
            if (visibleStavesCount % 2) {
                SysStaff* midStaff = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
                y1 = midStaff->bbox().top();
                y2 = midStaff->bbox().bottom();
                if (InstrumentName* staffName = midStaff->individualStaffName) {
                    if (staffName->position() != AlignH::RIGHT) {
                        double lineSpacing = t->fontMetrics().lineSpacing();
                        double instrNameBottom = t->ldata()->blocks.back().y();
                        double centerY = 0.5 * (midStaff->bbox().top() + midStaff->bbox().bottom());
                        t->mutldata()->setPosY(centerY - instrNameBottom - 0.25 * lineSpacing);
                        double staffNameTop = staffName->ldata()->blocks.front().y();
                        staffName->mutldata()->setPosY(centerY - staffNameTop + 0.75 * lineSpacing);
                    } else if (staffName->ldata()->rows() == 1 && t->ldata()->rows() == 1) {
                        t->mutldata()->setPosY(staffName->y());
                    } else {
                        t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
                    }
                } else {
                    t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
                }
            } else {
                SysStaff* staffAboveMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2 - 1]);
                SysStaff* staffBelowMid = system->staff(visibleStavesOfPart[visibleStavesCount / 2]);
                y1 = staffAboveMid->bbox().top();
                y2 = staffBelowMid->bbox().bottom();
                t->mutldata()->setPosY(0.5 * (y1 + y2) - yCenter);
            }
        }

        staffIdx += nstaves;
    }
}

void SystemHeaderLayout::setInstrumentNamesHorizontalPos(System* system, double maxNamesWidth, double instrumentNameOffset)
{
    //---------------------------------------------------
    //  layout instrument names x position
    //     at this point it is not clear which staves will
    //     be hidden, so layout all instrument names
    //---------------------------------------------------

    for (const SysStaff* s : system->staves()) {
        if (InstrumentName* t = s->individualStaffName) {
            const RectF& bbox = t->ldata()->bbox();
            switch (t->position()) {
            case AlignH::JUSTIFY: // Justify is not supported for instrument names
            case AlignH::LEFT:
                t->mutldata()->setPosX(0 - bbox.left());
                break;
            case AlignH::HCENTER:
                t->mutldata()->setPosX(maxNamesWidth * .5 - (bbox.right() + bbox.left()) * .5);
                break;
            case AlignH::RIGHT:
                t->mutldata()->setPosX(maxNamesWidth - bbox.right());
                break;
            }
        }
    }

    for (staff_idx_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        const SysStaff* s = system->staff(staffIdx);
        const Part* p = system->score()->staff(staffIdx)->part();
        if (InstrumentName* t = s->instrumentName; t && t->effectiveStaffIdx() != muse::nidx) {
            const RectF& bbox = t->ldata()->bbox();
            std::vector<staff_idx_t> visibleStavesForPart = system->visibleStavesOfPart(p);
            size_t visibleStaveCount = visibleStavesForPart.size();
            switch (t->position()) {
            case AlignH::JUSTIFY:   // Justify is not supported for instrument names
            case AlignH::LEFT:
                t->mutldata()->setPosX(0 - bbox.left());
                break;
            case AlignH::HCENTER:
                t->mutldata()->setPosX(maxNamesWidth * .5 - (bbox.right() + bbox.left()) * .5);
                break;
            case AlignH::RIGHT:
                if (visibleStaveCount % 2) {
                    staff_idx_t centerStaff = visibleStavesForPart[visibleStaveCount / 2];
                    if (InstrumentName* staffName = system->staff(centerStaff)->individualStaffName) {
                        t->mutldata()->setPosX(staffName->x() - staffName->width() - instrumentNameOffset);
                    } else {
                        t->mutldata()->setPosX(maxNamesWidth - bbox.right());
                    }
                } else {
                    t->mutldata()->setPosX(maxNamesWidth - bbox.right());
                }
                break;
            }
        }
    }
}

void SystemHeaderLayout::setInstrumentNames(System* system, LayoutContext& ctx, bool longName, Fraction tick)
{
    //
    // remark: add/remove instrument names is not undo/redoable
    //         as add/remove of systems is not undoable
    //
    if (system->vbox()) {                 // ignore vbox
        return;
    }
    if (!ctx.conf().isShowInstrumentNames()
        || (ctx.conf().styleB(Sid::hideInstrumentNameIfOneInstrument) && ctx.dom().visiblePartCount() <= 1)
        || (ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE)
        || (!ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>()
            == InstrumentLabelVisibility::HIDE)) {
        for (SysStaff* staff : system->staves()) {
            if (staff->instrumentName) {
                ctx.mutDom().removeElement(staff->instrumentName);
            }
        }
        return;
    }

    for (size_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        SysStaff* staff = system->staves()[staffIdx];
        const Staff* s = ctx.dom().staff(staffIdx);
        Part* part = s->part();

        bool atLeastOneVisibleStaff = false;
        for (Staff* partStaff : part->staves()) {
            if (partStaff->show()) {
                atLeastOneVisibleStaff = true;
                break;
            }
        }

        bool showName = part->show() && atLeastOneVisibleStaff;
        if (!s->isTop() || !showName) {
            if (staff->instrumentName) {
                ctx.mutDom().removeElement(staff->instrumentName);
            }
            continue;
        }

        const String& name = longName ? part->longName(tick) : part->shortName(tick);
        if (name.empty()) {
            continue;
        }

        InstrumentName* iname = staff->instrumentName;
        if (!iname) {
            iname = new InstrumentName(system);
            iname->setGenerated(true);
            iname->setParent(system);
            iname->setSysStaff(staff);
            iname->setTrack(staffIdx * VOICES);
            iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
            iname->setInstrumentNameRole(InstrumentNameRole::PART);
            ctx.mutDom().addElement(iname);
        }
        iname->setAlign(Align(iname->align().horizontal, AlignV::BASELINE));
        iname->setXmlText(name);
    }
}

void SystemHeaderLayout::setIndividualStaffNames(System* system, LayoutContext& ctx, bool longName, Fraction tick)
{
    if (system->vbox()) {
        return;
    }

    if (!ctx.conf().isShowInstrumentNames()
        || (ctx.conf().styleB(Sid::hideInstrumentNameIfOneInstrument) && ctx.dom().visibleStaffCount() <= 1)
        || (ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::firstSystemInstNameVisibility).value<InstrumentLabelVisibility>() == InstrumentLabelVisibility::HIDE)
        || (!ctx.state().firstSystem()
            && ctx.conf().styleV(Sid::subsSystemInstNameVisibility).value<InstrumentLabelVisibility>()
            == InstrumentLabelVisibility::HIDE)) {
        for (SysStaff* staff : system->staves()) {
            if (staff->individualStaffName) {
                ctx.mutDom().removeElement(staff->instrumentName);
            }
        }
        return;
    }

    for (size_t staffIdx = 0; staffIdx < system->staves().size(); ++staffIdx) {
        SysStaff* sysStaff = system->staves()[staffIdx];
        const Staff* staff = ctx.dom().staff(staffIdx);
        Part* part = staff->part();
        if (!staff->show() || !part->show() || part->nstaves() < 2) {
            if (sysStaff->individualStaffName) {
                ctx.mutDom().removeElement(sysStaff->individualStaffName);
            }
            continue;
        }

        const String& name = longName ? staff->individualStaffNameLong(tick) : staff->individualStaffNameShort(tick);
        if (name.empty()) {
            if (sysStaff->individualStaffName) {
                ctx.mutDom().removeElement(sysStaff->individualStaffName);
            }
            continue;
        }

        InstrumentName* iname = sysStaff->individualStaffName;
        if (!iname) {
            iname = new InstrumentName(system);
            iname->setGenerated(true);
            iname->setParent(system);
            iname->setSysStaff(sysStaff);
            iname->setTrack(staffIdx * VOICES);
            iname->setInstrumentNameType(longName ? InstrumentNameType::LONG : InstrumentNameType::SHORT);
            iname->setInstrumentNameRole(InstrumentNameRole::STAFF);
            ctx.mutDom().addElement(iname);
        }
        iname->setAlign(Align(iname->align().horizontal, AlignV::BASELINE));
        iname->setXmlText(name);
    }
}
