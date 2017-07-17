# coding: utf-8
import sys
import socket
import threading
import time
import ctypes
import pickle
import os
from PySide import QtCore, QtGui

CDU_COLUMNS = 24
CDU_ROWS = 14
SETTING_FILENAME = '747CDUClient.setting'
SETTING_PATH = os.path.os.path.expanduser('~') + '/' + SETTING_FILENAME

 
class SettingWindow( QtGui.QDialog ):

	def setupUi( self ):
		self.resize(418, 159)
		
		self.verticalLayout = QtGui.QVBoxLayout(self)
		self.gridLayout = QtGui.QGridLayout()
		self.gridLayout.setObjectName("gridLayout")
		
		self.label = QtGui.QLabel(self)
		self.label.setObjectName("label")
		self.label.setText( 'IP' )
		self.gridLayout.addWidget(self.label, 0, 0, 1, 1)
		
		self.label_2 = QtGui.QLabel(self)
		self.label_2.setObjectName("label_2")
		self.label_2.setText( 'Port' )
		self.gridLayout.addWidget(self.label_2, 1, 0, 1, 1)
		
		self.lineEdit_IP = QtGui.QLineEdit(self)
		self.lineEdit_IP.setObjectName("lineEdit_IP")
		self.lineEdit_IP.setText( self.parentWindow.setting.ip )
		self.gridLayout.addWidget(self.lineEdit_IP, 0, 1, 1, 1)
		
		self.lineEdit_Port = QtGui.QLineEdit(self)
		self.lineEdit_Port.setObjectName("lineEdit_Port")
		self.lineEdit_Port.setValidator( QtGui.QIntValidator( 0, 65535, self ) )
		self.lineEdit_Port.setText( str( self.parentWindow.setting.port ) )
		self.gridLayout.addWidget(self.lineEdit_Port, 1, 1, 1, 1)
		
		self.doubleSpinBox_FontScale = QtGui.QDoubleSpinBox(self)
		self.doubleSpinBox_FontScale.setObjectName("doubleSpinBox_FontScale")
		self.doubleSpinBox_FontScale.setValue( self.parentWindow.setting.fontScale )
		self.doubleSpinBox_FontScale.setSingleStep( 0.05 )
		self.gridLayout.addWidget(self.doubleSpinBox_FontScale, 2, 1, 1, 1)
		
		self.label_3 = QtGui.QLabel(self)
		self.label_3.setObjectName("label_3")
		self.label_3.setText( 'Font Scale' )
		self.gridLayout.addWidget(self.label_3, 2, 0, 1, 1)

		self.label_4 = QtGui.QLabel(self)
		self.label_4.setObjectName("label_4")
		self.gridLayout.addWidget(self.label_4, 4, 0, 1, 1)
		self.comboBox = QtGui.QComboBox(self)
		self.comboBox.setObjectName("comboBox")
		self.comboBox.addItem("CDU0")
		self.comboBox.addItem("CDU1")
		self.comboBox.addItem("CDU2")
		self.comboBox.setCurrentIndex ( 0 )
		if( self.parentWindow.setting.targetCDU == 'CDU1' ):
			self.comboBox.setCurrentIndex ( 1 )
		elif( self.parentWindow.setting.targetCDU == 'CDU2' ):
			self.comboBox.setCurrentIndex ( 2 )
		self.gridLayout.addWidget(self.comboBox, 4, 1, 1, 1)
		
		self.doubleSpinBox_UpdateInterval = QtGui.QDoubleSpinBox(self)
		self.doubleSpinBox_UpdateInterval.setObjectName("doubleSpinBox_UpdateInterval")
		self.doubleSpinBox_UpdateInterval.setValue( self.parentWindow.setting.updateInterval )
		self.doubleSpinBox_UpdateInterval.setSingleStep( 1.0 )
		self.doubleSpinBox_UpdateInterval.setMinimum( 10.0 )
		self.doubleSpinBox_UpdateInterval.setMaximum( 10000.0 )
		self.gridLayout.addWidget(self.doubleSpinBox_UpdateInterval, 3, 1, 1, 1)
		
		self.label_4 = QtGui.QLabel(self)
		self.label_4.setObjectName("label_4")
		self.label_4.setText( 'Update Interval(ms)' )
		self.gridLayout.addWidget(self.label_4, 3, 0, 1, 1)
		
		self.verticalLayout.addLayout(self.gridLayout)
		
		self.horizontalLayout = QtGui.QHBoxLayout()
		self.horizontalLayout.setObjectName("horizontalLayout")
		self.pushButton_Connect = QtGui.QPushButton(self)
		self.pushButton_Connect.setObjectName("pushButton_Connect")
		self.pushButton_Connect.setText( 'Connect' )
		self.pushButton_Connect.clicked.connect( self.on_pushButton_Connect_clicked )
		self.horizontalLayout.addWidget(self.pushButton_Connect)
		
		self.pushButton_Apply = QtGui.QPushButton(self)
		self.pushButton_Apply.setObjectName("pushButton_Apply")
		self.pushButton_Apply.setText( 'Apply' )
		self.pushButton_Apply.clicked.connect( self.on_pushButton_Apply_clicked )
		self.horizontalLayout.addWidget(self.pushButton_Apply)
		
		self.pushButton_Close = QtGui.QPushButton(self)
		self.pushButton_Close.setObjectName("pushButton_Close")
		self.pushButton_Close.setText( 'Close' )
		self.pushButton_Close.clicked.connect( self.on_pushButton_Close_clicked )
		
		self.horizontalLayout.addWidget(self.pushButton_Close)
		self.verticalLayout.addLayout(self.horizontalLayout)

		QtCore.QMetaObject.connectSlotsByName( self )

	def on_pushButton_Apply_clicked( self ):
		self.parentWindow.setting.ip = self.lineEdit_IP.text()
		self.parentWindow.setting.port = int( self.lineEdit_Port.text() )
		self.parentWindow.setting.fontScale = self.doubleSpinBox_FontScale.value()
		self.parentWindow.setting.updateInterval = self.doubleSpinBox_UpdateInterval.value()
		self.parentWindow.setting.targetCDU = self.comboBox.currentText()

		self.parentWindow.update()
			
	def on_pushButton_Connect_clicked( self ):
		self.parentWindow.disconnect()
		self.on_pushButton_Apply_clicked()
		self.parentWindow.connect()
		
	def on_pushButton_Close_clicked( self ):
		self.close()
		
	def __init__( self, parentWindow ):
		super( SettingWindow, self ).__init__( parentWindow )
		self.parentWindow = parentWindow
		self.setupUi()
		self.setWindowTitle( '747 CDU Setting' )


