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
import QtQuick 2.15
import QtQuick.Controls 2.15

StyledListView {
    id: root

    property alias radioButtonGroup: buttonGroup

    implicitHeight: Math.max(1, contentItem.childrenRect.height)
    implicitWidth: contentWidth

    spacing: 4

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled
    orientation: ListView.Horizontal
    interactive: false
    clip: false

    ButtonGroup {
        id: buttonGroup
    }
}
