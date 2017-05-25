from maya import cmds
from maya import mel
from maya import OpenMayaUI as omui
from threading import Timer
import urllib2
import json
import socket

try:
    from PySide2.QtCore import * 
    from PySide2.QtGui import * 
    from PySide2.QtWidgets import *
    from PySide2.QtUiTools import *
    from shiboken2 import wrapInstance 
except ImportError:
    from PySide.QtCore import * 
    from PySide.QtGui import * 
    from PySide.QtUiTools import *
    from shiboken import wrapInstance 

mayaMainWindowPtr = omui.MQtUtil.mainWindow() 
mayaMainWindow = wrapInstance(long(mayaMainWindowPtr), QWidget)

class ServerMessenger(object):
    def __init__(self):
        self.serverAddress = ''

    def setServer(self, address, port):
        self.serverAddress = "http://"+ address + ":" + str(port)

    def requestAllMeshes(self):
        if not self.serverAddress : return
        try:
            response = urllib2.urlopen( self.serverAddress + "/")
            data = json.load(response)
            response.close()
            return data
        except urllib2.URLError, e:
            data = {}
            data["status"] = 404
            return data

    def deleteMesh(self, meshId):
        if not self.serverAddress : return
        if not meshId : return
        response = urllib2.urlopen( self.serverAddress + "/" + meshId + "/delete")
        response.close()

    def heartbeat(self):
        try:
            response = urllib2.urlopen( self.serverAddress + "/heartbeat")
            data = json.load(response)
            response.close()
            if data['status'] is 200 :
                return True
            else:
                return False
        except urllib2.URLError, e:
            return False

class serverConnectWidget(QFrame):

    connected = Signal(bool)

    def __init__(self, messenger):
        super(serverConnectWidget, self).__init__()
        self.messenger = messenger
        self.initUI()
        self.connectionMade = False
        # perform hertbeat func every 10 seconds
        self.heartbeatWait = 5.0
        self.heartbeatTimer = Timer(self.heartbeatWait, self.heartbeatFunc)
        self.setFrameStyle(QFrame.StyledPanel)
        # This code will set up the current server settings if it exists
        if self.pluginLoaded() is True:
            settings = mel.eval('SetServer -q')
            if settings != None :
                self.port_spin.setValue(int(settings[0]))
                self.address_line.setText(settings[1])
                if settings[2] != socket.gethostbyname(socket.gethostname()) :
                    self.user_id_line.setText(settings[2])
                self.messenger.setServer(settings[1], int(settings[0]) + 1)
                self.setConnectedLabel(True)
                self.startHeartbeat()
                self.connectionMade = True
                self.connected.emit(self.connectionMade)
            
    def initUI(self):
        self.connect_btn = QPushButton('Connect', self)
        self.connect_btn.clicked.connect(self.connectToServer)
        self.connection_label = QLabel('disconnected')
        self.connection_label.setStyleSheet('color: red')

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.connect_btn)
        button_layout.addWidget(self.connection_label)

        self.user_id_line = QLineEdit()
        self.address_line = QLineEdit()
        self.address_line.setText('localhost')
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65536)
        self.port_spin.setValue(8080)
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(2, 2, 2, 2)
        settings_layout.addRow(QLabel('User ID (optional)'), self.user_id_line)
        settings_layout.addRow(QLabel('Network address'), self.address_line)
        settings_layout.addRow(QLabel('Port number'), self.port_spin)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addLayout(settings_layout)
        main_layout.addLayout(button_layout)
        self.setLayout(main_layout)

    def connectToServer(self):
        self.stopHeartbeat()
        if self.pluginLoaded() is True:
            address = self.address_line.text()
            port = self.port_spin.value()
            userID = self.user_id_line.text()
            # add one to the port
            self.messenger.setServer(address, port + 1)
            if self.messenger.heartbeat() is True :
                self.setConnectedLabel(True)
                self.setServerCmd(address, port, userID)
                self.startHeartbeat()
                self.connectionMade = True
                self.connected.emit(self.connectionMade)
            else:
                self.setConnectedLabel(False)
                self.connectionMade = False
                self.connected.emit(self.connectionMade)
                cmds.confirmDialog( title='Error', message='Cannot connect to server.')
        else :
            cmds.confirmDialog( title='Error', message='Plugin is not loaded.')    

    def startHeartbeat(self):
        def interval_wrapper():
            self.startHeartbeat() 
            self.heartbeatFunc()
        self.heartbeatTimer = Timer(self.heartbeatWait, interval_wrapper)
        self.heartbeatTimer.start()

    def stopHeartbeat(self):
        self.heartbeatTimer.cancel()

    def heartbeatFunc(self):
        self.setConnectedLabel(self.messenger.heartbeat())

    def pluginLoaded(self):
         return cmds.pluginInfo('PluginDebugTest', query=True ,loaded=True)

    def setConnectedLabel(self, isConnected):
        if isConnected is True:
            self.connection_label.setText('connected')
            self.connection_label.setStyleSheet('color: green')
        else :
            self.connection_label.setText('disconnected')
            self.connection_label.setStyleSheet('color: red')

    def setServerCmd(self, address, port, userID):
        if not userID:
            userID = socket.gethostbyname(socket.gethostname())
        cmd = 'SetServer -a "' + address + '" -p ' + str(port) + ' -uid "' + userID + '"'
        mel.eval(cmd)

    def isServerConnected(self):
        return self.connectionMade

