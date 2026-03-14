import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import MyMusicPlayer 1.0

ApplicationWindow {
    id: window

    width: 1140
    height: 720
    minimumWidth: 980
    minimumHeight: 620
    visible: true
    title: "音乐魔盒"

    Material.theme: Material.Dark
    Material.accent: "#22c55e"
    Material.primary: "#111827"
    Material.background: "#0b0d12"
    Material.foreground: "#e5e7eb"

    readonly property color bg: "#0b0d12"
    readonly property color panel: "#111520"
    readonly property color panel2: "#0f131c"
    readonly property color border: "#232836"
    readonly property color textMuted: "#9ca3af"

    property int selectedIndex: player.currentIndex
    property string filterText: ""
    property bool dragOverlayVisible: false

    background: Rectangle { color: bg }

    function formatTime(ms) {
        if (ms === undefined || ms === null) return "00:00"
        const totalSec = Math.max(0, Math.floor(ms / 1000))
        const min = Math.floor(totalSec / 60)
        const sec = totalSec % 60
        return String(min).padStart(2, "0") + ":" + String(sec).padStart(2, "0")
    }

    function playbackModeLabel(mode) {
        switch (mode) {
        case PlayerEngine.Sequential:
            return "顺序"
        case PlayerEngine.Loop:
            return "循环"
        case PlayerEngine.Random:
            return "随机"
        case PlayerEngine.CurrentItemInLoop:
            return "单曲"
        default:
            return ""
        }
    }

    Shortcut { sequence: "Space"; onActivated: player.togglePlay() }
    Shortcut { sequence: "Ctrl+O"; onActivated: app.pickAndAddFiles() }
    Shortcut { sequence: "Ctrl+F"; onActivated: searchField.forceActiveFocus() }
    Shortcut { sequence: "Escape"; onActivated: { if (searchField.activeFocus) { searchField.text = "" } } }

    Connections {
        target: player
        function onCurrentIndexChanged() { selectedIndex = player.currentIndex }
    }

    DropArea {
        anchors.fill: parent
        onEntered: dragOverlayVisible = true
        onExited: dragOverlayVisible = false
        onDropped: {
            dragOverlayVisible = false
            app.addDroppedUrls(drop.urls)
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: dragOverlayVisible
        enabled: false
        z: 999
        color: "#00000080"

        Rectangle {
            width: Math.min(parent.width - 80, 520)
            height: 140
            anchors.centerIn: parent
            radius: 14
            color: panel
            border.color: Material.accent
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 18
                spacing: 8
                Label {
                    Layout.fillWidth: true
                    text: "释放以添加到播放队列"
                    font.pixelSize: 18
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                Label {
                    Layout.fillWidth: true
                    text: "支持拖拽多个本地音频文件"
                    color: textMuted
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }

    Menu {
        id: mainMenu

        MenuItem { text: "添加歌曲..."; onTriggered: app.pickAndAddFiles() }
        MenuItem { text: "清空队列"; enabled: playlistView.count > 0; onTriggered: player.clear() }
        MenuSeparator {}

        Menu {
            title: "播放模式"
            MenuItem { text: "顺序播放"; checkable: true; checked: player.playbackMode === PlayerEngine.Sequential; onTriggered: player.playbackMode = PlayerEngine.Sequential }
            MenuItem { text: "列表循环"; checkable: true; checked: player.playbackMode === PlayerEngine.Loop; onTriggered: player.playbackMode = PlayerEngine.Loop }
            MenuItem { text: "随机播放"; checkable: true; checked: player.playbackMode === PlayerEngine.Random; onTriggered: player.playbackMode = PlayerEngine.Random }
            MenuItem { text: "单曲循环"; checkable: true; checked: player.playbackMode === PlayerEngine.CurrentItemInLoop; onTriggered: player.playbackMode = PlayerEngine.CurrentItemInLoop }
        }

        MenuSeparator {}
        MenuItem { text: "登录..."; onTriggered: loginDialog.open() }
        MenuItem { text: "关于"; onTriggered: aboutDialog.open() }
        MenuSeparator {}
        MenuItem { text: "退出"; onTriggered: Qt.quit() }
    }

    Item {
        id: layoutRoot
        anchors.fill: parent

        ToolBar {
            id: topBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top

            background: Rectangle {
                color: panel
                border.color: border
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Image {
                    source: "qrc:/resources/img/headphones.png"
                    width: 22
                    height: 22
                    fillMode: Image.PreserveAspectFit
                }

                Label {
                    text: "音乐魔盒"
                    font.pixelSize: 16
                    font.bold: true
                }

                TextField {
                    id: searchField
                    Layout.fillWidth: true
                    placeholderText: "搜索歌曲或歌手（Ctrl+F）"
                    text: filterText
                    onTextChanged: filterText = text
                }

                ToolButton {
                    text: "添加"
                    onClicked: app.pickAndAddFiles()
                }

                ToolButton {
                    text: playbackModeLabel(player.playbackMode)
                    onClicked: {
                        const nextMode = (player.playbackMode + 1) % 4
                        player.playbackMode = nextMode
                    }
                    ToolTip.visible: hovered
                    ToolTip.text: "播放模式：" + playbackModeLabel(player.playbackMode)
                }

                ToolButton {
                    text: "菜单"
                    onClicked: mainMenu.popup()
                }
            }
        }

        Rectangle {
            id: bottomBar
            readonly property int pad: 10
            readonly property int minHeight: 96
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: Math.max(minHeight, bottomLayout.implicitHeight + pad * 2)
            color: panel
            border.color: border

            ColumnLayout {
                id: bottomLayout
                anchors.fill: parent
                anchors.margins: bottomBar.pad
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label { text: formatTime(player.position); color: textMuted }
                    Slider {
                        id: progress
                        Layout.fillWidth: true
                        from: 0
                        to: Math.max(1, player.duration)
                        value: player.position
                        onMoved: player.position = value
                    }
                    Label { text: formatTime(player.duration); color: textMuted }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Label {
                        text: player.currentTitle.length ? (player.currentTitle + " - " + player.currentArtist) : ""
                        elide: Text.ElideRight
                        color: Material.foreground
                        Layout.preferredWidth: 340
                    }

                    Item { Layout.fillWidth: true }

                    ToolButton {
                        text: "⏮"
                        enabled: playlistView.count > 0
                        font.pixelSize: 18
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 34
                        onClicked: player.previous()
                        ToolTip.visible: hovered
                        ToolTip.text: "上一曲"
                    }

                    ToolButton {
                        text: player.playing ? "⏸" : "⏵"
                        enabled: playlistView.count > 0
                        font.pixelSize: 20
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 34
                        onClicked: player.togglePlay()
                        ToolTip.visible: hovered
                        ToolTip.text: player.playing ? "暂停（Space）" : "播放（Space）"
                    }

                    ToolButton {
                        text: "⏭"
                        enabled: playlistView.count > 0
                        font.pixelSize: 18
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 34
                        onClicked: player.next()
                        ToolTip.visible: hovered
                        ToolTip.text: "下一曲"
                    }

                    Item { Layout.fillWidth: true }

                    ToolButton {
                        text: player.muted ? "🔇" : "🔊"
                        font.pixelSize: 18
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 34
                        onClicked: player.muted = !player.muted
                        ToolTip.visible: hovered
                        ToolTip.text: player.muted ? "取消静音" : "静音"
                    }
                    Slider {
                        Layout.preferredWidth: 150
                        from: 0
                        to: 100
                        value: player.volume
                        onMoved: player.volume = value
                    }
                }
            }
        }

        SplitView {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: topBar.bottom
            anchors.bottom: bottomBar.top
            clip: true
            orientation: Qt.Horizontal

        Pane {
            id: sidebar
            SplitView.preferredWidth: 380
            SplitView.minimumWidth: 320

            padding: 12
            background: Rectangle {
                color: panel2
                border.color: border
            }

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Label {
                        text: "播放队列"
                        font.pixelSize: 16
                        font.bold: true
                        Layout.fillWidth: true
                    }

                    Label {
                        text: playlistView.count > 0 ? (playlistView.count + " 首") : ""
                        color: textMuted
                    }

                    ToolButton {
                        text: "清空"
                        enabled: playlistView.count > 0
                        onClicked: player.clear()
                    }
                }

                ListView {
                    id: playlistView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: player.playlist
                    spacing: 2

                    delegate: ItemDelegate {
                        width: playlistView.width
                        hoverEnabled: true

                        readonly property bool matchesFilter: filterText.length === 0
                            || title.toLowerCase().indexOf(filterText.toLowerCase()) >= 0
                            || artist.toLowerCase().indexOf(filterText.toLowerCase()) >= 0

                        visible: matchesFilter
                        height: visible ? implicitHeight : 0

                        highlighted: index === selectedIndex

                        onClicked: {
                            selectedIndex = index
                            player.currentIndex = index
                            player.play()
                        }

                        contentItem: RowLayout {
                            spacing: 10
                            Rectangle {
                                width: 3
                                height: 34
                                radius: 1.5
                                color: index === player.currentIndex ? Material.accent : "transparent"
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: title
                                    elide: Text.ElideRight
                                    font.pixelSize: 14
                                    font.bold: index === player.currentIndex
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: artist
                                    elide: Text.ElideRight
                                    color: textMuted
                                    font.pixelSize: 12
                                }
                            }

                            ToolButton {
                                visible: hovered || index === selectedIndex
                                text: "删除"
                                onClicked: {
                                    const removing = index
                                    player.removeAt(removing)
                                    if (selectedIndex === removing) {
                                        selectedIndex = player.currentIndex
                                    }
                                }
                            }
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                }

                Rectangle {
                    Layout.fillWidth: true
                    visible: playlistView.count === 0
                    height: 110
                    radius: 12
                    color: "#0b0d12"
                    border.color: border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 8
                        Label { text: "还没有音乐"; font.bold: true; font.pixelSize: 14 }
                        Label { text: "点击“添加”或将文件拖拽到窗口"; color: textMuted; wrapMode: Text.Wrap }
                    }
                }
            }
        }

        Item {
            id: mainPane
            SplitView.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    height: 150
                    radius: 16
                    color: panel
                    border.color: border

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 14

                        Rectangle {
                            width: 120
                            height: 120
                            radius: 16
                            color: "#0b0d12"
                            border.color: border

                            Image {
                                anchors.fill: parent
                                anchors.margins: 18
                                source: "qrc:/resources/img/headphones.png"
                                fillMode: Image.PreserveAspectFit
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 6

                            Label {
                                Layout.fillWidth: true
                                text: player.currentTitle.length ? player.currentTitle : "欢迎使用音乐魔盒"
                                font.pixelSize: 22
                                font.bold: true
                                elide: Text.ElideRight
                            }
                            Label {
                                Layout.fillWidth: true
                                text: player.currentArtist.length ? player.currentArtist : "拖拽音频文件到窗口开始播放"
                                color: textMuted
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Item { Layout.fillWidth: true }

                                Label {
                                    text: "模式：" + playbackModeLabel(player.playbackMode)
                                    color: textMuted
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 16
                    color: panel2
                    border.color: border

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: "歌词"
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Label {
                                text: player.lyrics.hasLyrics ? "" : "暂无歌词"
                                color: textMuted
                            }
                        }

                        ListView {
                            id: lyricsView
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            clip: true
                            model: player.lyrics
                            spacing: 8

                            delegate: Label {
                                width: lyricsView.width
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.Wrap
                                text: model.text
                                color: index === player.lyrics.currentIndex ? Material.foreground : textMuted
                                font.pixelSize: index === player.lyrics.currentIndex ? 20 : 14
                                font.bold: index === player.lyrics.currentIndex
                            }

                            ScrollBar.vertical: ScrollBar {}
                        }

                        Connections {
                            target: player.lyrics
                            function onCurrentIndexChanged() {
                                if (player.lyrics.currentIndex >= 0) {
                                    lyricsView.positionViewAtIndex(player.lyrics.currentIndex, ListView.Center)
                                }
                            }
                        }
                    }
                }
            }
        }
        }
    }

    Dialog {
        id: aboutDialog
        modal: true
        title: "关于"
        standardButtons: Dialog.Ok

        contentItem: ColumnLayout {
            spacing: 8
            Label { text: "音乐魔盒（Qt6）"; font.pixelSize: 18; font.bold: true }
            Label { text: "本地音乐播放器（QML + Qt Multimedia）"; color: textMuted }
            Label { text: "项目主页：github.com/angeiv/mymusicplayer"; color: "#93c5fd" }
        }
    }

    Dialog {
        id: loginDialog
        modal: true
        title: "登录"

        property string verifyCode: ""
        property string errorText: ""

        onOpened: {
            const a = Math.floor(Math.random() * 10)
            const b = Math.floor(Math.random() * 10)
            const c = Math.floor(Math.random() * 10)
            const d = Math.floor(Math.random() * 10)
            verifyCode = "" + a + b + c + d
            errorText = ""
            userField.text = ""
            passField.text = ""
            verifyField.text = ""
            userField.forceActiveFocus()
        }

        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel
            onAccepted: {
                if (verifyField.text.length === 0) {
                    loginDialog.errorText = "请输入验证码"
                    verifyField.forceActiveFocus()
                    return
                }
                if (verifyField.text !== loginDialog.verifyCode) {
                    loginDialog.errorText = "验证码错误，请重试"
                    verifyField.forceActiveFocus()
                    return
                }
                loginDialog.errorText = ""
                loginDialog.close()
            }
            onRejected: loginDialog.close()
        }

        contentItem: ColumnLayout {
            spacing: 10

            GridLayout {
                columns: 2
                rowSpacing: 8
                columnSpacing: 10

                Label { text: "用户名" }
                TextField { id: userField; Layout.preferredWidth: 280; placeholderText: "请输入用户名" }

                Label { text: "密码" }
                TextField { id: passField; Layout.preferredWidth: 280; placeholderText: "请输入密码"; echoMode: TextInput.Password }

                Label { text: "验证码" }
                RowLayout {
                    spacing: 10
                    TextField { id: verifyField; Layout.preferredWidth: 160; placeholderText: "4位数字"; inputMethodHints: Qt.ImhDigitsOnly }
                    Label { text: loginDialog.verifyCode; color: "#ef4444"; font.pixelSize: 20 }
                }
            }

            Label {
                visible: loginDialog.errorText.length > 0
                text: loginDialog.errorText
                color: "#ef4444"
            }
        }
    }
}
