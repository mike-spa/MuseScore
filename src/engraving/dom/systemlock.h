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

#ifndef MU_ENGRAVING_SYSTEM_LOCK_H
#define MU_ENGRAVING_SYSTEM_LOCK_H

#include <map>

#include "measurebase.h"

namespace mu::engraving {
class SystemLock
{
public:
    SystemLock(const MeasureBase* start, const MeasureBase* end)
        : m_startMeasure(start), m_endMeasure(end) { assert(m_startMeasure->isBefore(m_endMeasure) || m_startMeasure == m_endMeasure); }

    const MeasureBase* startMeasure() const { return m_startMeasure; }
    const MeasureBase* endMeasure() const { return m_endMeasure; }

private:
    const MeasureBase* m_startMeasure;
    const MeasureBase* m_endMeasure;
};

class SystemLocks
{
public:
    void add(const SystemLock& lock);
    void add(const MeasureBase* start, const MeasureBase* end) { add(SystemLock(start, end)); }
    void remove(const MeasureBase* start);
    void remove(const SystemLock& lock) { remove(lock.startMeasure()); }
    void clear() { m_systemLocks.clear(); }

private:
    void sanityCheck();

    struct Ordering
    {
        bool operator()(const MeasureBase* a, const MeasureBase* b) const
        {
            return a->isBefore(b);
        }
    };

    std::map<const MeasureBase*, SystemLock, Ordering> m_systemLocks;
};
} // namespace mu::engraving

#endif // MU_ENGRAVING_SYSTEM_LOCK_H
