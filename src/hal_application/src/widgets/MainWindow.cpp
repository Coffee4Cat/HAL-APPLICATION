#include "widgets/MainWindow.h"
#include "widgets/JoystickWidget.h"
#include "widgets/JoystickWidgetM.h"
#include "widgets/VerticalJoystickWidget.h"
#include "widgets/HorizontalJoystickWidget.h"
#include "widgets/AxisView.h"
#include "CONFIG.h"
#include "widgets/Diode.h"

// HELPER FUNCTIONS - BEGIN
Diode* diodeFactory(bool status, MainWindow* parent, QWidget* placeholder) {
    Diode *diode = new Diode(status, parent);
    auto diode_layout = new QVBoxLayout(placeholder);
    diode_layout->setContentsMargins(0, 0, 0, 0);
    diode_layout->addWidget(diode);
    return diode;
} 

template<typename JoystickType>
JoystickType* joystickFactory(MainWindow* parent, QWidget* placeholder) {
    JoystickType* joy = new JoystickType(parent);
    auto joy_layout = new QVBoxLayout(placeholder);
    joy_layout->setContentsMargins(0, 0, 0, 0);
    joy_layout->addWidget(joy);
    return joy;
}
// HELPER FUNCTIONS - END;







MainWindow::MainWindow(ChassisControlNode* chassis_node, ManipulatorControlNode* manipulator_node, AutonomousNode* autonomous_node, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), chassis_node(chassis_node), manipulator_node(manipulator_node), autonomous_node(autonomous_node) {
    setWindowTitle("HalApplication [MAIN PAGE]");
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    // COMM ALIVE
    Diode* alive_diode_1 = diodeFactory(false, this, ui->alive_diode_1);
    Diode* alive_diode_2 = diodeFactory(false, this, ui->alive_diode_2);
    for(Diode* alive_diode : QList<Diode*>({alive_diode_1,alive_diode_2})) {
        connect(chassis_node, &ChassisControlNode::aliveStatus, alive_diode, [alive_diode] (bool status) {
            if (status) {alive_diode->enable();} else {alive_diode->disable();}
        });
    }




    // TELEOPERATION
    // CHASSIS
    JoystickWidget *joystick = joystickFactory<JoystickWidget>(this, ui->JoystickPlaceholder);
    connect(joystick, &JoystickWidget::joystickMoved, chassis_node, &ChassisControlNode::updateVelocity);
    connect(ui->block_all_button_3, &QPushButton::clicked, this, &MainWindow::onBlockAllClicked);
    // LAMP INDICATOR
    connect(ui->lamp_red_button, &QPushButton::clicked, chassis_node, [chassis_node] () {
        // QString red = QString("red");
        chassis_node->lightChange("red");
    });
    connect(ui->lamp_green_button, &QPushButton::clicked, chassis_node, [chassis_node] () {
        // QString green = QString("green");
        chassis_node->lightChange("green");
    });
    connect(ui->lamp_blue_button, &QPushButton::clicked, chassis_node, [chassis_node] () {
        // QString blue = QString("blue");
        chassis_node->lightChange("blue");
    });


    // GENERAL CONTROL SETTINGS
    Diode *chassis_gcs_diode = diodeFactory(false, this, ui->chassis_gcs_diode);
    Diode *manipulator_gcs_diode = diodeFactory(false, this, ui->manipulator_gcs_diode);
    connect(chassis_node, &ApplicationNode::status, chassis_gcs_diode, [chassis_gcs_diode](bool status){
        if (status) {chassis_gcs_diode->enable();} else {chassis_gcs_diode->disable();}
    } );
    connect(manipulator_node, &ApplicationNode::status, manipulator_gcs_diode, [manipulator_gcs_diode](bool status){
        if (status) {manipulator_gcs_diode->enable();} else {manipulator_gcs_diode->disable();}
    } );


    // GAMEPAD ACCESS
    Diode *gamepad_chassis_diode = diodeFactory(false, this, ui->gamepad_chassis_diode);
    Diode *gamepad_manipulator_diode = diodeFactory(false, this, ui->gamepad_manipulator_diode);
    connect(ui->gamepad_chassis_button, &QPushButton::clicked, this, &MainWindow::onGamepadChassisClicked);
    connect(ui->gamepad_manipulator_button, &QPushButton::clicked, this, &MainWindow::onGamepadManipulatorClicked);
    connect(ui->gamepad_none_button, &QPushButton::clicked, this, &MainWindow::onGamepadNoneClicked);
    connect(chassis_node, &ChassisControlNode::gamepadStatus, gamepad_chassis_diode, [gamepad_chassis_diode](bool status){
        if (status) {gamepad_chassis_diode->enable();} else {gamepad_chassis_diode->disable();}
    } );
    connect(manipulator_node, &ManipulatorControlNode::gamepadStatus, gamepad_manipulator_diode, [gamepad_manipulator_diode](bool status){
        if (status) {gamepad_manipulator_diode->enable();} else {gamepad_manipulator_diode->disable();}
    } );


    // PRESETS & MOTION SEQUENCER
    std::vector<std::string> color_vec = {"#2d89ef","#812def","#ef2d87","#82b60a"};
    for (const auto& [preset_name, preset] : presetMap) {
        QPushButton* btn = new QPushButton(QString::fromStdString(preset_name), ui->preset_widget);
        int color_index = static_cast<int>(preset[7]);
        QString color = QString::fromStdString(color_vec[color_index]);

        btn->setMinimumHeight(50);
        btn->setStyleSheet(
            QString(
                "QPushButton { "
                "background-color: %1; "
                "color: rgba(30,30,30,255); "
                "border: 0px solid rgba(255, 251, 227, 255); "
                "border-radius: 10px; "
                "padding: 6px 12px; "
                "font: 75 25pt \"Monospace\"; "
                "} "

                "QPushButton:hover { "
                "background-color: #0a3020; "
                "color: rgba(255,255,255,255); "
                "border: 4px solid #0a3020; "
                "} "

                "QPushButton:pressed { "
                "background-color: rgba(160,255,216,255); "
                "color: rgba(30,30,30,255); "
                "border: 4px solid #fc9997; "
                "}"
            )
            .arg(color)
        );

        ui->preset_layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, manipulator_node, [manipulator_node, preset_name]() {
                manipulator_node->moveToPreset(preset_name);
            }
        );
    }

    for (const auto& [motion_name, motion] : motionMap) {
        QPushButton* btn = new QPushButton(QString::fromStdString(motion_name), ui->motion_widget);

        btn->setMinimumHeight(50);
        btn->setStyleSheet(
            QString(
                "QPushButton { "
                "background-color: #2d89ef; "
                "color: rgba(30,30,30,255); "
                "border: 0px solid rgba(255, 251, 227, 255); "
                "border-radius: 10px; "
                "padding: 6px 12px; "
                "font: 75 25pt \"Monospace\"; "
                "} "

                "QPushButton:hover { "
                "background-color: #0a3020; "
                "color: rgba(255,255,255,255); "
                "border: 4px solid #0a3020; "
                "} "

                "QPushButton:pressed { "
                "background-color: rgba(160,255,216,255); "
                "color: rgba(30,30,30,255); "
                "border: 4px solid #fc9997; "
                "}"
            )
        );

        ui->motion_layout->addWidget(btn);

        connect(btn, &QPushButton::clicked, manipulator_node, [manipulator_node, motion_name]() {
                manipulator_node->executeSequence(motion_name);
            }
        );
    }

    // MANIPULATOR SETTINGS
    int minValSlider = ui->motion_speed_slider->minimum();
    int maxValSLider = ui->motion_speed_slider->maximum();
    ui->min_motion_speed->setText(QString::number(minValSlider));
    ui->max_motion_speed->setText(QString::number(maxValSLider));
    connect(ui->motion_speed_slider, &QSlider::valueChanged, [manipulator_node, this](int value) {
        ui->current_motion_speed->setText("Current: " + QString::number(value));
        manipulator_node->changeMotionMul(value);
    });




    // ACTIVATION
    connect(ui->enable_cmd_vel_1, &QPushButton::clicked, this, &MainWindow::onEnableCmdVel1Clicked);
    connect(ui->chassis_arm_button, &QPushButton::clicked, this, &MainWindow::onArmChassisClicked);
    connect(ui->manipulator_arm_button, &QPushButton::clicked, this, &MainWindow::onArmManipulatorClicked);
    connect(ui->enable_joint_states_1, &QPushButton::clicked, this, &MainWindow::onEnableJointStates1Clicked);
    connect(chassis_node, &ApplicationNode::status, this, &MainWindow::onCmdvelStatus);
    connect(manipulator_node, &ApplicationNode::status, this, &MainWindow::onJointStatesStatus);
    // MANIPULATOR
    //JOYSTICKS
    JoystickWidgetM *joystick_xy_t = joystickFactory<JoystickWidgetM>(this, ui->JoystickPlaceholder_2);
    connect(joystick_xy_t, &JoystickWidgetM::joystickMoved, manipulator_node, &ManipulatorControlNode::updateXYT);

    VerticalJoystickWidget *joystick_z_t = joystickFactory<VerticalJoystickWidget>(this, ui->JoystickPlaceholder_3);
    connect(joystick_z_t, &VerticalJoystickWidget::joystickMoved, manipulator_node, &ManipulatorControlNode::updateZT);

    VerticalJoystickWidget *joystick_y_o = joystickFactory<VerticalJoystickWidget>(this, ui->JoystickPlaceholder_5);
    connect(joystick_y_o, &VerticalJoystickWidget::joystickMoved, manipulator_node, &ManipulatorControlNode::updateYO);

    HorizontalJoystickWidget *joystick_z_o = joystickFactory<HorizontalJoystickWidget>(this, ui->JoystickPlaceholder_6);
    connect(joystick_z_o, &HorizontalJoystickWidget::joystickMoved, manipulator_node, &ManipulatorControlNode::updateZO);

    HorizontalJoystickWidget *joystick_x_o = joystickFactory<HorizontalJoystickWidget>(this, ui->JoystickPlaceholder_4);
    connect(joystick_x_o, &HorizontalJoystickWidget::joystickMoved, manipulator_node, &ManipulatorControlNode::updateXO);

    // GRIPPER
    connect(ui->gripper_open_button, &QPushButton::pressed, manipulator_node, &ManipulatorControlNode::gripperOpen);
    connect(ui->gripper_close_button, &QPushButton::pressed, manipulator_node, &ManipulatorControlNode::gripperClose);
    connect(ui->gripper_stop_button, &QPushButton::pressed, manipulator_node, &ManipulatorControlNode::gripperStop);
    connect(ui->gripper_open_button, &QPushButton::released, manipulator_node, &ManipulatorControlNode::gripperStop);
    connect(ui->gripper_close_button, &QPushButton::released, manipulator_node, &ManipulatorControlNode::gripperStop);

    // ORIENTATION PRESET
    connect(ui->front_preset_button, &QPushButton::clicked, manipulator_node, &ManipulatorControlNode::frontPreset);
    connect(ui->ground_preset_button, &QPushButton::clicked, manipulator_node, &ManipulatorControlNode::groundPreset);
    // HOME
    connect(ui->home_button, &QPushButton::clicked, manipulator_node, &ManipulatorControlNode::home);
    // MODE 45
    Diode* mode45_diode = diodeFactory(false, this, ui->mode45_diode);
    connect(ui->mode45_button, &QPushButton::clicked, manipulator_node, &ManipulatorControlNode::toggleMode45);
    connect(manipulator_node, &ManipulatorControlNode::mode45Status, mode45_diode, [mode45_diode] (bool status) {
        if (status) { mode45_diode->enable(); } else { mode45_diode->disable(); }
    });




    // TELEMETRY
    // connect(mux_node, &MuxNode::telemetry, this, [this] (double x, double z) {
    //     ui->chassis_x->setText(QString::number(x, 'f',2));
    //     ui->chassis_z->setText(QString::number(z, 'f',2));
    // });
    connect(manipulator_node, &ManipulatorControlNode::telemetry, this, [this] (double t0,double t1,double t2,double o0,double o1,double o2,double o3) {
        ui->manipulator_x_t->setText(QString::number(t0, 'f',2));
        ui->manipulator_y_t->setText(QString::number(t1, 'f',2));
        ui->manipulator_z_t->setText(QString::number(t2, 'f',2));
        ui->manipulator_w_o->setText(QString::number(o0, 'f',2));
        ui->manipulator_x_o->setText(QString::number(o1, 'f',2));
        ui->manipulator_y_o->setText(QString::number(o2, 'f',2));
        ui->manipulator_z_o->setText(QString::number(o3, 'f',2));
    });
    // DOF LIMIT
    Diode* d1_diode = diodeFactory(true, this, ui->d1_diode);
    Diode* d2_diode = diodeFactory(true, this, ui->d2_diode);
    Diode* d3_diode = diodeFactory(true, this, ui->d3_diode);
    Diode* d4_diode = diodeFactory(true, this, ui->d4_diode);
    Diode* d5_diode = diodeFactory(true, this, ui->d5_diode);
    Diode* d6_diode = diodeFactory(true, this, ui->d6_diode);
    connect(manipulator_node, &ManipulatorControlNode::dofStatus, this, [d1_diode, d2_diode, d3_diode, d4_diode, d5_diode, d6_diode] (int d1,int d2,int d3,int d4,int d5,int d6) {
        if (d1 == 0) { d1_diode->enable(); } else if (d1 == 1) { d1_diode->danger(); } else if (d1 == 2) { d1_diode->disable(); }
        if (d2 == 0) { d2_diode->enable(); } else if (d2 == 1) { d2_diode->danger(); } else if (d2 == 2) { d2_diode->disable(); }
        if (d3 == 0) { d3_diode->enable(); } else if (d3 == 1) { d3_diode->danger(); } else if (d3 == 2) { d3_diode->disable(); }
        if (d4 == 0) { d4_diode->enable(); } else if (d4 == 1) { d4_diode->danger(); } else if (d4 == 2) { d4_diode->disable(); }
        if (d5 == 0) { d5_diode->enable(); } else if (d5 == 1) { d5_diode->danger(); } else if (d5 == 2) { d5_diode->disable(); }
        if (d6 == 0) { d6_diode->enable(); } else if (d6 == 1) { d6_diode->danger(); } else if (d6 == 2) { d6_diode->disable(); }
    });
    // AXIS VIEW
    AxisView* axis_view = new AxisView(this);
    auto axis_view_layout = new QVBoxLayout(ui->axis_view);
    axis_view_layout->setContentsMargins(0, 0, 0, 0);
    axis_view_layout->addWidget(axis_view);
    connect(manipulator_node, &ManipulatorControlNode::dof6Angle, axis_view, &AxisView::rotate);


    // Mani









    // POSE NAVIGATOR
    // TELEMETRY
    // connect(mux_node, &MuxNode::telemetry, this, [this] (double x, double z) {
    //     ui->chassis_x_2->setText(QString::number(x, 'f',2));
    //     ui->chassis_z_2->setText(QString::number(z, 'f',2));
    // });

    // Action Panel
    connect(ui->block_all_button_2, &QPushButton::clicked, this, &MainWindow::onBlockAllClicked);
    // CURRENT ACTION

    // MAPVIEW 
    // QString image_path = QString::fromStdString(ament_index_cpp::get_package_share_directory("hal_application") + "/maps/costmap_erc_marsyard.png");
    // QString image_path = QString::fromStdString(ament_index_cpp::get_package_share_directory("hal_application") + "/maps/mapa_mel.png");
    map_view = new MapView(this);
    map_view->loadMap(image_path);
    auto map_view_layout = new QVBoxLayout(ui->map_view_placeholder);
    map_view_layout->setContentsMargins(0,0,0,0);
    map_view_layout->addWidget(map_view);
    // ARROW ITEM







    // APPLICATION-NAVIGATION SLOTS & MAIN MENU
    connect(ui->chassis_and_manipulator_button, &QPushButton::clicked, this, &MainWindow::onChassisAndManipulatorClicked);
    // connect(ui->chassis_and_laboratory_button, &QPushButton::clicked, this, &MainWindow::onChassisAndLaboratoryClicked);
    for(QPushButton* goback_button : QList<QPushButton*>({ui->goback,ui->goback_2,ui->goback_3,ui->goback_4})) {
        connect(goback_button, &QPushButton::clicked, this, &MainWindow::onMainPageClicked);
    }










    // MUX DIODES
    Diode *teleoperation_mode_diode = diodeFactory(false, this, ui->teleoperation_mode_diode);
    Diode *autonomous_mode_diode = diodeFactory(false, this, ui->autonomous_mode_diode);
    Diode *autonomy_diode_2 = diodeFactory(false, this, ui->autonomy_diode_2);
    connect(ui->teleoperation_mode_button, &QPushButton::clicked, this, &MainWindow::onTeleoperationModeClicked);
    connect(ui->autonomous_mode_button, &QPushButton::clicked, this, &MainWindow::onAutonomousModeClicked);
    connect(ui->block_all_button, &QPushButton::clicked, this, &MainWindow::onBlockAllClicked);
    // connect(mux_node, &MuxNode::muxTeleoperatorStatus, teleoperation_mode_diode,[teleoperation_mode_diode](bool status){
    //     if (status) {teleoperation_mode_diode->enable();} else {teleoperation_mode_diode->disable();}
    // } );
    // connect(mux_node, &MuxNode::muxAutonomyStatus, autonomous_mode_diode,[autonomous_mode_diode](bool status){
    //     if (status) {autonomous_mode_diode->enable();} else {autonomous_mode_diode->disable();}
    // } );
    // connect(mux_node, &MuxNode::muxAutonomyStatus, autonomy_diode_2,[autonomy_diode_2](bool status){
    //     if (status) {autonomy_diode_2->enable();} else {autonomy_diode_2->disable();}
    // } );


}
MainWindow::~MainWindow() {
    delete ui;
}




