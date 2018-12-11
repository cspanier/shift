import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Universal 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.10
import Editor 1.0

ApplicationWindow {
  id: root
  title: "Shift Editor"
  flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
  color: "#00000000"
  width: 300
  height: 1024
  visible: true

  x: editor.windowPosition.x - width
  y: editor.windowPosition.y

  Column {
    anchors.fill: parent

    Pane {
      anchors.left: parent.left
      anchors.right: parent.right

      Flow {
        id: menuFlow
        anchors.fill: parent

        EditorButton {
          imageSource: "../icons/editor/ic_create_new_folder_black_48dp.png"
          // imageSource: "../icons/editor/ic_create_new_folder_48px.svg"
          ToolTip.text: qsTr("Create a new empty project")
        }
        EditorButton {
          imageSource: "../icons/editor/ic_folder_open_black_48dp.png"
          // imageSource: "../icons/editor/ic_folder_open_48px.svg"
          ToolTip.text: qsTr("Load a project from file")
        }
        EditorButton {
          imageSource: "../icons/editor/ic_save_black_48dp.png"
          // imageSource: "../icons/editor/ic_save_48px.svg"
          ToolTip.text: qsTr("Save the active project")
        }
        EditorButton {
          imageSource: "../icons/editor/cursor_arrow_icon&48.png"
          ToolTip.text: qsTr("Select object tool")
        }
        EditorButton {
          imageSource: "../icons/editor/ic_open_with_black_48dp.png"
          // imageSource: "../icons/editor/ic_open_with_48px.svg"
          ToolTip.text: qsTr("Move object tool")
        }
        EditorButton {
          imageSource: "../icons/editor/ic_rotate_left_black_48dp.png"
          // imageSource: "../icons/editor/ic_rotate_left_48px.svg"
          ToolTip.text: qsTr("Rotate object tool")
        }
        EditorButton {
          imageSource: "../icons/editor/ic_photo_size_select_small_black_48dp.png"
          // imageSource: "../icons/editor/ic_photo_size_select_small_24px.svg"
          ToolTip.text: qsTr("Scale object tool")
        }

        EditorButton {
          imageSource: "../icons/editor/folder_open_icon&32.png"
          onClicked: {
            schematicsEditor.show();
          }
        }
      }
    }

    Pane {
      anchors.left: parent.left
      anchors.right: parent.right
      height: 250
    }
  }

  SchematicsEditor {
    id: schematicsEditor
    visible: false
  }
}
