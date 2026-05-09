#pragma once
#include <string>
#include <QString>
#include <array>
#include <map>
#include <vector>
#include <ament_index_cpp/get_package_share_directory.hpp>




// ERC SETUP
constexpr double zero[2] = {3.2,11.6};
constexpr double pixels_per_meter = 5.0;
const QString image_path = QString::fromStdString(ament_index_cpp::get_package_share_directory("hal_application") + "/maps/costmap_erc_1.png");
const std::map<int, std::array<double,2>> arucoMap = {
    {1, {3.1374,4.3246}},
    {2, {9.0888,-4.5555}},
    {3, {8.2731,2.2478}},
    {4, {13.5552,3.3260}},
    {5, {17.6623,-2.7646}},
    {6, {23.8746,-2.3014}},
    {7, {27.7097,2.7192}},
    {8, {28.3320,8.6813}},
    {9, {25.8693,7.3431}},
    {10, {18.6570,4.5163}},
    {11, {14.9031,6.1368}},
    {12, {13.2623,11.3769}},
    {13, {10.0015,5.6827}},
    {14, {8.0354,12.9120}},
    {15, {2.7876,13.5601}}
};


// MEL SETUP
// constexpr double zero[2] = {6.0,21.5};
// constexpr double pixels_per_meter = 1.0/0.0375;
// const QString image_path = QString::fromStdString(ament_index_cpp::get_package_share_directory("hal_application") + "/maps/mapa_mel.png");


// MANIPULATOR PRESET
// FORMAT : X Y Z   W X Y Z   COLOR
const std::map<std::string, std::vector<float>> presetMap = {
    {"HOME1", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 0.0}},
    {"HOME2", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 0.0}},
    {"HOME3", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 0.0}},
    {"HOME4", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 0.0}},
    {"HOME5", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 1.0}},
    {"HOME6", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 1.0}},
    {"HOME7", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 1.0}},
    {"HOME8", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 1.0}},
    {"HOME9", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 2.0}},
    {"HOME11", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 2.0}},
    {"HOME22", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 2.0}},
    {"HOME33", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 2.0}},
    {"HOME44", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}},
    {"HOME55", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}},
    {"HOME66", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}},
    {"HOME77", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}},
    {"HOME88", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}},
    {"HOME99", {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 3.0}}
};

// MANIPULATOR MOTION
// FORMAT : X Y Z   W X Y Z   DURATION
const std::map<std::string, std::vector<std::vector<float>>> motionMap = {
    {"TEST", {
                    {0.7,0.0,0.9,   0.71,0.0,0.71,0.0, 6.0},
                    {0.7,-0.3,0.7,   0.71,0.0,0.71,0.0, 6.0},
                    {0.7,0.0,0.5,   0.71,0.0,0.71,0.0, 6.0},
                    {0.7,0.3,0.7,   0.71,0.0,0.71,0.0, 6.0}
                }
    },

    {"MOTION2", {
                    {0.36,-0.23,0.99,   0.49,-0.51,0.49,-0.51, 6.0},
                    {0.36,0.23,0.99,   0.49,-0.51,0.49,-0.51, 6.0}
                    // {0.7,0.0,1.2,   0.71,0.0,0.71,0.0, 1.0}
                }
    }

};



