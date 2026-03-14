import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import MyMusicPlayer 1.0

ApplicationWindow {
    id: window

    width: 980
    height: 620
    minimumWidth: 880
    minimumHeight: 560
    visible: true
    title: "音乐魔盒"

    property int selectedIndex: -1

    function formatTime(ms) {
        if (ms === undefined || ms === null) return "00:00"
        const totalSec = Math.max(0, Math.floor(ms / 1000))
        const min = Math.floor(totalSec / 60)
        const sec = totalSec % 60
        return String(min).padStart(2, "0") + ":" + String(sec).padStart(2, "0")
    }

    menuBar: MenuBar {
        Menu {
            title: "文件"
            MenuItem { text: "添加歌曲..."; onTriggered: app.pickAndAddFiles() }
            MenuItem { text: "清空列表"; enabled: playlistView.count > 0; onTriggered: player.clear() }
            MenuSeparator {}
            MenuItem { text: "退出"; onTriggered: Qt.quit() }
        }
        Menu {
            title: "账号"
            MenuItem { text: "登录..."; onTriggered: loginDialog.open() }
        }
        Menu {
            title: "播放模式"
            MenuItem {
                text: "顺序播放"
                checkable: true
                checked: player.playbackMode === PlayerEngine.Sequential
                onTriggered: player.playbackMode = PlayerEngine.Sequential
            }
            MenuItem {
                text: "列表循环"
                checkable: true
                checked: player.playbackMode === PlayerEngine.Loop
                onTriggered: player.playbackMode = PlayerEngine.Loop
            }
            MenuItem {
                text: "随机播放"
                checkable: true
                checked: player.playbackMode === PlayerEngine.Random
                onTriggered: player.playbackMode = PlayerEngine.Random
            }
            MenuItem {
                text: "单曲循环"
                checkable: true
                checked: player.playbackMode === PlayerEngine.CurrentItemInLoop
                onTriggered: player.playbackMode = PlayerEngine.CurrentItemInLoop
            }
        }
        Menu {
            title: "帮助"
            MenuItem { text: "关于"; onTriggered: aboutDialog.open() }
        }
    }

    Dialog {
        id: aboutDialog
        modal: true
        title: "关于"
        standardButtons: Dialog.Ok

        contentItem: ColumnLayout {
            spacing: 8
            Label { text: "音乐魔盒（Qt6）"; font.pixelSize: 18 }
            Label {
                text: "项目主页：github.com/angeiv/mymusicplayer"
                color: "#2b6cb0"
            }
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

                Label { text: "用户名"; }
                TextField { id: userField; Layout.preferredWidth: 260; placeholderText: "请输入用户名"; }

                Label { text: "密码"; }
                TextField { id: passField; Layout.preferredWidth: 260; placeholderText: "请输入密码"; echoMode: TextInput.Password }

                Label { text: "验证码"; }
                RowLayout {
                    spacing: 10
                    TextField { id: verifyField; Layout.preferredWidth: 160; placeholderText: "4位数字"; inputMethodHints: Qt.ImhDigitsOnly }
                    Label { text: loginDialog.verifyCode; color: "#ef4444"; font.pixelSize: 20; }
                }
            }

            Label {
                visible: loginDialog.errorText.length > 0
                text: loginDialog.errorText
                color: "#ef4444"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2
                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        font.pixelSize: 22
                        text: player.currentTitle.length ? player.currentTitle : "欢迎使用音乐魔盒"
                    }
                    Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        color: "#666"
                        text: player.currentArtist.length ? player.currentArtist : "V1.0"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 10
                    color: "#f6f7f9"
                    border.color: "#e5e7eb"

                    ListView {
                        id: lyricsView
                        anchors.fill: parent
                        anchors.margins: 12
                        clip: true
                        model: player.lyrics
                        spacing: 6

                        delegate: Label {
                            width: lyricsView.width
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                            text: model.text
                            color: index === player.lyrics.currentIndex ? "#111827" : "#6b7280"
                            font.pixelSize: index === player.lyrics.currentIndex ? 18 : 14
                            font.bold: index === player.lyrics.currentIndex
                        }
                    }

                    Label {
                        anchors.centerIn: parent
                        visible: !player.lyrics.hasLyrics
                        text: "暂无歌词"
                        color: "#9ca3af"
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

            Rectangle {
                Layout.preferredWidth: 330
                Layout.fillHeight: true
                radius: 10
                color: "#ffffff"
                border.color: "#e5e7eb"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 8

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8
                        Label { text: "播放列表"; font.pixelSize: 16; Layout.fillWidth: true }
                        Button { text: "添加"; onClicked: app.pickAndAddFiles() }
                        Button {
                            text: "删除"
                            enabled: selectedIndex >= 0
                            onClicked: {
                                player.removeAt(selectedIndex)
                                selectedIndex = -1
                            }
                        }
                    }

                    ListView {
                        id: playlistView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: player.playlist

                        delegate: ItemDelegate {
                            width: playlistView.width
                            text: title + "  -  " + artist
                            highlighted: index === selectedIndex
                            onClicked: {
                                selectedIndex = index
                                player.currentIndex = index
                                player.play()
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 6

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Label { text: formatTime(player.position) }
                Slider {
                    id: progress
                    Layout.fillWidth: true
                    from: 0
                    to: Math.max(1, player.duration)
                    value: player.position
                    onMoved: player.position = value
                }
                Label { text: formatTime(player.duration) }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button { text: "上一曲"; enabled: playlistView.count > 0; onClicked: player.previous() }
                Button {
                    text: player.playing ? "暂停" : "播放"
                    enabled: playlistView.count > 0
                    onClicked: player.togglePlay()
                }
                Button { text: "下一曲"; enabled: playlistView.count > 0; onClicked: player.next() }

                Item { Layout.fillWidth: true }

                Label { text: "音量" }
                Slider {
                    Layout.preferredWidth: 140
                    from: 0
                    to: 100
                    value: player.volume
                    onMoved: player.volume = value
                }
                Button {
                    text: player.muted ? "取消静音" : "静音"
                    onClicked: player.muted = !player.muted
                }
            }
        }
    }
}
