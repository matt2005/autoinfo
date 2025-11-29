import QtQuick 2.15

Item {
    // Minimal styled button stub for linting
    property alias text: _text
    property bool enabled: true
    signal clicked()

    property string _text: ""

    function click() { clicked(); }
}
/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15

// Minimal stub for linting: acts like a normal Button so types and properties
// such as `text`, `width`, `height`, `enabled`, `background` resolve for qmllint.
Button {
  id: control
  Accessible.name: ""
}
