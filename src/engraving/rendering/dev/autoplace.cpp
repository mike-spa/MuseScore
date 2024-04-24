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
#include "autoplace.h"

#include "style/style.h"

#include "dom/chordrest.h"
#include "dom/segment.h"
#include "dom/spanner.h"
#include "dom/staff.h"
#include "dom/system.h"
#include "dom/measure.h"

using namespace muse;
using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void Autoplace::autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    if (item->autoplace() && item->explicitParent()) {
        const Segment* s = toSegment(item->explicitParent());
        const Measure* m = s->measure();

        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(m->ldata()->isSetPos());
        LD_CONDITION(s->ldata()->isSetPos());

        double sp = item->style().spatium();
        staff_idx_t si = item->staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == muse::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double mag = item->staff()->staffMag(item);
        sp *= mag;
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        RectF r = item->ldata()->bbox().translated(m->pos() + s->pos() + item->pos());

        // Adjust bbox Y pos for staffType offset
        if (item->staffType()) {
            double stYOffset = item->staffType()->yoffset().val() * sp;
            r.translate(0.0, stYOffset);
        }

        SkylineLine sk(!above);
        double d;
        if (above) {
            sk.add(r.x(), r.bottom(), r.width());
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(r.x(), r.top(), r.width());
            d = ss->skyline().south().minDistance(sk);
        }

        if (d > -minDistance) {
            double yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }

            ldata->moveY(yd);
            r.translate(PointF(0.0, yd));
        }
        if (add && item->addToSkyline()) {
            ss->skyline().add(r);
        }
    }
}

void Autoplace::autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add)
{
    if (item->autoplace() && item->explicitParent()) {
        const Measure* m = toMeasure(item->explicitParent());

        LD_CONDITION(ldata->isSetPos());
        LD_CONDITION(ldata->isSetBbox());
        LD_CONDITION(m->ldata()->isSetPos());

        staff_idx_t si = item->staffIdxOrNextVisible();

        // if there's no good staff for this object, obliterate it
        ldata->setIsSkipDraw(si == muse::nidx);
        const_cast<EngravingItem*>(item)->setSelectable(!ldata->isSkipDraw());
        if (ldata->isSkipDraw()) {
            return;
        }

        double sp = item->style().spatium();
        double minDistance = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);
        // shape rather than bbox is good for tuplets especially
        Shape sh = item->shape().translate(m->pos() + item->pos());

        SkylineLine sk(!above);
        double d;
        if (above) {
            sk.add(sh);
            d = sk.minDistance(ss->skyline().north());
        } else {
            sk.add(sh);
            d = ss->skyline().south().minDistance(sk);
        }
        minDistance *= item->staff()->staffMag(item);
        if (d > -minDistance) {
            double yd = d + minDistance;
            if (above) {
                yd *= -1.0;
            }
            ldata->moveY(yd);
            sh.translateY(yd);
        }
        if (add && item->addToSkyline()) {
            ss->skyline().add(sh);
        }
    }
}

void Autoplace::autoplaceSpannerSegment(const SpannerSegment* item, EngravingItem::LayoutData* ldata, double sp)
{
    if (item->spanner()->anchor() == Spanner::Anchor::NOTE) {
        return;
    }

    if (item->autoplace()) {
        if (!item->systemFlag() && !item->spanner()->systemFlag()) {
            sp *= item->staff()->staffMag(item->spanner()->tick());
        }
        double md = item->minDistance().val() * sp;
        bool above = item->spanner()->placeAbove();
        SkylineLine sl(!above);
        Shape sh = item->shape();
        sl.add(sh.translate(item->pos()));
        double yd = 0.0;
        staff_idx_t stfIdx = item->systemFlag() ? item->staffIdxOrNextVisible() : item->staffIdx();
        if (stfIdx == muse::nidx) {
            ldata->setIsSkipDraw(true);
            return;
        } else {
            ldata->setIsSkipDraw(false);
        }
        if (above) {
            double d = item->system()->topDistance(stfIdx, sl);
            if (d > -md) {
                yd = -(d + md);
            }
        } else {
            double d = item->system()->bottomDistance(stfIdx, sl);
            if (d > -md) {
                yd = d + md;
            }
        }
        if (!RealIsNull(yd)) {
            ldata->moveY(yd);
        }
    }
}

//---------------------------------------------------------
//   doAutoplace
//    check for collisions
//---------------------------------------------------------
void Autoplace::doAutoplace(const Articulation* item, Articulation::LayoutData* ldata)
{
    if (item->autoplace() && item->explicitParent()) {
        Segment* s = item->segment();
        Measure* m = item->measure();
        staff_idx_t si = item->vStaffIdx();

        double sp = item->spatium();
        double md = item->minDistance().val() * sp;

        SysStaff* ss = m->system()->staff(si);

        Shape thisShape = item->shape().translate(item->chordRest()->pos() + m->pos() + s->pos() + item->pos());

        for (const ShapeElement& shapeEl : thisShape.elements()) {
            RectF r = shapeEl;

            double d = 0.0;
            bool above = item->up();
            SkylineLine sk(!above);
            if (above) {
                sk.add(r.x(), r.bottom(), r.width());
                d = sk.minDistance(ss->skyline().north());
            } else {
                sk.add(r.x(), r.top(), r.width());
                d = ss->skyline().south().minDistance(sk);
            }

            if (d > -md) {
                double yd = d + md;
                if (above) {
                    yd *= -1.0;
                }
                ldata->moveY(yd);
                thisShape.translateY(yd);
            }
        }
    }
}