class meshSelectionWidget(QFrame):

    meshRequested = Signal(str)

    def __init__(self, messenger):
        super(meshSelectionWidget, self).__init__()
        self.messenger = messenger
        self.initUI()
        self.setFrameStyle(QFrame.StyledPanel)

    def initUI(self):
        self.list = QListWidget()
        self.list.setSelectionMode(QAbstractItemView.ExtendedSelection)

        self.getMesh_btn = QPushButton('Get', self)
        self.delMesh_btn = QPushButton('Delete', self)
        self.updateMeshes_btn = QPushButton('Request All Meshes', self)

        self.getMesh_btn.clicked.connect(self.getMesh)
        self.delMesh_btn.clicked.connect(self.delMesh)
        self.updateMeshes_btn.clicked.connect(self.requestAllMesh)

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.getMesh_btn)
        button_layout.addWidget(self.delMesh_btn)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addWidget(self.updateMeshes_btn)
        main_layout.addWidget(self.list)
        main_layout.addLayout(button_layout)
        self.setLayout(main_layout)

    def requestAllMesh(self):
        request = self.messenger.requestAllMeshes()
        self.list.clear()
        for i in range(len(request['meshNames'])):
            newItem = QListWidgetItem()
            newItem.setText(request['meshNames'][i])
            newItem.setData(Qt.ToolTipRole, request['meshIds'][i])
            self.list.addItem(newItem)

    def getMesh(self):
        items = self.list.selectedItems()
        if len(items) == 1 :
            self.requestMeshCmd(items[0].toolTip())
            self.meshRequested.emit(items[0].text())
        else :
            if len(items) > 1:
                cmds.confirmDialog( title='Error', message='Only one mesh can be requested.')
            else :
                cmds.confirmDialog( title='Error', message='No mesh selected.')

    def requestMeshCmd(self, meshId):
        clearCmd = 'ClearCurrentMesh'
        reqCmd = 'RequestMesh -id "' + meshId + '"'
        mel.eval(clearCmd)
        mel.eval(reqCmd)

    def delMesh(self):
        items = self.list.selectedItems()
        for i in range(len(items)) :
            self.messenger.deleteMesh(items[i].toolTip())
        self.requestAllMesh()

class currentMeshWidget(QFrame):

    meshRegistered = Signal()

    def __init__(self, messenger):
        super(currentMeshWidget, self).__init__()
        self.messenger = messenger
        self.initUI()
        self.setFrameStyle(QFrame.StyledPanel)

    def initUI(self):
        self.currentMesh_label = QLabel('Current Mesh:', self)
        
        self.send_btn = QPushButton('send', self)
        self.update_btn = QPushButton('update', self)

        self.reg_btn = QPushButton('register selected', self)
        self.clear_btn = QPushButton('clear', self)

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.send_btn)
        button_layout.addWidget(self.update_btn)

        self.send_btn.clicked.connect(self.forceSendCmd)
        self.update_btn.clicked.connect(self.forceUpdateCmd)

        button1_layout = QHBoxLayout()
        button1_layout.setContentsMargins(2, 2, 2, 2)
        button1_layout.addWidget(self.reg_btn)
        button1_layout.addWidget(self.clear_btn)

        self.reg_btn.clicked.connect(self.registerMeshCmd)
        self.clear_btn.clicked.connect(self.clearCmd)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addWidget(self.currentMesh_label)
        main_layout.addLayout(button_layout)
        main_layout.addLayout(button1_layout)
        self.setLayout(main_layout)

    def registerMeshCmd(self):
        cmd = "RegisterMesh"
        mel.eval(cmd)

        self.updateCurrentMeshLabel(self.getSelectedMesh())
        self.meshRegistered.emit()
        
    def clearCmd(self):
        cmd = 'ClearCurrentMesh'
        mel.eval(cmd)
        self.updateCurrentMeshLabel('')

    def forceSendCmd(self):
        cmd = "SendUpdates"
        mel.eval(cmd)
            
    def forceUpdateCmd(self):
        cmd = 'RequestUpdate'
        mel.eval(cmd)

    def updateCurrentMeshLabel(self, meshName):
        self.currentMesh_label.setText('Current Mesh: ' + meshName)

    def getSelectedMesh(self):
        meshes = cmds.ls(sl=True, type='transform')
        if not meshes: return ''
        return meshes[0]
    

