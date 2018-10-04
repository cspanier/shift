import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Controls.Universal 2.3

Button {
  property url imageSource

  width: height

  ToolTip.visible: ToolTip.text.length > 0 && hovered
  ToolTip.delay: 500
  ToolTip.timeout: 5000

  Image {
    anchors.fill: parent
    fillMode: Image.PreserveAspectFit
    source: imageSource
  }
}
