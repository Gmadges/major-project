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

import os.path

mayaMainWindowPtr = omui.MQtUtil.mainWindow() 
mayaMainWindow = wrapInstance(long(mayaMainWindowPtr), QWidget) 

class CreatePolygonUI(QWidget):
    def __init__(self, *args, **kwargs):
        super(CreatePolygonUI, self).__init__(*args, **kwargs)
        self.setParent(mayaMainWindow)
        self.setWindowFlags(Qt.Window)
        self.initUI()
        self.sendCmd = 'ScanSend'
        self.updateCmd = 'ReceiveUpdate'
        
    def initUI(self):
        loader = QUiLoader()        
        #hard code for now
        file = QFile("C:\Projects\major-project.git\pythonScripts\commandPanel.ui")        
        file.open(QFile.ReadOnly)        
        self.ui = loader.load(file, parentWidget=self)        
        file.close()
        
        
    def text_dataChange(self):
        self.cmd = 'poly' + text + '()'
        
    def button_sendFunction(self):
        mel.eval( self.cmd )
            
def main():
    ui = CreatePolygonUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()
