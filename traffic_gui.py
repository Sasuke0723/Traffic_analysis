import sys
import subprocess
import os
import pandas as pd
import matplotlib.pyplot as plt
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLineEdit, QTextEdit, QLabel, QFileDialog, QComboBox,
    QSpinBox, QGroupBox, QTabWidget
)
from PyQt6.QtCore import Qt, QProcess

class TrafficAnalysisGUI(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Network Traffic Analysis System")
        self.setGeometry(100, 100, 1000, 700)
        
        # C程序路径
        self.c_program_path = os.path.join(os.getcwd(), "traffic_analysis.exe")
        self.csv_path = ""
        
        # 初始化UI
        self.init_ui()
        
        # 进程对象（用于调用C程序）
        self.process = QProcess()
        self.process.readyReadStandardOutput.connect(self.read_c_output)
        self.process.readyReadStandardError.connect(self.read_c_error)

    def init_ui(self):
        # 中心窗口
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        
        # 标签页
        tab_widget = QTabWidget()
        main_layout.addWidget(tab_widget)
        
        # 1. 数据读取标签页
        read_tab = QWidget()
        read_layout = QVBoxLayout(read_tab)
        
        # CSV文件选择
        file_layout = QHBoxLayout()
        self.csv_edit = QLineEdit()
        self.csv_edit.setPlaceholderText("CSV file path")
        file_btn = QPushButton("Select CSV")
        file_btn.clicked.connect(self.select_csv)
        read_btn = QPushButton("Read CSV Data")
        read_btn.clicked.connect(self.read_csv)
        
        file_layout.addWidget(QLabel("CSV File:"))
        file_layout.addWidget(self.csv_edit)
        file_layout.addWidget(file_btn)
        file_layout.addWidget(read_btn)
        read_layout.addLayout(file_layout)
        
        # 2. 流量分析标签页
        analysis_tab = QWidget()
        analysis_layout = QVBoxLayout(analysis_tab)
        
        # 排序选项
        sort_layout = QHBoxLayout()
        self.sort_combo = QComboBox()
        self.sort_combo.addItems([
            "Total Traffic", "HTTPS Nodes", "Send Ratio >80%"
        ])
        sort_btn = QPushButton("Run Sort")
        sort_btn.clicked.connect(self.run_sort)
        
        sort_layout.addWidget(QLabel("Sort Type:"))
        sort_layout.addWidget(self.sort_combo)
        sort_layout.addWidget(sort_btn)
        analysis_layout.addLayout(sort_layout)
        
        # 路径查找
        path_layout = QHBoxLayout()
        self.start_ip_edit = QLineEdit()
        self.start_ip_edit.setPlaceholderText("Start IP")
        self.end_ip_edit = QLineEdit()
        self.end_ip_edit.setPlaceholderText("End IP")
        path_btn = QPushButton("Find Path")
        path_btn.clicked.connect(self.find_path)
        
        path_layout.addWidget(QLabel("Start IP:"))
        path_layout.addWidget(self.start_ip_edit)
        path_layout.addWidget(QLabel("End IP:"))
        path_layout.addWidget(self.end_ip_edit)
        path_layout.addWidget(path_btn)
        analysis_layout.addLayout(path_layout)
        
        # 3. 拓扑检测标签页
        topology_tab = QWidget()
        topology_layout = QVBoxLayout(topology_tab)
        
        # 星型拓扑检测
        star_layout = QHBoxLayout()
        self.min_edge_spin = QSpinBox()
        self.min_edge_spin.setRange(1, 100)
        self.min_edge_spin.setValue(20)
        star_btn = QPushButton("Detect Star Topology")
        star_btn.clicked.connect(self.detect_star)
        
        star_layout.addWidget(QLabel("Min Edge Count:"))
        star_layout.addWidget(self.min_edge_spin)
        star_layout.addWidget(star_btn)
        topology_layout.addLayout(star_layout)
        
        # 安全规则检测
        security_btn = QPushButton("Detect Security Violations")
        security_btn.clicked.connect(self.detect_security)
        topology_layout.addWidget(security_btn)
        
        # 4. 结果显示标签页
        result_tab = QWidget()
        result_layout = QVBoxLayout(result_tab)
        self.result_text = QTextEdit()
        self.result_text.setReadOnly(True)
        result_layout.addWidget(self.result_text)
        
        # 添加标签页
        tab_widget.addTab(read_tab, "Data Read")
        tab_widget.addTab(analysis_tab, "Traffic Analysis")
        tab_widget.addTab(topology_tab, "Detection")
        tab_widget.addTab(result_tab, "Results")
        
        # 状态栏
        self.status_bar = self.statusBar()
        self.status_bar.showMessage("Ready")

    def select_csv(self):
        file_path, _ = QFileDialog.getOpenFileName(
            self, "Select CSV File", "", "CSV Files (*.csv)"
        )
        if file_path:
            self.csv_path = file_path
            self.csv_edit.setText(file_path)

    def run_c_command(self, command):
        """运行C程序命令"""
        self.result_text.clear()
        self.status_bar.showMessage("Running command...")
        self.process.start(self.c_program_path)
        # 向C程序输入命令（模拟交互）
        for cmd in command:
            self.process.write(f"{cmd}\n".encode())

    def read_c_output(self):
        """读取C程序标准输出"""
        output = self.process.readAllStandardOutput().data().decode()
        self.result_text.append(output)
        self.status_bar.showMessage("Command completed")

    def read_c_error(self):
        """读取C程序错误输出"""
        error = self.process.readAllStandardError().data().decode()
        self.result_text.append(f"Error: {error}")
        self.status_bar.showMessage("Command failed")

    def read_csv(self):
        """读取CSV数据"""
        if not self.csv_path:
            self.result_text.append("Please select a CSV file first!")
            return
        # C程序交互命令：1 -> CSV路径 -> 0
        command = ["1", self.csv_path, "0"]
        self.run_c_command(command)

    def run_sort(self):
        """运行流量排序"""
        sort_type = self.sort_combo.currentIndex() + 3  # 3=总流量,4=HTTPS,5=占比
        command = ["1", self.csv_path, str(sort_type), "0"]
        self.run_c_command(command)

    def find_path(self):
        """查找路径"""
        start_ip = self.start_ip_edit.text()
        end_ip = self.end_ip_edit.text()
        if not start_ip or not end_ip:
            self.result_text.append("Please enter start and end IP!")
            return
        command = ["1", self.csv_path, "6", start_ip, end_ip, "0"]
        self.run_c_command(command)

    def detect_star(self):
        """检测星型拓扑"""
        min_edge = self.min_edge_spin.value()
        command = ["1", self.csv_path, "7", str(min_edge), "0"]
        self.run_c_command(command)

    def detect_security(self):
        """检测安全违规"""
        command = ["1", self.csv_path, "8", "0"]
        self.run_c_command(command)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = TrafficAnalysisGUI()
    window.show()
    sys.exit(app.exec())