class Setting( object ):
	def __init__( self ):
		super( Setting, self ).__init__()

		self.ip = '127.0.0.1'
		self.targetCDU = 'CDU0'
		self.fontScale = 1.0
		self.port = 47474
		self.updateInterval = 20.0

class PollingClient( threading.Thread ):
	def __init__( self, parentWindow ):
		super( PollingClient, self ).__init__()
		self.halt = False
		
		self.parentWindow = parentWindow
		self.socketLock = threading.Lock()

		self.isConneted = False

	def __del__( self ):
		if( self.sock ):
			self.sock.close()

	def run( self ):
		self.sock = socket.socket( socket.AF_INET, socket.SOCK_STREAM )

		while( True ):
			if( self.halt ):
				return
			
			try:
				self.sock.connect( ( self.parentWindow.setting.ip, self.parentWindow.setting.port ) )
				break
			except:
				continue

		self.isConneted = True

		cnt = 0
		needRedraw = False
		while( not self.halt ):

			self.sock.send( 'Req:' + self.parentWindow.setting.targetCDU )
			ret = self.sock.recv( CDU_COLUMNS * CDU_ROWS * 4 )
			if( ret != self.parentWindow.screenData ):
				self.parentWindow.screenData = ret
				needRedraw = True


			self.sock.send( 'Req:ANNU' )
			ret = self.sock.recv( 1 )
			v = ctypes.c_char( ret )
			rawVal = ctypes.cast( (ctypes.c_char * 1 )(v), ctypes.POINTER(ctypes.c_byte) ).contents.value
			if( rawVal & 0b00000001 ):
				self.parentWindow.annuEXEC = True
			else:
				self.parentWindow.annuEXEC = False
				
			if( rawVal & 0b00000010 ):
				self.parentWindow.annuDSPY = True
			else:
				self.parentWindow.annuDSPY = False
				
			if( rawVal & 0b00000100 ):
				self.parentWindow.annuFAIL = True
			else:
				self.parentWindow.annuFAIL = False
				
			if( rawVal & 0b00001000 ):
				self.parentWindow.annuMSG = True
			else:
				self.parentWindow.annuMSG = False

			if( needRedraw ):
				self.parentWindow.update()
				needRedraw = False

			time.sleep( self.parentWindow.setting.updateInterval / 1000.0 )

			
