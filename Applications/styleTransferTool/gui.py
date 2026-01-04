import sys
import core
from PyQt5 import QtCore, QtGui, QtWidgets

class StyleTransferWidget(QtWidgets.QWidget):
    def __init__(self):
        super().__init__()

        font = QtGui.QFont()
        self.setWindowTitle("Style Transfer Tool")
        self.setObjectName("StyleTransferWidget")
        self.resize(741, 257)
        self.setMinimumSize(QtCore.QSize(741, 257))
        self.setMaximumSize(QtCore.QSize(741, 257))
        self.setStyleSheet("background-color: rgb(53, 53, 53);")

        self.inputImagePathText = QtWidgets.QPlainTextEdit(self)
        self.inputImagePathText.setGeometry(QtCore.QRect(10, 40, 601, 31))
        self.inputImagePathText.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputImagePathText.setObjectName("inputImagePathText")

        self.inputImageBrowse = QtWidgets.QPushButton(self)
        self.inputImageBrowse.clicked.connect(self.set_content_image_path)
        self.inputImageBrowse.setGeometry(QtCore.QRect(620, 40, 111, 31))
        self.inputImageBrowse.setStyleSheet("background-color: rgb(225, 0, 0);\n"
                                            "color: rgb(255, 255, 255);")
        self.inputImageBrowse.setObjectName("inputImageBrowse")
        self.inputImageBrowse.setText("Browse")

        self.inputStyleImageBrowse = QtWidgets.QPushButton(self)
        self.inputStyleImageBrowse.clicked.connect(self.set_style_image_path)
        self.inputStyleImageBrowse.setGeometry(QtCore.QRect(620, 110, 111, 31))
        self.inputStyleImageBrowse.setStyleSheet("background-color: rgb(225, 0, 0);\n"
                                                "color: rgb(255, 255, 255);")
        self.inputStyleImageBrowse.setObjectName("inputStyleImageBrowse")
        self.inputStyleImageBrowse.setText("Browse")

        self.inputStyleImagePathText = QtWidgets.QPlainTextEdit(self)
        self.inputStyleImagePathText.setGeometry(QtCore.QRect(10, 110, 601, 31))
        self.inputStyleImagePathText.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputStyleImagePathText.setObjectName("inputStyleImagePathText")

        self.start = QtWidgets.QPushButton(self)
        self.start.clicked.connect(self.start_style_transfer)
        self.start.setGeometry(QtCore.QRect(620, 170, 111, 31))
        self.start.setStyleSheet("background-color: rgb(48, 220, 0);\n"
                                "color: rgb(255, 255, 255);")
        self.start.setObjectName("start")
        self.start.setText("Start")

        font.setPointSize(14)
        self.inputImagePath = QtWidgets.QLabel(self)
        self.inputImagePath.setGeometry(QtCore.QRect(260, 10, 161, 21))
        self.inputImagePath.setFont(font)
        self.inputImagePath.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputImagePath.setObjectName("inputImagePath")
        self.inputImagePath.setText("Input Image Path:")

        self.inputStyleImagePath = QtWidgets.QLabel(self)
        self.inputStyleImagePath.setGeometry(QtCore.QRect(230, 80, 211, 21))
        self.inputStyleImagePath.setFont(font)
        self.inputStyleImagePath.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputStyleImagePath.setObjectName("inputStyleImagePath")
        self.inputStyleImagePath.setText("Input Style Image Path:")

        self.inputImageInfluenceText = QtWidgets.QPlainTextEdit(self)
        self.inputImageInfluenceText.setGeometry(QtCore.QRect(40, 180, 61, 31))
        self.inputImageInfluenceText.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputImageInfluenceText.setObjectName("inputImageInfluenceText")
        self.inputImageInfluenceText.setPlainText("1")

        font.setPointSize(10)
        self.inputImageInfluence = QtWidgets.QLabel(self)
        self.inputImageInfluence.setGeometry(QtCore.QRect(10, 150, 131, 21))
        self.inputImageInfluence.setFont(font)
        self.inputImageInfluence.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputImageInfluence.setObjectName("inputImageInfluence")
        self.inputImageInfluence.setText("Input Image Influence:")

        self.inputStyleImageInfluenceText = QtWidgets.QPlainTextEdit(self)
        self.inputStyleImageInfluenceText.setGeometry(QtCore.QRect(210, 180, 61, 31))
        self.inputStyleImageInfluenceText.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputStyleImageInfluenceText.setObjectName("inputStyleImageInfluenceText")
        self.inputStyleImageInfluenceText.setPlainText("1000")

        self.inputStyleImageInfluence = QtWidgets.QLabel(self)
        self.inputStyleImageInfluence.setGeometry(QtCore.QRect(170, 150, 131, 21))
        self.inputStyleImageInfluence.setFont(font)
        self.inputStyleImageInfluence.setStyleSheet("color: rgb(255, 255, 255);")
        self.inputStyleImageInfluence.setObjectName("inputStyleImageInfluence")
        self.inputStyleImageInfluence.setText("Style Image Influence:")

        self.qualitySteps = QtWidgets.QLabel(self)
        self.qualitySteps.setGeometry(QtCore.QRect(350, 150, 81, 21))
        self.qualitySteps.setFont(font)
        self.qualitySteps.setStyleSheet("color: rgb(255, 255, 255);")
        self.qualitySteps.setObjectName("qualitySteps")
        self.qualitySteps.setText("Quality Steps:")

        self.qualityStepsText = QtWidgets.QPlainTextEdit(self)
        self.qualityStepsText.setGeometry(QtCore.QRect(360, 180, 61, 31))
        self.qualityStepsText.setStyleSheet("color: rgb(255, 255, 255);")
        self.qualityStepsText.setObjectName("qualityStepsText")
        self.qualityStepsText.setPlainText("500")

        self.reduceCheckBox = QtWidgets.QCheckBox(self)
        self.reduceCheckBox.setGeometry(QtCore.QRect(500, 220, 70, 17))
        self.reduceCheckBox.setStyleSheet("color: rgb(255, 255, 255);")
        self.reduceCheckBox.setChecked(True)
        self.reduceCheckBox.setObjectName("reduceCheckBox")
        self.reduceCheckBox.setText("Reduce")

        self.reducedSize = QtWidgets.QLabel(self)
        self.reducedSize.setGeometry(QtCore.QRect(490, 150, 81, 21))
        self.reducedSize.setStyleSheet("color: rgb(255, 255, 255);")
        self.reducedSize.setObjectName("reducedSize")
        self.reducedSize.setText("Reduced Size:")

        self.reduceSizeText = QtWidgets.QPlainTextEdit(self)
        self.reduceSizeText.setGeometry(QtCore.QRect(500, 180, 61, 31))
        self.reduceSizeText.setStyleSheet("color: rgb(255, 255, 255);")
        self.reduceSizeText.setObjectName("reduceSizeText")
        self.reduceSizeText.setPlainText("500")

    def set_content_image_path(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Select Content Image", "", 'Image files (*.png *.jpg *.jpeg)')
        self.inputImagePathText.setPlainText(path[0])

    def set_style_image_path(self):
        path = QtWidgets.QFileDialog.getOpenFileName(self, "Select Style Image", "", 'Image files (*.png *.jpg *.jpeg)')
        self.inputStyleImagePathText.setPlainText(path[0])

    def start_style_transfer(self):
        content_image_path = self.inputImagePathText.toPlainText()
        content_image_inf = int(self.inputImageInfluenceText.toPlainText())
        style_image_path = self.inputStyleImagePathText.toPlainText()
        stle_image_inf = int(self.inputStyleImageInfluenceText.toPlainText())
        quality_steps = int(self.qualityStepsText.toPlainText())
        reduced_size = int(self.reduceSizeText.toPlainText())
        reduce_flag = self.reduceCheckBox.isChecked()

        transfer = core.styleTransfer()
        transfer.transferStyle(content_image_inf, stle_image_inf, reduce_flag, reduced_size, quality_steps, content_image_path, style_image_path)

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    widget = StyleTransferWidget()
    widget.show()
    sys.exit(app.exec_())
