// []: # ```plaintext
// []: #  * Project: Crankshaft
// []: #  * This file is part of Crankshaft project.
// []: #  * Copyright (C) 2025 OpenCarDev Team
// []: #  *
// []: #  *  Crankshaft is free software: you can redistribute it and/or modify
// []: #  *  it under the terms of the GNU General Public License as published by
// []: #  *  the Free Software Foundation; either version 3 of the License, or
// []: #  *  (at your option) any later version.
// []: #  *
// []: #  *  Crankshaft is distributed in the hope that it will be useful,
// []: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// []: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// []: #  *  GNU General Public License for more details.
// []: #  *
// []: #  *  You should have received a copy of the GNU General Public License
// []: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
// []: # ```

import QtQuick

Item {
  id: root
  // Externally bound shortcuts (single-character keys by default)
  property string openSettings: "S"
  property string toggleTheme: "T"
  property string goHome: "H"
  property string cycleLeft: "A"
  property string cycleRight: "D"
  property string showHelp: "?"

  // Signals to let parent decide what to do
  signal openSettingsRequested()
  signal toggleThemeRequested()
  signal goHomeRequested()
  signal cycleLeftRequested()
  signal cycleRightRequested()
  signal toggleHelpRequested()

  focus: true

  function _matchKey(eventText, expected) {
    if (!eventText || !expected) return false;
    return eventText.toString().toUpperCase() === expected.toString().toUpperCase();
  }

  Keys.onPressed: {
    if (!event.text)
      return;
    if (_matchKey(event.text, openSettings)) {
      root.openSettingsRequested();
      event.accepted = true;
    } else if (_matchKey(event.text, toggleTheme)) {
      root.toggleThemeRequested();
      event.accepted = true;
    } else if (_matchKey(event.text, goHome)) {
      root.goHomeRequested();
      event.accepted = true;
    } else if (_matchKey(event.text, cycleLeft)) {
      root.cycleLeftRequested();
      event.accepted = true;
    } else if (_matchKey(event.text, cycleRight)) {
      root.cycleRightRequested();
      event.accepted = true;
    } else if (_matchKey(event.text, showHelp)) {
      root.toggleHelpRequested();
      event.accepted = true;
    }
  }
}
