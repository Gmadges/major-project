class CollabScene(object):

    def __init__(self):
      self.updateID = 0
      self.scanID = 0
      self.requestUpdate = False
      
    def lookForChanges(seconds):
      def isItTime():
        now = time.time()
        if now - isItTime.then > seconds:
          mel.eval("ScanSend")
          self.requestUpdate = True
          isItTime.then = now
        
      isItTime.then = time.time()
      self.scanID = cmds.scriptJob(event=("idle", isItTime))
        
    def updateScene(seconds):
      def isItTime():
        now = time.time()
        if now - isItTime.then > seconds:
          if self.requestUpdate:
            print 'ask for update'
            print 'update'
            self.requestUpdate = False
          isItTime.then = now
        
      isItTime.then = time.time()
      self.updateID = cmds.scriptJob(event=("idle", isItTime))