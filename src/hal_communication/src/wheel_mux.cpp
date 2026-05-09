#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/bool.hpp>
#include <rcl_interfaces/msg/set_parameters_result.hpp>



// OPIS WĘZŁA NA SAMYM DOLE

class WheelMuxNode : public rclcpp::Node {
public:

    WheelMuxNode() : Node("wheel_mux") {
        this->declare_parameter<std::string>("source", "app");
        source = this->get_parameter("source").as_string();
        param_cb_handle = this->add_on_set_parameters_callback( std::bind(&WheelMuxNode::on_param_change, this, std::placeholders::_1) );

        sub_autonomy = this->create_subscription<geometry_msgs::msg::Twist>( "/cmd_vel", 10, std::bind(&WheelMuxNode::autonomy_cb, this, std::placeholders::_1));
        sub_application = this->create_subscription<geometry_msgs::msg::Twist>("/app/cmd_vel", 10,std::bind(&WheelMuxNode::application_cb, this, std::placeholders::_1));
        sub_estop = this->create_subscription<std_msgs::msg::Bool>("/mux/estop", 10, std::bind(&WheelMuxNode::estop_cb, this, std::placeholders::_1));

        pub_cmd_vel = this->create_publisher<geometry_msgs::msg::Twist>("/mux/cmd_vel", 10);

        RCLCPP_INFO(this->get_logger(), "WheelMux started, source=%s", source.c_str());
    }

private:
    void autonomy_cb(const geometry_msgs::msg::Twist::SharedPtr msg) {
        if (estop) {return;}
        if (source == "auto") {pub_cmd_vel->publish(*msg);}
    }

    void application_cb(const geometry_msgs::msg::Twist::SharedPtr msg) {
        if (estop) {return;}
        if (source == "app") {pub_cmd_vel->publish(*msg);}
    }

    void estop_cb(const std_msgs::msg::Bool::SharedPtr msg) {
        estop = msg->data;
        if (estop) {
            geometry_msgs::msg::Twist zero;
            pub_cmd_vel->publish(zero);
            RCLCPP_WARN(this->get_logger(), "E-STOP Activated");
        } else {
            RCLCPP_INFO(this->get_logger(), "E-STOP Released");
        }
    }

    rcl_interfaces::msg::SetParametersResult on_param_change(const std::vector<rclcpp::Parameter> &params)
    {
        rcl_interfaces::msg::SetParametersResult result;
        result.successful = true;

        for (const auto &param : params) {
            if (param.get_name() == "source") {
                auto value = param.as_string();
                if (value != "auto" && value != "app") {
                    result.successful = false;
                    result.reason = "Invalid source";
                    return result;
                }

                source = value;
                RCLCPP_INFO(this->get_logger(), "Source switched to %s", source.c_str());
            }
        }
        return result;
    }

    std::string source{"auto"};
    bool estop{false};

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_autonomy;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_application;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_estop;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_cmd_vel;
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_cb_handle;
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<WheelMuxNode>());
  rclcpp::shutdown();
  return 0;
}






/*
WHEEL-MUX
Węzeł służący do sterowania przepływem Twistów na koła.
Za pomocą zmiany parametru 'source' można wybrać czy do kół idą polecenia autonomii czy polecenia aplikacji:

ros2 param set /wheel_mux source auto
ros2 param set /wheel_mux source app

/app/cmd_vel - [<] -  kanał dla aplikacji
/cmd_vel - [<] - kanał dla NAV2
/mux/cmd_vel - [>] - kanał do interakcji z wheel_interface (czyli już bezpośrednio z łazikiem)
/mux/estop - [<] - kanał służący do CAŁKOWITEGO ZATRZYMANIA WYSYŁANIA RAMEK. Jak wyślesz True to zablokuje transmisje. Wyślij true i łazik znowu ruszy. 


Węzeł jest załączany wraz z wheel_interface za pomocą launchfile communication.launch.py
*/