class settingsWidget(QFrame):

    def __init__(self):
        super(settingsWidget, self).__init__()
        self.initUI()
        self.setFrameStyle(QFrame.StyledPanel)

    def initUI(self):
        self.settings_label = QLabel('Settings', self)

        self.full_mesh_check = QCheckBox()
        self.full_mesh_check.stateChanged.connect(self.enableFullMeshSettings)
        self.update_interval_spin = QDoubleSpinBox()
        self.update_interval_spin.setDecimals(1)
        self.update_interval_spin.setSingleStep(0.1)
        self.update_interval_spin.setRange(0.5, 120.0)
        self.update_interval_spin.setValue(2.0)
        self.update_interval_spin.valueChanged.connect(self.changeUpdateInterval)
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(2, 2, 2, 2)
        settings_layout.addRow(QLabel('Enable full mesh updates'), self.full_mesh_check)
        settings_layout.addRow(QLabel('Set Update Interval'), self.update_interval_spin)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addWidget(self.settings_label)
        main_layout.addLayout(settings_layout)
        self.setLayout(main_layout)

    def enableFullMeshSettings(self):
        cmd = "Settings -fm "
        if self.full_mesh_check.isChecked():
            cmd += "1"
        else:
            cmd += "0"
        mel.eval(cmd)

    def changeUpdateInterval(self, value):
        cmd = "Settings -ui "
        cmd += str(value)
        self.update_interval_spin.setValue(value)
        mel.eval(cmd)


class CreateUI(QWidget):
    def __init__(self, *args, **kwargs):
        super(CreateUI, self).__init__(*args, **kwargs)
        self.setParent(mayaMainWindow)
        self.setWindowFlags(Qt.Window)
        self.messenger = ServerMessenger()
        self.initUI()

    def closeEvent(self, event):
        self.connectionWid.stopHeartbeat()

    def initUI(self):
        self.connectionWid = serverConnectWidget(self.messenger)
        self.meshSelectWid = meshSelectionWidget(self.messenger)
        self.currentMeshWid = currentMeshWidget(self.messenger)
        self.settingsWid = settingsWidget()

        # connect items
        self.connectionWid.connected.connect(self.meshSelectWid.requestAllMesh)
        self.connectionWid.connected.connect(self.enableWidgets)
        self.currentMeshWid.meshRegistered.connect(self.meshSelectWid.requestAllMesh)
        self.meshSelectWid.meshRequested.connect(self.currentMeshWid.updateCurrentMeshLabel)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addWidget(self.connectionWid)
        main_layout.addWidget(self.meshSelectWid)
        main_layout.addWidget(self.currentMeshWid)
        main_layout.addWidget(self.settingsWid)
        self.setLayout(main_layout)

        if self.connectionWid.isServerConnected() is True:
            self.setWidgets(True)
            self.meshSelectWid.requestAllMesh()
            # set current mesh if there is one
        else:
            self.setWidgets(False)

    def setWidgets(self, enable):
        self.meshSelectWid.setEnabled(enable)
        self.currentMeshWid.setEnabled(enable)
        self.settingsWid.setEnabled(enable)

    def enableWidgets(self):
        self.meshSelectWid.setEnabled(True)
        self.currentMeshWid.setEnabled(True)
        self.settingsWid.setEnabled(True)
            
def main():
    # todo
    # add plugin loading stuff here
    ui = CreateUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()
