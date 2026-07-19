#pragma once
#include <iostream>
#include <QMainWindow>
#include <QMessageBox>
#include "ui_MainWindow.h"
#include "MapView.h"
#include "nodes/ChassisControlNode.h"
#include "nodes/ManipulatorControlNode.h"
#include "nodes/AutonomousNode.h"
#include "nodes/ApplicationNode.h"
#include "CONFIG.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ChassisControlNode* chassis_node, ManipulatorControlNode* manipulator_node, AutonomousNode* autonomous_node, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMainPageClicked();
    void onChassisAndManipulatorClicked();
    void onChassisAndLaboratoryClicked();
    void onPoseNavigatorClicked();

    void onTeleoperationModeClicked();
    void onAutonomousModeClicked();
    void onBlockAllClicked();
    
    void onEnableCmdVel1Clicked();
    void onArmChassisClicked();
    void onCmdvelStatus(bool status);
    void onEnableJointStates1Clicked();
    void onArmManipulatorClicked();
    void onJointStatesStatus(bool status);

    void onGamepadChassisClicked();
    void onGamepadManipulatorClicked();
    void onGamepadNoneClicked();

    void onConnectClicked();



private:
    Ui::MainWindow* ui;
    MapView* map_view;
    ChassisControlNode* chassis_node;
    ManipulatorControlNode* manipulator_node;
    AutonomousNode* autonomous_node;


};
