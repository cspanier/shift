pragma Singleton
import QtQuick 2.9

QtObject {
    id: singleton

    // Keep these constants in sync with the enumeration on C++ side.
    // Note: We don't export the C++ enum to QML to avoid LGPL complications.
    readonly property int idle: 0
    readonly property int copy: 1
    readonly property int validate: 2
    readonly property int apply: 3
    readonly property int revert: 4
    readonly property int success: 5
    readonly property int error: 6
}
