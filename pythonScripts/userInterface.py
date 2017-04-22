from maya import cmds
from maya import mel
from maya import OpenMayaUI as omui
import urllib2
import json

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
        response = urllib2.urlopen( self.serverAddress + "/")
        data = json.load(response)
        response.close()
        return data

class serverConnectWidget(QWidget):

    def __init__(self, messenger):
        super(serverConnectWidget, self).__init__()
        self.messenger = messenger
        self.initUI()

    def initUI(self):
        self.connect_btn = QPushButton('Register', self)
        self.connection_label = QLabel('disconnected')
        self.connection_label.setStyleSheet('color: red')

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.connect_btn)
        self.connect_btn.clicked.connect(self.connectToServer)
        button_layout.addWidget(self.connection_label)

        self.address_line = QLineEdit()
        self.address_line.setText('localhost')
        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65536)
        self.port_spin.setValue(8081)
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(2, 2, 2, 2)
        settings_layout.addRow(QLabel('Network address'), self.address_line)
        settings_layout.addRow(QLabel('Port number'), self.port_spin)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addLayout(settings_layout)
        main_layout.addLayout(button_layout)
        self.setLayout(main_layout)

    def connectToServer(self):
        address = self.address_line.text()
        port = self.port_spin.value()
        self.messenger.setServer(address, port)
        request = self.messenger.requestAllMeshes()
        print request
        if request['status'] is 200 :
            self.connection_label.setText('connected')
            self.connection_label.setStyleSheet('color: green')
        else:
            self.connection_label.setText('disconnected')
            self.connection_label.setStyleSheet('color: red')


class CreateUI(QWidget):
    def __init__(self, *args, **kwargs):
        super(CreateUI, self).__init__(*args, **kwargs)
        self.setParent(mayaMainWindow)
        self.setWindowFlags(Qt.Window)
        self.messenger = ServerMessenger()
        self.initUI()

    def initUI(self):
        self.connectionWid = serverConnectWidget(self.messenger)
        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addWidget(self.connectionWid)
        self.setLayout(main_layout)
            
def main():
    ui = CreateUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()
