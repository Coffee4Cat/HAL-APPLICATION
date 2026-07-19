#include "hal_communication/comm_server.h"
#include <sys/socket.h>

#include <iomanip>
#include <sstream>
#include <string>

namespace HAL {


TCPClient::TCPClient(const TCPParams &serverParams): serverParams_(serverParams) {}
TCPClient::~TCPClient() { close(sockFd_); }


auto TCPClient::initConnection_() -> std::optional<std::string> {
  if (sockFd_ > 0) {
      close(sockFd_);
      sockFd_ = -1;
  }

  // Initialize server address
  servAddr_.sin_family = AF_INET;
  servAddr_.sin_port = htons(serverParams_.port);
  if (inet_pton(AF_INET, serverParams_.IP.c_str(), &servAddr_.sin_addr) <= 0) {
      return "Invalid IP address";
  }

  // Create a new socket
  sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd_ < 0) {
      return "Cannot create the socket";
  }

  int status = connect(sockFd_, (struct sockaddr *)&servAddr_, sizeof(servAddr_));
  if (status < 0) {
      close(sockFd_);
      sockFd_ = -1;
      return "Cannot connect to the rover";
  }
  
  return {};
}


auto TCPClient::restartConnection() -> std::optional<std::string> {
  close(sockFd_);
  auto res = initConnection_();
  if (res.has_value()) {
    return res.value();
  }
  return {};
}


auto TCPClient::sendMessage(const std::string &data) -> std::optional<std::string> {
  
  if (sockFd_ < 0) {               // or <= -> check with HAL
      std::cout << "------------RESET REQUIRED-------" << std::endl;
      auto res = initConnection_();
      if (res.has_value()) {
          return res.value();
      }
  }
  
  ssize_t res = send(sockFd_, data.c_str(), data.length(), 0);
  if (res < 0) {                            
      close(sockFd_);
      sockFd_ = -1;
      std::cout << "------------CANNOT SEND STUFF-------" << std::endl;
      return "Cannot send data to the rover, connection lost. Reconnecting...";
  }

  return {};
}


std::string TCPClient::receiveMessage() {

  if (sockFd_ < 0) {               // or <= -> check with HAL
    auto res = initConnection_();
    if (res.has_value()) {
        return res.value();
    }
  }

  static std::string buffer_accumulator;  
  char buffer[19] = {0};                  
  ssize_t bytesRead = recv(sockFd_, buffer, sizeof(buffer), 0);

  if (bytesRead <= 0) {
      if (bytesRead == 0) {
          return "CONNECTION CLOSED";  
      } else {
          return "ERROR RECEIVING DATA";  
      }
  }

  
  buffer_accumulator += std::string(buffer, bytesRead);

  
  size_t start_pos = buffer_accumulator.find('#');
  if (start_pos == std::string::npos) {
      buffer_accumulator.clear();
      std::cout << "NO FRAME START FOUND" << std::endl;
      return "NO FRAME START FOUND";
  }

  
  if (buffer_accumulator.size() < start_pos + 19) {
      std::cout << "INCOMPLETE FRAME" << std::endl;
      return "INCOMPLETE FRAME";  
  }

  
  std::string complete_frame = buffer_accumulator.substr(start_pos, 19);
  buffer_accumulator.erase(0, start_pos + 19);

  return complete_frame;  

}









CommunicationServer::CommunicationServer(): Node("communication_server") {

  declare_parameter<std::string>(comm::CONST::PARAM_IP, "192.168.1.98");
  declare_parameter<int>(comm::CONST::PARAM_PORT, 8888);

  
  TCPParams params;
  auto result = readConnectionParams_(params);
  if (result.has_value()) {
    throw std::runtime_error((*result).c_str());
  }
  
  tcpHandler_ = TCPClient(params);
  auto response = tcpHandler_.restartConnection();
  RCLCPP_INFO(this->get_logger(), "%d",tcpHandler_.sockFd_);
  
  server_ = create_service<hal_interfaces::srv::Communication>(
      comm::CONST::SRV_NAME,
      std::bind(&CommunicationServer::sendData, this, std::placeholders::_1, std::placeholders::_2));
  param_cb_handle_ = this->add_on_set_parameters_callback(
      std::bind(&CommunicationServer::on_param_change, this, std::placeholders::_1));    
  RCLCPP_INFO(this->get_logger(), "Initialize communication server");
}