class CDU747( QtGui.QLabel ):
	def __init__( self, p, setting ):
		super( CDU747, self ).__init__( p, 0 )
		self.setting = setting
		
		self.fontSize = 16
		
		self.screenData = None
		self.annuEXEC = False
		self.annuDSPY = False
		self.annuMSG = False
		self.annuFAIL = False

		self.sock = None

		self.pollingClient = None
		
		self.lastPressKeyIndex = -1

		self.clickSpot = (
			#0 - 5
			#EVENT_CDU_L_L1...
			(7,85),
			(7,122),
			(7,160),
			(7,196),
			(7,235),
			(7,274),

			#6 - 11
			#EVENT_CDU_L_R1...
			(444,85),
			(444,122),
			(444,160),
			(444,196),
			(444,235),
			(444,274),

			#12 - 26
			#EVENT_CDU_L_INIT_REF...
			(64,354),
			(125,354),
			(186,354),
			(246,354),
			(307,354),
			(64,400),
			(125,400),
			(185,400),
			(246,400),
			(307,400),
			(376,400),
			(63,446),
			(125,446),
			(64,493),
			(125,493),

			#27 - 37
			#EVENT_CDU_L_1...
			(60,540),
			(110,540),
			(155,540),
			(60,586),
			(110,586),
			(155,586),
			(60,635),
			(110,635),
			(155,635),
			(60,680),
			(110,680),
			(155,680),

			#38 - 
			#EVENT_CDU_L_A...
			(203,451),
			(249,451),
			(296,451),
			(343,451),
			(390,451),
			
			(203,495),
			(249,495),
			(296,495),
			(343,495),
			(390,495),

			(202,540),
			(249,540),
			(296,540),
			(343,540),
			(390,540),

			(203,588),
			(249,588),
			(296,588),
			(343,588),
			(390,588),

			(202,636),
			(249,636),
			(296,636),
			(343,636),
			(390,636),

			(202,681),
			(249,681),
			(296,681),
			(343,681),
			(390,681),
		)

		self.displayUpperLeft = ( 0.15, 0.09 )
		self.displayLowerRight = ( 0.85, 0.45 )
		
		self.clickPosBitmap = None

		pix = QtGui.QPixmap( '747CDU.png' )
		self.setPixmap( pix )
		self.setScaledContents( True )
		
		self.setWindowTitle( '747 CDU' )

	def contextMenuEvent( self, ev ):
		menu = QtGui.QMenu( self )
		menu.addAction( QtGui.QAction( 'Connect', self, triggered = self.connect ) )
		menu.addAction( QtGui.QAction( 'Disonnect', self, triggered = self.disconnect ) )
		menu.addAction( QtGui.QAction( 'Setting', self, triggered = self.showSetting ) )
		menu.exec_( ev.globalPos() )

	def __del__( self ):
		if( self.pollingClient ):
			self.pollingClient.halt = True

	def closeEvent(self, event):
		if( self.sock ):
			self.sock.close()

		if( self.pollingClient ):
			self.pollingClient.halt = True

		with open( SETTING_PATH, 'wb' ) as fhdl:
			pickle.dump( self.setting, fhdl )
		
	def paintKeyPressFeedback( self, ev ):
		geom = self.geometry()
		
		painter = QtGui.QPainter()
		painter.begin( self )
		xratio = float( geom.width() ) / 480.0
		yratio = float( geom.height() ) / 740.0
		painter.fillRect( QtCore.QRectF( self.clickSpot[self.lastPressKeyIndex][0] * xratio, self.clickSpot[self.lastPressKeyIndex][1] * yratio, 30.0 * xratio, 30 * yratio ), QtGui.QColor( 250, 200, 200, 180 ) )
		painter.end()

	def connect( self ):
		if( not self.pollingClient ):
			self.pollingClient = PollingClient( self )
			self.pollingClient.start()

		if( not self.sock ):
			self.sock = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
			self.sock.connect( ( self.setting.ip, self.setting.port ) )

	def disconnect( self ):
		if( self.pollingClient ):
			self.pollingClient.halt = True
			self.pollingClient = None
			
		if( self.sock ):
			self.sock.close()
			self.sock = None
	def showSetting( self ):
		subWindow = SettingWindow( self )
		subWindow.show()

	def paintDisplay( self, ev ):
		geom = self.geometry()
		
		painter = QtGui.QPainter()
		painter.begin( self )

		fontSizeNormal = ( float( geom.width() ) / 480.0 ) * 16.0 * self.setting.fontScale
		fontSizeSmall = ( float( geom.width() ) / 480.0 ) * 12.0 * self.setting.fontScale

		colorNormal = QtGui.QColor( 100, 255, 100 )
		colorDark = QtGui.QColor( 50, 155, 50 )
		colorORG = QtGui.QColor( 220, 220, 100 )
		painter.setPen( colorNormal )

		perX = ( ( self.displayLowerRight[0] - self.displayUpperLeft[0] ) * float( geom.width() ) ) / float( CDU_COLUMNS )
		perY = ( ( self.displayLowerRight[1] - self.displayUpperLeft[1] ) * float( geom.height() ) ) / float( CDU_ROWS )

		coord = QtCore.QPointF( 0.0, 0.0 )

		if( self.screenData ):
			for y in xrange( CDU_ROWS ):
				for x in xrange( CDU_COLUMNS ):
					idx = y * CDU_COLUMNS * 2 + x * 2
					if( idx < len( self.screenData ) ):
						s = self.screenData[ idx ]
						v = ctypes.c_char( self.screenData[ idx ] )
						rawVal = ctypes.cast( (ctypes.c_char * 1 )(v), ctypes.POINTER(ctypes.c_byte) ).contents.value
						if( rawVal == -22 ):
							s = u'❏'
						if( rawVal == -95 ):
							s = u'⇠'
						if( rawVal == -94 ):
							s = u'⇢'

						v = ctypes.c_char( self.screenData[ idx + 1 ] )
						flag = ctypes.cast( (ctypes.c_char * 1 )(v), ctypes.POINTER(ctypes.c_byte) ).contents.value
					
						fontSize = fontSizeNormal
						if( flag & 0x01 ):
							fontSize = fontSizeSmall
						col = colorNormal
						if( flag & 0x04 ):
							col = colorDark

						painter.setPen( col )
						painter.setFont( QtGui.QFont( 'Consolas', fontSize, QtGui.QFont.Bold ) )
						
						xf = float(x) * perX + geom.width() * self.displayUpperLeft[0]
						yf = float(y) * perY + geom.height() * self.displayUpperLeft[1]
						coord.setX( xf )
						coord.setY( yf )
						painter.drawText( coord, s )

		xratio = float( geom.width() ) / 480.0
		yratio = float( geom.height() ) / 740.0
		if( self.annuEXEC ):
			painter.setPen( colorNormal )
			painter.fillRect( QtCore.QRectF( 380.0 * xratio, 420.0 * yratio, 36.0 * xratio, 5 * yratio ), colorNormal )
			
		if( self.annuMSG ):
			painter.setPen( colorORG )
			painter.setFont( QtGui.QFont( 'Consolas', fontSizeNormal, QtGui.QFont.Bold ) )
			xf = xratio * 443.0
			yf = yratio * 525.0
			coord.setX( xf )
			coord.setY( yf )
			painter.drawText( coord, 'M' )
			yf = yratio * 545
			coord.setY( yf )
			painter.drawText( coord, 'S' )
			yf = yratio * 565
			coord.setY( yf )
			painter.drawText( coord, 'G' )
		
		painter.end()

	def paintEvent( self, ev ):
		super( CDU747, self ).paintEvent( ev )
		if( self.lastPressKeyIndex >= 0 ):
			self.paintKeyPressFeedback( ev )
			self.paintDisplay( ev )
		else:
			self.paintDisplay( ev )

	def isInPosRect( self, ev, p, w, h, xratio, yratio ):
		cmpRect = ( ( p[0] * xratio, ( p[0] + w ) * xratio ),
					( p[1] * yratio, ( p[1] + h ) * yratio ) )
		if( ev.pos().x() > cmpRect[0][0] and ev.pos().x() < cmpRect[0][1] and
			ev.pos().y() > cmpRect[1][0] and ev.pos().y() < cmpRect[1][1] ):
			return True
		return False
		
	def isInPosCir( self, ev, p, rad, xratio, yratio ):
		dx = float( p[0] ) - float( ev.pos().x() )
		dx *= xratio
		dy = float( p[1] ) - float( ev.pos().y() )
		dy *= yratio
		d2 = dx * dx + dy * dy
		if( d2 < rad * rad ):
			return True
		return False

	def resizeEvent( self, ev ):
		geom = self.geometry()
		self.clickPosBitmap = [ [ -1 for y in xrange( geom.height() ) ] for x in xrange( geom.width() ) ]
		xratio = float( geom.width() ) / 480.0
		yratio = float( geom.height() ) / 740.0

		def rectSetBitmap( idx, src, w, h, dstBitmap, expandSz = 8 ):
			xstart = int( (src[0] - expandSz) * xratio )
			ystart = int( (src[1] - expandSz) * yratio )
			xend = xstart + int( ( w + expandSz * 2 ) * xratio )
			yend = ystart + int( ( h + expandSz * 2 ) * yratio )
			for y in xrange( ystart, yend ):
				for x in xrange( xstart, xend ):
					dstBitmap[x][y] = idx
			
		for i, p in enumerate( self.clickSpot ):
			if( i >= 38 ):
				rectSetBitmap( i, p, 28, 28, self.clickPosBitmap )
			elif( i >= 0 and i < 12 ):
				#L/R Keys
				rectSetBitmap( i, p, 28, 28, self.clickPosBitmap )
			elif( i >= 12 and i < 27 ):
				#FUNC Keys
				rectSetBitmap( i, p, 46, 31, self.clickPosBitmap )
			elif( i >= 27 and i < 38 ):
				#Number Keys
				rectSetBitmap( i, p, 32, 32, self.clickPosBitmap )

	def keyPressEvent( self, ev ):

		#print ev.key()
		
		idx = -1
		if( ev.key() >= 49 and ev.key() <= 57 ):
			#Number 1-9
			idx = ev.key() - 22
		elif( ev.key() == 48 ):
			#Number 0
			idx = 37
		elif( ev.key() == 46 ):
			#Number dot
			idx = 36
		elif( ev.key() == 46 ):
			#Number +
			idx = 38
		elif( ev.key() >= 65 and ev.key() <= 90 ):
			#Alpha
			idx = ev.key() - 26
		elif( ev.key() == 32 ):
			#Space
			idx = 65
		elif( ev.key() == 16777223 ):
			#Del
			idx = 66
		elif( ev.key() == 47 ):
			#/
			idx = 67
		elif( ev.key() == 16777219 ):
			#CLR (BackSpace)
			idx = 68
		elif( ev.key() >= 16777264 and ev.key() <= 16777275 ):
			if( ev.nativeModifiers() == 512 ):
				#F1-F12( CDU L1-L6/R1-R6 )
				idx = ev.key() - 16777264
			elif( ev.nativeModifiers() == 513 ):
				#SHIFT+F1-F12( INIT_REF - PROG, MENU - NAV_RAD)
				idx = ev.key() - 16777264 + 12
				if( idx >= 22 ):
					idx += 1
					
		elif( ev.key() == 16777220 ):
			#ENTER (EXEC)
			idx = 22
				
		elif( ev.key() >= 16777238 and ev.key() <= 16777239 ):
			#PGUP/DN
			idx = ev.key() - 16777238 + 25

			
		if( idx >= 0 and self.sock ):
			self.sock.send( self.setting.targetCDU + ':' + str( idx ) )
			
	def mouseReleaseEvent( self, ev ):
		try:
			idx = self.clickPosBitmap[ev.pos().x()][ev.pos().y()]

			if( idx >= 0 and self.sock ):
				self.sock.send( self.setting.targetCDU + ':' + str( idx ) )
				
				geom = self.geometry()
				xratio = float( geom.width() ) / 480.0
				yratio = float( geom.height() ) / 740.0
				
				self.lastPressKeyIndex = idx

				self.update()
				self.repaint()
				self.lastPressKeyIndex = -1

		except:
			pass

	def mouseDoubleClickEvent( self, ev ):
		try:
			idx = self.clickPosBitmap[ev.pos().x()][ev.pos().y()]
			if( idx >= 0 ):
				return
		except:
			pass
		
		if( self.isFullScreen() ):
			self.showNormal()
		else:
			self.showFullScreen()

def main():
	app = QtGui.QApplication( sys.argv )

	setting = Setting()
	if( os.path.isfile( SETTING_PATH ) ):
		with open( SETTING_PATH, 'rb' ) as fhdl:
			prevSetting = pickle.load( fhdl )

			for k,i in prevSetting.__dict__.items():
				setting.__dict__[k] = i

	if( len( sys.argv ) > 1 ):
		setting.fontScale = float( sys.argv[1] )

	if( len( sys.argv ) > 2 ):
		setting.ip = sys.argv[2]

	if( len( sys.argv ) > 3 ):
		setting.targetCDU = sys.argv[3]
	
	window = CDU747( None, setting )
	
	window.show()

	while( window.isVisible() ):
		app.processEvents()
		time.sleep( 1.0 / 1000.0 )


if __name__ == "__main__":
	main()
