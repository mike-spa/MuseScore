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

#include "measurebase.h"
#include "system.h"
#include "systemlock.h"

#include "log.h"

namespace mu::engraving {
bool SystemLock::contains(const MeasureBase* mb) const
{
    return (m_startMeasure->isBefore(mb) || m_startMeasure == mb)
           && (mb->isBefore(m_endMeasure) || mb == m_endMeasure);
}

void SystemLocks::add(const SystemLock& lock)
{
    m_systemLocks.emplace(lock.startMeasure(), lock);

#ifndef NDEBUG
    sanityCheck();
#endif
}

void SystemLocks::remove(const MeasureBase* start)
{
    m_systemLocks.erase(start);

#ifndef NDEBUG
    sanityCheck();
#endif
}

const SystemLock* SystemLocks::lockClosestTo(const MeasureBase* mb) const
{
    if (m_systemLocks.empty()) {
        return nullptr;
    }

    return &((*m_systemLocks.lower_bound(mb)).second);
}

void SystemLocks::sanityCheck()
{
    for (auto iter = m_systemLocks.begin(); iter != m_systemLocks.end();) {
        auto curIter = iter;
        auto nextIter = ++iter;
        if (nextIter == m_systemLocks.end()) {
            break;
        }

        const MeasureBase* curMB = (*curIter).first;
        const SystemLock& curSysLock = (*curIter).second;

        DO_ASSERT(curSysLock.startMeasure() == curMB);

        const MeasureBase* nextMB = (*nextIter).first;
        const SystemLock& nextSysLock = (*nextIter).second;

        DO_ASSERT(nextSysLock.startMeasure() == nextMB);
        DO_ASSERT(curMB->isBefore(nextMB));
        DO_ASSERT(curSysLock.endMeasure()->isBefore(nextSysLock.startMeasure()));
    }
}

SystemLockIndicator::SystemLockIndicator(Type type, MeasureBase* parent)
    : m_type(type), EngravingItem(ElementType::SYSTEM_LOCK_INDICATOR, parent, ElementFlag::SYSTEM | ElementFlag::GENERATED) {}

SystemLockIndicator::SystemLockIndicator(const SystemLockIndicator& sli)
    : EngravingItem(sli)
{
    m_type = sli.m_type;
}
} // namespace mu::engraving
