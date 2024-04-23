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
#include "inspectormodelwithvoiceandpositionoptions.h"

using namespace mu::inspector;
using namespace mu::engraving;

InspectorModelWithVoiceAndPositionOptions::InspectorModelWithVoiceAndPositionOptions(QObject* parent, IElementRepositoryService* repository,
                                                                                     ElementType elementType)
    : AbstractInspectorModel(parent, repository, elementType)
{
    createProperties();
}

void InspectorModelWithVoiceAndPositionOptions::createProperties()
{
    m_voiceBasedPosition = buildPropertyItem(Pid::DIRECTION);
    m_applyToVoice = buildPropertyItem(Pid::APPLY_TO_VOICE);
    m_centerBetweenStaves = buildPropertyItem(Pid::CENTER_BETWEEN_STAVES);
    updateIsMultiStaffInstrument();
    updateIsStaveCenteringAvailable();
}

void InspectorModelWithVoiceAndPositionOptions::loadProperties()
{
    loadPropertyItem(m_voiceBasedPosition);
    loadPropertyItem(m_applyToVoice);
    loadPropertyItem(m_centerBetweenStaves);
    updateIsMultiStaffInstrument();
    updateIsStaveCenteringAvailable();
}

void InspectorModelWithVoiceAndPositionOptions::resetProperties()
{
    m_voiceBasedPosition->resetToDefault();
    m_applyToVoice->resetToDefault();
    m_centerBetweenStaves->resetToDefault();
}

void InspectorModelWithVoiceAndPositionOptions::updateIsMultiStaffInstrument()
{
    bool isMultiStaffInstrument = true;
    for (EngravingItem* item : m_elementList) {
        if (item->part()->nstaves() <= 1) {
            isMultiStaffInstrument = false;
            break;
        }
    }

    setIsMultiStaffInstrument(isMultiStaffInstrument);
}

void InspectorModelWithVoiceAndPositionOptions::updateIsStaveCenteringAvailable()
{
    if (!m_isMultiStaffInstrument) {
        setIsStaveCenteringAvailable(false);
        return;
    }

    bool isStaveCenteringAvailable = true;
    for (EngravingItem* item : m_elementList) {
        staff_idx_t thisStaffIdx = item->staffIdx();
        bool above = item->placeAbove();
        const std::vector<Staff*>& partStaves = item->part()->staves();
        staff_idx_t firstStaffOfPart = partStaves.front()->idx();
        staff_idx_t lastStaffOfPart = partStaves.back()->idx();
        if ((above && thisStaffIdx == firstStaffOfPart) || (!above && thisStaffIdx == lastStaffOfPart)) {
            isStaveCenteringAvailable = false;
            break;
        }
    }

    setIsStaveCenteringAvailable(isStaveCenteringAvailable);
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::voiceBasedPosition() const
{
    return m_voiceBasedPosition;
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::applyToVoice() const
{
    return m_applyToVoice;
}

PropertyItem* InspectorModelWithVoiceAndPositionOptions::centerBetweenStaves() const
{
    return m_centerBetweenStaves;
}

bool InspectorModelWithVoiceAndPositionOptions::isMultiStaffInstrument() const
{
    return m_isMultiStaffInstrument;
}

bool InspectorModelWithVoiceAndPositionOptions::isStaveCenteringAvailable() const
{
    return m_isStaveCenteringAvailable;
}

void InspectorModelWithVoiceAndPositionOptions::setIsMultiStaffInstrument(bool v)
{
    if (v == m_isMultiStaffInstrument) {
        return;
    }

    m_isMultiStaffInstrument = v;
    emit isMultiStaffInstrumentChanged(m_isMultiStaffInstrument);
}

void InspectorModelWithVoiceAndPositionOptions::setIsStaveCenteringAvailable(bool v)
{
    if (v == m_isStaveCenteringAvailable) {
        return;
    }

    m_isStaveCenteringAvailable = v;
    emit isStaveCenteringAvailableChanged(m_isStaveCenteringAvailable);
}
