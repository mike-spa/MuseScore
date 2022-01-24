/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "playtechannotation.h"

#include "rw/xml.h"
#include "types/typesconv.h"
#include "segment.h"

using namespace mu::engraving;
using namespace Ms;

static const ElementStyle annotationStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

PlayTechAnnotation::PlayTechAnnotation(Segment* parent, PlayingTechniqueType techniqueType, TextStyleType tid)
    : StaffTextBase(ElementType::PLAYTECH_ANNOTATION, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF),
    m_techniqueType(techniqueType)
{
    initElementStyle(&annotationStyle);
}

void PlayTechAnnotation::setTechniqueType(const PlayingTechniqueType techniqueType)
{
    m_techniqueType = techniqueType;
}

PlayTechAnnotation* PlayTechAnnotation::clone() const
{
    return new PlayTechAnnotation(*this);
}

void PlayTechAnnotation::layout()
{
    StaffTextBase::layout();
    autoplaceSegmentElement();
}

void PlayTechAnnotation::write(XmlWriter& writer) const
{
    writer.startObject(this);
    writeProperty(writer, Pid::PLAY_TECH_TYPE);
    StaffTextBase::writeProperties(writer);
    writer.endObject();
}

void PlayTechAnnotation::read(XmlReader& reader)
{
    while (reader.readNextStartElement()) {
        const QStringRef& tag(reader.name());

        if (readProperty(tag, reader, Pid::PLAY_TECH_TYPE)) {
            continue;
        }

        if (!StaffTextBase::readProperties(reader)) {
            reader.unknown();
        }
    }
}

PropertyValue PlayTechAnnotation::getProperty(Pid id) const
{
    switch (id) {
    case Pid::PLAY_TECH_TYPE:
        return m_techniqueType;
    default:
        return StaffTextBase::getProperty(id);
    }
}

bool PlayTechAnnotation::setProperty(Pid propertyId, const mu::engraving::PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PLAY_TECH_TYPE:
        m_techniqueType = PlayingTechniqueType(val.toInt());
        break;
    default:
        if (!StaffTextBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }

    triggerLayout();
    return true;
}

PropertyValue PlayTechAnnotation::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    case Pid::PLAY_TECH_TYPE:
        return PlayingTechniqueType::Natural;
    default:
        return StaffTextBase::propertyDefault(id);
    }
}
