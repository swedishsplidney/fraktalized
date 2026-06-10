import QtQuick
import QtQuick.Controls
import fraktalizedModule

Window {
    id: rootWindow
    visible: true
    width: 1280
    height: 720
    title: "fraktalized"

    // openGL viewport
    FractalEngine {
        id: engine
        anchors.fill: parent

        maxIterations: Math.round(maxIterationsSlider.value)

        colorTint: Qt.vector3d(rSlider.value, gSlider.value, bSlider.value)

        fractalType: typeSelector.currentIndex

        juliaC: Qt.vector2d(parseFloat(realInput.text), parseFloat(imagInput.text))

        // mouse interaction layer
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton

            // track drag gestures
            property point lastPos: Qt.point(0, 0)

            onPressed: (mouse) => {
                lastPos = Qt.point(mouse.x, mouse.y)

                sidebar.forceActiveFocus()
            }

            onPositionChanged: (mouse) => {
                if (mouse.buttons & Qt.LeftButton) {
                    // calc pixel differences
                    var dx = mouse.x - lastPos.x
                    var dy = mouse.y - lastPos.y

                    engine.panCamera(dx, dy, parent.width, parent.height)

                    lastPos = Qt.point(mouse.x, mouse.y)
                }
            }

            // scroll wheel logic
            onWheel: (wheel) => {
                var zoomFactor = 1.15
                if (wheel.angleDelta.y > 0) {
                    // zoom in
                    engine.zoomLevel /= zoomFactor
                } else {
                    // zoom out
                    engine.zoomLevel *= zoomFactor
                }

                wheel.accepted = true
            }
        }
    }

    // floating panel
    Rectangle {
        id: sidebar
        width: 320
        height: parent.height - 40
        x: 20
        y: 20
        color: "#d00d0e15"
        border.color: "#35ffffff"
        radius: 12
        z: 1
        focus: true

        Column {
            anchors.fill: parent
            anchors.margins: 25
            spacing: 20

            Text {
                text: "fraktalized"
                color: "#7d00ff"
                font.pixelSize: 18
                font.bold: true
                font.family: "Monospace"
            }

            // set changer
            ComboBox {
                id: typeSelector
                width: parent.width
                model: ["mandelbrot set", "julia set"]

                // reset zoom on switch
                onCurrentIndexChanged: {
                    engine.zoomLevel = 2.0
                    engine.zoomCenter = Qt.vector2d(currentIndex === 0 ? -0.5 : 0.0, 0.0)
                }
            }

            // iteration controls
            Column {
                width: parent.width
                spacing: 6

                // header
                Row {
                    width: parent.width
                    height: 24
                    spacing: 6

                    Text {
                        text: "iterations / detail"
                        color: "#8a90a6"
                        font.pixelSize: 13
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    // menu button
                    Button {
                        id: menuButton
                        width: 24
                        height: 24
                        flat: true
                        checkable: true

                        background: Item {
                        } // invis bg

                        contentItem: Text {
                            text: menuButton.checked ? "∨" : ">"
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: menuButton.hovered ? "#7d00ff" : "#8a90a6"
                        }
                    }
                }

                // settings panel
                Column {
                    id: iterationsSettings
                    width: parent.width
                    spacing: 8
                    clip: true

                    visible: menuButton.checked || heightAnimation.running > 0

                    // smooth animation
                    height: menuButton.checked ? 55 : 0

                    Behavior on height {
                        NumberAnimation {
                            id: heightAnimation
                            duration: 150
                        }
                    }

                    Row {
                        width: parent.width
                        spacing: 10

                        // min input
                        Column {
                            width: (parent.width - 10) / 2
                            spacing: 3
                            Text { text: "min value:"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: minLimitInput
                                width: parent.width
                                height: 28
                                text: "1"
                                placeholderText: "0"
                                selectByMouse: true
                                color: "#ffffff"
                                font.pixelSize: 12
                                verticalAlignment: TextInput.AlignVCenter
                                leftPadding: 8

                                background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }

                                onTextChanged: {
                                    var val = parseInt(text)
                                    if (!isNaN(val) && val >= 0) maxIterationsSlider.from = val
                                }
                            }
                        }

                        // max input
                        Column {
                            width: (parent.width - 10) / 2
                            spacing: 3
                            Text { text: "max value:"; color: "#aaaaaa"; font.pixelSize: 10 }
                            TextField {
                                id: maxLimitInput
                                width: parent.width
                                height: 28
                                text: "500"
                                placeholderText: "0"
                                selectByMouse: true
                                color: "#ffffff"
                                font.pixelSize: 12
                                verticalAlignment: TextInput.AlignVCenter
                                leftPadding: 8

                                background: Rectangle { color: "#222222"; radius: 4; border.color: "#444444" }

                                onTextChanged: {
                                    var val = parseInt(text)
                                    if (!isNaN(val)) {
                                        maxIterationsSlider.to = val
                                    }
                                }
                            }
                        }
                    }
                }

                // warn box
                Rectangle {
                    id: warnBox
                    width: parent.width
                    clip: true
                    color: "#2a0808"
                    border.color: "#ff3333"
                    border.width: 1
                    radius: 6

                    property bool isDismissed: false
                    property bool shouldShow: (Math.round(maxIterationsSlider.value) > 1500) && !isDismissed

                    visible: shouldShow || warnAnimation.running
                    height: shouldShow ? 60 : 0

                    Behavior on height { NumberAnimation { id: warnAnimation; duration: 150 } }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 4

                        Text {
                            text: "warning"
                            color: "#ff3333"
                            font.pixelSize: 10
                            font.bold: true
                        }
                        Text {
                            text: "values over 1500 iterations can cause problems on low-end hardware"
                            color: "#ffaaaa"
                            font.pixelSize: 9
                            wrapMode: Text.Wrap
                            width: parent.width - 10
                        }
                    }

                    // dismiss button
                    Button {
                        id: okButton
                        width: 36
                        height: 20
                        anchors.bottom: parent.bottom
                        anchors.right: parent.right
                        anchors.margins: 6

                        background: Rectangle {
                            color: okButton.hovered ? "#ff3333" : "#4a1212"
                            radius: 4
                            border.color: "#ff3333"
                            border.width: 1

                            Behavior on color { ColorAnimation { duration: 100 } }
                        }

                        contentItem: Text {
                            text: "ok"
                            color: okButton.hovered ? "#2a0808" : "#ff3333"
                            font.pixelSize: 10
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            warnBox.isDismissed = true
                        }
                    }
                }

                Column {
                    width: parent.width
                    spacing: -4

                    // slider
                    Slider {
                        id: maxIterationsSlider
                        width: parent.width
                        from: 10
                        to: 500
                        value: 100

                        onPressedChanged: if (pressed) forceActiveFocus()
                    }

                    // display current value and have a clickable box
                    TextField {
                        id: currentValueInput
                        width: parent.width
                        height: 30
                        color: activeFocus ? "#ffffff" : "#8a90a6"
                        font.pixelSize: 10
                        font.family: "Monospace"
                        selectByMouse: true
                        verticalAlignment: TextInput.AlignVCenter
                        leftPadding: 4

                        property bool isOutOfBounds: false

                        Binding on text {
                            when: !currentValueInput.activeFocus && !currentValueInput.isOutOfBounds
                            value: "current: " + Math.round(maxIterationsSlider.value)
                        }

                        // numbers only
                        validator: IntValidator {
                            bottom: 0
                            top: 2147483647
                        }

                        background: Rectangle {
                            color: parent.activeFocus ? "#221133" : "transparent"
                            border.color: parent.activeFocus ? "#7d00ff" : "transparent"
                            border.width: 1
                            radius: 4
                        }

                        Timer {
                            id: resetTimer
                            interval: 1200
                            onTriggered: {
                                currentValueInput.isOutOfBounds = false
                            }
                        }

                        onEditingFinished: {
                            var val = parseInt(text)
                            if (!isNaN(val)) {
                                // make sure its within the set range
                                if ((val > maxIterationsSlider.to) || (val < maxIterationsSlider.from)) {
                                    currentValueInput.isOutOfBounds = true
                                    currentValueInput.text = "outside of currently set range"
                                    resetTimer.restart()
                                }

                                val = Math.max(maxIterationsSlider.from, Math.min(val, maxIterationsSlider.to))
                                maxIterationsSlider.value = val
                            }
                            deselect()
                            focus = false
                            rootWindow.contentItem.focus = true
                            rootWindow.contentItem.forceActiveFocus()
                        }

                        onActiveFocusChanged: {
                            if (activeFocus) {
                                resetTimer.stop()
                                currentValueInput.isOutOfBounds = false
                                text = Math.round(maxIterationsSlider.value).toString()
                                selectAll()
                            }
                        }
                    }
                }
            }

            // color controls
            Text {
                text: "color setup:"
                color: "#8a90a6"
                font.pixelSize: 13
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "red:"; color: "#ff5555"; font.pixelSize: 11 }
                Slider { id: rSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "green:"; color: "#55ff55"; font.pixelSize: 11 }
                Slider { id: gSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            Column {
                width: parent.width
                spacing: 5
                Text { text: "blue:"; color: "#5555ff"; font.pixelSize: 11 }
                Slider { id: bSlider; width: parent.width; from: 0.0; to: 2.0; value: 0.2; onPressedChanged: if (pressed) forceActiveFocus() }
            }

            // julia set constant value setter
            Column {
                width: parent.width
                spacing: 8
                // only show when julia set
                visible: typeSelector.currentIndex === 1

                Text {
                    text: "julia constant (c):"
                    color: "#7d00ff"
                    font.pixelSize: 12; font.family: "Monospace"
                }

                Row {
                    width: parent.width
                    spacing: 10

                    // real value
                    Column {
                        width: (parent.width - 10) / 2
                        spacing: 3
                        Text { text: "real (x):"; color: "#aaaaaa"; font.pixelSize: 10 }
                        TextField {
                            id: realInput
                            width: parent.width
                            text: "-0.7"
                            placeholderText: "0.0"
                            selectByMouse: true
                            color: "#ffffff"
                            background: Rectangle { color: "#222222"; radius: 4}
                        }
                    }

                    // imaginary value
                    Column {
                        width: (parent.width - 10) / 2
                        spacing: 3
                        Text { text: "imag (y):"; color: "#aaaaaa"; font.pixelSize: 10 }
                        TextField {
                            id: imagInput
                            width: parent.width
                            text: "0.27015"
                            placeholderText: "0.0"
                            selectByMouse: true
                            color: "#ffffff"
                            background: Rectangle { color: "#222222"; radius: 4 }
                        }
                    }
                }
            }
        }
    }
}