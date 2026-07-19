#pragma once
#include <arpa/inet.h>
#include <thread>
#include <mutex>


#include "hal_interfaces/srv/communication.hpp"
#include "hal_interfaces/msg/transmission.hpp"
#include "rclcpp/rclcpp.hpp"

namespace HAL {
namespace comm::CONST {

/** Number of bytes in the frame */
const int COMMUNICATION_FRAME_SIZE = 19;
/** Maximal number of connection attempts before returning failure */
const int MAX_ATTEMPT_COUNT = 4;
/** Name of communication service */
const std::string SRV_NAME = "comm_server";
/** Name of communication params on parameter server */
const std::string PARAM_PREFIX = "communication";
/** Name of server IP on parameter server  */
const std::string PARAM_IP = "rover_IP";
/** Name of server port on parameter server  */
const std::string PARAM_PORT = "rover_port";
/** Name of receiver topic */
const std::string TRANSMISSION_TOPIC = "/transmission";

}  // namespace comm::CONST

/** Parameters of TCP connection  */
struct TCPParams {
  std::string IP;
  int port;
};

class IHandler {
 public:
  /**
   * @brief Restarts connection by closing and reopening the socket
   * @returns if OK empty optional, if error optional with error string
   */
  virtual std::optional<std::string> restartConnection() = 0;

  /**
   * @brief Sends message to TCP server
   * @param data meassage to server
   * @returns if OK empty optional, if error optional with error string
   */
  virtual std::optional<std::string> sendMessage(const std::string &data) = 0;

  virtual std::string receiveMessage() = 0;
};

/** Handling of TCP connection  */
class TCPClient : public IHandler {
 private:
  /** Parameters of TCP connection  */
  TCPParams serverParams_;
  /** IP address of TCP server  */
  struct sockaddr_in servAddr_;
  /** Socket mutex */
  static std::mutex sockFdMutex;
  
  /**
   * @brief Setup of connection params
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> initConnection_();
  
  public:
  void setParams(const TCPParams& params) { serverParams_ = params; }
  TCPClient() {}
  TCPClient(const TCPParams &serverParams);
  ~TCPClient();
  
  
  int sockFd_ = -1;
  /**
   * @brief Restarts connection by closing and reopening the socket
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> restartConnection() override;


  /**
   * @brief Sends message to TCP server
   * @param data meassage to server
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> sendMessage(const std::string &data) override;
  std::string receiveMessage() override;
};


/** Handling of bluetooth connection  */
class BTClient : public IHandler {
 public:
  ~BTClient() {}

  
  /**
   * @brief Restarts connection by closing and reopening the socket
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> restartConnection() override { return {}; }


  /**
   * @brief Sends message to TCP server
   * @param data meassage to server
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> sendMessage(const std::string &data) override { return {}; }

  std::string receiveMessage() override{ return {}; }
};

/** ROS server class that handles [ROS <-> rover] communication */
class CommunicationServer : public rclcpp::Node {
 private:
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_cb_handle_;
  rcl_interfaces::msg::SetParametersResult on_param_change(const std::vector<rclcpp::Parameter> &params);
  rclcpp::Service<hal_interfaces::srv::Communication>::SharedPtr server_;
  rclcpp::Publisher<hal_interfaces::msg::Transmission>::SharedPtr transmitter_;
  /** Handler of TCP connection  */
  TCPClient tcpHandler_;
  /** Handler of bluetooth connection  */
  BTClient btHandler_;

  /**  TCP listener thread */
  std::thread background_thread_;

  /** Flag for stopping the thread */
  std::atomic_bool running_{true};

  /**
   * @brief Converts decimal numbers to hex
   * @param dec_num decimal number
   * @returns number in hex
   */
  std::string convert2Hex_(int dec_num);

  /**
   * @brief Converts decimal numbers to values to hex according to communication
   * standard described in docs
   * @param id ID of message
   * @param data vector with message data
   * @returns string of data formatted according to communication standard
   */
  std::string convert2Standard_(uint8_t id, std::vector<uint8_t> &data);

  /**
   * @brief Reads connection params from parameter server
   * @param[out] params returns connection parameters read from parameter server
   * @returns if OK empty optional, if error optional with error string
   */
  std::optional<std::string> readConnectionParams_(TCPParams &params);

  void listenData_();

 public:
  CommunicationServer();
  ~CommunicationServer();

  
  /**
   * @brief Handles the service request, sends messages over requested protocol
   * (TCP or Bluetooth)
   * @param req request handler
   * @param resp response handler
   * @returns true if succeeded, false otherwise
   */
  bool sendData(const std::shared_ptr<hal_interfaces::srv::Communication::Request> req,
                std::shared_ptr<hal_interfaces::srv::Communication::Response> resp);
  
};
}  // namespace HAL
