import QtQuick 2.12

QtObject {
  property string currentLocale: "en-GB"
  function extensionTranslationStatus() { return [] }
  function refreshTranslations() { }
}
/*
 * Project: Crankshaft (lint stub)
 * Minimal I18nManager stub for qmllint.
 */
import QtQuick 2.15

QtObject {
    property string currentLocale: "en-GB"
    function extensionTranslationStatus() {
        return [];
    }
    function refreshTranslations() {
        // no-op
    }
}