rcl_interfaces::msg::SetParametersResult CommunicationServer::on_param_change(const std::vector<rclcpp::Parameter> &params) {
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;
  bool changed = false;
  TCPParams new_params;
  
  // Read current params
  new_params.IP = this->get_parameter(comm::CONST::PARAM_IP).as_string();
  new_params.port = this->get_parameter(comm::CONST::PARAM_PORT).as_int();

  for (const auto &param : params) {
    if (param.get_name() == comm::CONST::PARAM_IP) {
      new_params.IP = param.as_string();
      changed = true;
    } else if (param.get_name() == comm::CONST::PARAM_PORT) {
      new_params.port = param.as_int();
      changed = true;
    }
  }

  if (changed) {
    RCLCPP_INFO(this->get_logger(), "Connection params changed. New IP: %s, Port: %d", new_params.IP.c_str(), new_params.port);
    tcpHandler_.setParams(new_params);
    auto res = tcpHandler_.restartConnection();
    
    if (res.has_value()) {
      RCLCPP_WARN(this->get_logger(), "Failed to reconnect: %s", res.value().c_str());
      result.successful = false;
      result.reason = res.value();
    } else {
      RCLCPP_INFO(this->get_logger(), "Reconnected successfully to %s:%d", new_params.IP.c_str(), new_params.port);
    }
  }
  return result;
}

CommunicationServer::~CommunicationServer() {
  running_ = false;
}

auto CommunicationServer::convert2Hex_(int dec_num) -> std::string {
  std::stringstream stream;
  stream << std::uppercase << std::hex << dec_num;
  std::string result(stream.str());
  return result;
}

auto CommunicationServer::convert2Standard_(uint8_t id,
                                            std::vector<uint8_t> &data)
    -> std::string {
  std::string result;
  result = "#";  // # starts the frame
  data.insert(data.begin(), id);
  for (auto element : data) {
    auto hex_str = convert2Hex_(element);
    if (hex_str.size() == 1) {
      // for one-positional hex numbers
      hex_str.insert(0, 1, '0');
    }
    if (hex_str.size() != 2) {
      // when single hex is longer/shorter than 2 signs
      throw std::runtime_error("Message conversion error");
    }
    result.append(hex_str);
  }
  // for fills remaining fields with 'x' according to standard
  for (int i = result.size(); i < comm::CONST ::COMMUNICATION_FRAME_SIZE; i++) {
    result.append("x");
  }
  RCLCPP_INFO(this->get_logger(), "Sendin': '%s'", result.c_str());
  return result;
}


auto CommunicationServer::readConnectionParams_(TCPParams &params) -> std::optional<std::string> {
  try {
    RCLCPP_INFO(this->get_logger(), "READING CONNECTION PARAMS");
    params.IP = get_parameter(comm::CONST::PARAM_IP).as_string();
    params.port = get_parameter(comm::CONST::PARAM_PORT).as_int();
    RCLCPP_INFO(this->get_logger(), "Params, IP: %s, %d", params.IP.c_str(), params.port);
  } catch (const rclcpp::ParameterTypeException &e) {
    return "Error reading parameters: " + std::string(e.what());
  }
  return {};
}

void CommunicationServer::listenData_(){
  // RCLCPP_INFO(this->get_logger(), "Received a message %s!",tcpHandler_.recvSockFd_);
  while (rclcpp::ok() && running_) {
    std::string message = tcpHandler_.receiveMessage();    
    RCLCPP_INFO(this->get_logger(), "Received a message %s!",message.c_str());
    // RCLCPP_INFO(this->get_logger(), "Recv %d!",tcpHandler_.recvSockFd_);
    if (message.size() > 5 && message[0] == '#') {
      // std::string message = std::string(message);
      hal_interfaces::msg::Transmission msg;
      int id = std::stoi(message.substr(1, 2), nullptr, 16);
      std::string data = message.substr(3, 16);
      msg.id = id;
      msg.data = data;
      RCLCPP_INFO(this->get_logger(), "ID: %d, DATA: %s", id, data.c_str());
      this->transmitter_->publish(msg);
    }
  }
}

bool CommunicationServer::sendData(
    const std::shared_ptr<hal_interfaces::srv::Communication::Request> req,
    std::shared_ptr<hal_interfaces::srv::Communication::Response> resp) {
      
  RCLCPP_INFO(this->get_logger(), "BEGIN SENDING DATA");
  
  if (req->comm_mode == req->TCPIP) {
  auto res = tcpHandler_.sendMessage(convert2Standard_(req->id, req->data));
    RCLCPP_INFO(this->get_logger(), "-------------GOT RESPONSE--------------");
    if (res.has_value()) {
      resp->success = false;
      resp->message = res.value();
      return true;
    }
  } else if (req->comm_mode == req->BLUETOOTH) {
    RCLCPP_INFO(this->get_logger(), "TRAP");
  } else {
    resp->success = false;
    resp->message = "No appropriate 'comm_mode' field in request";
    return false;
  }
  resp->success = false;
  return true;
}



}  // namespace HAL
