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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"

Column {
    id: root

    width: parent.width

    required property QtObject model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    spacing: 12

    InspectorPropertyView {
        id: applyToVoiceSection

        titleText: qsTrc("inspector", "Apply to voice")

        propertyItem: root.model ? root.model.applyToVoice : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
        navigationRowEnd: individualVoicesSection.navigationRowEnd

        FlatRadioButtonList {
            id: allVoicesSection

            navigationPanel: root.navigationPanel
            navigationRowStart: parent.navigationRowStart

            visible: root.model ? root.model.isMultiStaffInstrument : false

            height: 30
            width: parent.width

            currentValue: parent.propertyItem && !parent.propertyItem.isUndefined ? parent.propertyItem.value : undefined

            onToggled: function(newValue) {
                if (parent.propertyItem) {
                    parent.propertyItem.value = newValue
                }
            }

            model: [
                { text: qsTrc("inspector", "All in instrument"), value: VoiceTypes.VOICE_ALL_IN_INSTRUMENT },
                { text: qsTrc("inspector", "All in this staff"), value: VoiceTypes.VOICE_ALL_IN_STAFF },
            ]
        }

        FlatRadioButtonList {
            id: individualVoicesSection

            navigationPanel: root.navigationPanel
            navigationRowStart: allVoicesSection.navigationRowEnd + 1

            height: 30
            width: parent.width

            currentValue: parent.propertyItem && !parent.propertyItem.isUndefined ? parent.propertyItem.value : undefined

            onToggled: function(newValue) {
                if (parent.propertyItem) {
                    parent.propertyItem.value = newValue
                }
            }

            model: root.model && root.model.isMultiStaffInstrument ? [
                { iconCode: IconCode.VOICE_1, value: VoiceTypes.VOICE_ONE },
                { iconCode: IconCode.VOICE_2, value: VoiceTypes.VOICE_TWO },
                { iconCode: IconCode.VOICE_3, value: VoiceTypes.VOICE_THREE },
                { iconCode: IconCode.VOICE_4, value: VoiceTypes.VOICE_FOUR }
            ] : [
                { text: qsTrc("inspector", "All"), value: VoiceTypes.VOICE_ALL_IN_INSTRUMENT},
                { iconCode: IconCode.VOICE_1, value: VoiceTypes.VOICE_ONE },
                { iconCode: IconCode.VOICE_2, value: VoiceTypes.VOICE_TWO },
                { iconCode: IconCode.VOICE_3, value: VoiceTypes.VOICE_THREE },
                { iconCode: IconCode.VOICE_4, value: VoiceTypes.VOICE_FOUR }
            ]
        }
    }

    FlatRadioButtonGroupPropertyView {
        id: positionSection

        titleText: qsTrc("inspector", "Position")

        propertyItem: root.model ? root.model.voiceBasedPosition : null

        navigationPanel: root.navigationPanel
        navigationRowStart: applyToVoiceSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO },
            { text: qsTrc("inspector", "Above"), value: DirectionTypes.VERTICAL_UP },
            { text: qsTrc("inspector", "Below"), value: DirectionTypes.VERTICAL_DOWN }
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: centerStavesSection

        visible: root.model ? root.model.isMultiStaffInstrument : false
        enabled: root.model ? root.model.isStaveCenteringAvailable : false

        titleText: qsTrc("inspector", "Center between staves")

        propertyItem: root.model ? root.model.centerBetweenStaves : null

        navigationPanel: root.navigationPanel
        navigationRowStart: positionSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.CENTER_STAVES_AUTO },
            { text: qsTrc("inspector", "On"), value: DirectionTypes.CENTER_STAVES_ON },
            { text: qsTrc("inspector", "Off"), value: DirectionTypes.CENTER_STAVES_OFF }
        ]
    }
}