// APPLICTAION-NAVIGATION SLOTS
void MainWindow::onChassisAndManipulatorClicked() {
    setWindowTitle("HalApplication [CHASSIS AND MANIPULATOR]");
    ui->stackedWidget->setCurrentIndex(3);
}
void MainWindow::onChassisAndLaboratoryClicked() {
    setWindowTitle("HalApplication [CHASSIS AND LABORATORY]");
    ui->stackedWidget->setCurrentIndex(2);
}
void MainWindow::onPoseNavigatorClicked() {
    setWindowTitle("HalApplication [POSE NAVIGATOR]");
    ui->stackedWidget->setCurrentIndex(1);
}
void MainWindow::onMainPageClicked() {
    setWindowTitle("HalApplication [MAIN PAGE]");
    ui->stackedWidget->setCurrentIndex(0);
}
void MainWindow::onTeleoperationModeClicked() {
    // mux_node->teleoperatorMode();
    chassis_node->zeroVelocity();
}
void MainWindow::onAutonomousModeClicked() {
    // mux_node->autonomyMode();
    chassis_node->zeroVelocity();

}
void MainWindow::onBlockAllClicked() {
    // mux_node->blockAll();
    chassis_node->zeroVelocity();
}





// CHASSIS AND MANIPULATOR
void MainWindow::onEnableCmdVel1Clicked() {
    chassis_node->toggle_status();
}
void MainWindow::onArmChassisClicked() {
    chassis_node->armChassis();
    ui->chassis_arm_button->setText("Armed");
}
void MainWindow::onCmdvelStatus(bool status) {
    if (status) {
        ui->enable_cmd_vel_1->setText("Activated");
    } else {
        ui->enable_cmd_vel_1->setText("Deactivated");
    }
}
void MainWindow::onEnableJointStates1Clicked() {
    manipulator_node->toggle_status();
}
void MainWindow::onArmManipulatorClicked() {
    manipulator_node->initManipulator();
    ui->manipulator_arm_button->setText("Armed");
}
void MainWindow::onJointStatesStatus(bool status) {
    if (status) {
        ui->enable_joint_states_1->setText("Activated");
    } else {
        ui->enable_joint_states_1->setText("Deactivated");
    }
}



// GAMEPAD
void MainWindow::onGamepadChassisClicked() {
    chassis_node->enableGamepad();
    manipulator_node->disableGamepad();
}
void MainWindow::onGamepadManipulatorClicked() {
    chassis_node->disableGamepad();
    manipulator_node->enableGamepad();
}
void MainWindow::onGamepadNoneClicked() {
    chassis_node->disableGamepad();
    manipulator_node->disableGamepad();
}


