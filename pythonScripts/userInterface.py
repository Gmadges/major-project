from maya import cmds
from maya import mel
from maya import OpenMayaUI as omui 

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

class CreateUI(QWidget):
    def __init__(self, *args, **kwargs):
        super(CreateUI, self).__init__(*args, **kwargs)
        self.setParent(mayaMainWindow)
        self.setWindowFlags(Qt.Window)
        self.address = 'localhost'
        self.port = 8080
        self.meshList = []
        self.currentMeshIndex = 0
        self.initUI()
        
    def initUI(self):
        # controls
        self.send_btn = QPushButton('Send', self)
        self.send_btn.clicked.connect(self.send)
        self.update_btn = QPushButton('Update', self)
        self.update_btn.clicked.connect(self.update)

        self.address_line = QLineEdit()
        self.address_line.setText(self.address)
        self.address_line.textChanged[str].connect(self.addressChanged)

        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65536)
        self.port_spin.setValue(self.port)
        self.port_spin.valueChanged[int].connect(self.portChanged)

        self.mesh_combo = QComboBox();
        self.mesh_combo.currentIndexChanged[int].connect(self.meshChanged)
        # test
        self.updateMeshList()

        # layout code
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(2, 2, 2, 2)
        settings_layout.addRow(QLabel('Network address'), self.address_line)
        settings_layout.addRow(QLabel('Port number'), self.port_spin)
        settings_layout.addRow(QLabel('Mesh select'), self.mesh_combo)

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.send_btn)
        button_layout.addWidget(self.update_btn)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addLayout(settings_layout)
        main_layout.addLayout(button_layout)

        self.setLayout(main_layout)
    
    def addressChanged(self, text):
        self.address = text

    def portChanged(self, port):
        self.port = port

    def meshChanged(self, index):
        self.currentMeshIndex = index

    def updateMeshList(self):
        self.requestInfo()
        self.mesh_combo.clear()
        for i in self.meshList : 
            self.mesh_combo.addItem(i[0])
        self.meshChanged(0)

    def requestInfo(self):
        sendCmd = 'getInfo -a ' + self.address + ' -p ' + str(self.port)
        meshResult = mel.eval(sendCmd)
        self.meshList = []
        for i in range(0, len(meshResult), 2):
            meshVal = [meshResult[i], meshResult[i+1]]
            self.meshList.append(meshVal)

        
    def send(self):
        sendCmd = 'ScanSend -a ' + self.address + ' -p ' + str(self.port)
        mel.eval(sendCmd)
        
    def update(self):
        updateCmd = 'ReceiveUpdate -a ' + self.address + ' -p ' + str(self.port) + ' -id ' + self.meshList[self.currentMeshIndex][1]
        print updateCmd
        #mel.eval(updateCmd)
            
def main():
    ui = CreateUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()
