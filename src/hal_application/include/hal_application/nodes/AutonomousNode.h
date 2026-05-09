#pragma once
#include "nodes/ApplicationNode.h"
#include <vector>
#include <map>
#include <functional>
#include <string>
#include <cmath>




class AutonomousNode : public ApplicationNode {
    Q_OBJECT
    
private:

public:
    AutonomousNode(QObject *parent = nullptr);
    ~AutonomousNode() override;
    void run() override final;

};