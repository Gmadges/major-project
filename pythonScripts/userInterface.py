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
        self.initUI()
        self.sendCmd = 'ScanSend'
        self.updateCmd = 'ReceiveUpdate'
        
    def initUI(self):
        # controls
        self.send_btn = QPushButton('Send', self)
        self.update_btn = QPushButton('Update', self)

        self.address_line = QLineEdit()
        self.address_line.setText('localhost')

        self.port_spin = QSpinBox()
        self.port_spin.setRange(1, 65536)
        self.port_spin.setValue(8080)

        # layout code
        settings_layout = QFormLayout()
        settings_layout.setContentsMargins(2, 2, 2, 2)
        settings_layout.addRow(QLabel('Network address'), self.address_line)
        settings_layout.addRow(QLabel('Port number'), self.port_spin)

        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(2, 2, 2, 2)
        button_layout.addWidget(self.send_btn)
        button_layout.addWidget(self.update_btn)

        main_layout = QVBoxLayout()
        main_layout.setContentsMargins(2, 2, 2, 2)
        main_layout.addLayout(settings_layout)
        main_layout.addLayout(button_layout)

        self.setLayout(main_layout)


    def text_dataChange(self):
        self.cmd = 'poly' + text + '()'
        
    def button_sendFunction(self):
        mel.eval( self.cmd )
            
def main():
    ui = CreateUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()
