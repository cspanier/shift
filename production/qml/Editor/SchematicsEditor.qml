import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Controls.Universal 2.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.3
import QtQuick.Shapes 1.0
import Editor 1.0

Window {
  id: schematicsEditor
  title: "Shift Schematics Editor"
  flags: Qt.Tool
  width: 640
  height: 480
  visible: true

  Shape {
    anchors.fill: parent

    Repeater {
      id: node
      model: schematicsController.nodeModel

      Rectangle {
        x: 100 * index
        y: 10
        width: 80
        height: 200

        Label {
          x: 66
          y: 14
          text: modelData.name
        }

        Repeater {
          model: modelData.inputPortsModel

          Rectangle {
            x: 5
            y: 21 + 10 * index
            width: 8
            height: 8
            color: "red"
            radius: width * 0.5
          }
        }

        Repeater {
          model: modelData.outputPortsModel

          Rectangle {
            x: 25
            y: 21 + 10 * index
            width: 8
            height: 8
            color: "green"
            radius: width * 0.5
          }
        }
      }
    }

    ShapePath {
      strokeColor: "red"
      strokeWidth: 20
      fillColor: "transparent"
      joinStyle: ShapePath.RoundJoin

      startX: 20; startY: 20
      PathLine { x: 100; y: 100 }
      PathLine { x: 20; y: 150 }
      PathLine { x: 20; y: 20 }
    }

    ShapePath {
      strokeColor: "black"
      strokeWidth: 20
      capStyle: ShapePath.RoundCap

      startX: 150; startY: 20
      PathCubic {
        x: 150
        y: 150

        control1X: 120
        control1Y: 50

        control2X: 200
        control2Y: 200
      }
    }
  }
}
