#include <uWS/uWS.h>
#include <iostream>
#include "json.hpp"
#include "PID.h"
#include <math.h>

// for convenience
using json = nlohmann::json;

// For converting back and forth between radians and degrees.
constexpr double pi() { return M_PI; }
double deg2rad(double x) { return x * pi() / 180; }
double rad2deg(double x) { return x * 180 / pi(); }

// Checks if the SocketIO event has JSON data.
// If there is data the JSON object in string format will be returned,
// else the empty string "" will be returned.
std::string hasData(std::string s) {
  auto found_null = s.find("null");
  auto b1 = s.find_first_of("[");
  auto b2 = s.find_last_of("]");
  if (found_null != std::string::npos) {
    return "";
  }
  else if (b1 != std::string::npos && b2 != std::string::npos) {
    return s.substr(b1, b2 - b1 + 1);
  }
  return "";
}


int main()
{
  uWS::Hub h;

  PID steer_pid;
  PID throttle_pid;
  // TODO: Initialize the pid variable.
  //Kp, Ki, Kd
  // pid.Init(.4,0.0,1.5);
  steer_pid.Init(.2, 0.0001, 3.0);
  throttle_pid.Init(.5, 0.0, .01);
  h.onMessage([&steer_pid, &throttle_pid](uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length, uWS::OpCode opCode) {
    // "42" at the start of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event
    if (length && length > 2 && data[0] == '4' && data[1] == '2')
    {
      auto s = hasData(std::string(data).substr(0, length));
      if (s != "") {
        auto j = json::parse(s);
        std::string event = j[0].get<std::string>();
        if (event == "telemetry") {
          // j[1] is the data JSON object
          double cte = std::stod(j[1]["cte"].get<std::string>());
          double speed = std::stod(j[1]["speed"].get<std::string>());
          double angle = std::stod(j[1]["steering_angle"].get<std::string>());
          double steer_value;
          /*
          * TODO: Calcuate steering value here, remember the steering value is
          * [-1, 1].
          * NOTE: Feel free to play around with the throttle and speed. Maybe use
          * another PID controller to control the speed!
          */
          steer_pid.UpdateError(cte);
          steer_value = -steer_pid.Kp * steer_pid.p_error - steer_pid.Kd * steer_pid.d_error - steer_pid.Ki * steer_pid.i_error;
         
          throttle_pid.UpdateError(cte);
          double throttle2 =  0.6 -throttle_pid.Kp * throttle_pid.p_error - throttle_pid.Kd * throttle_pid.d_error - throttle_pid.Ki * throttle_pid.i_error;
          double throttle = 0.3;

          double max_angle= M_PI/4;
          // double sum = std::accumulate(steer_last_5.begin(), steer_last_5.end(), 0.0);
          // // if(step>200){
          //   if (counter == 5)
          //   {
          //     double avg_steer = sum/steer_last_5.size();
          //     double min_steer = *std::min_element(steer_last_5.begin(), steer_last_5.end());
          //     double max_steer = *std::max_element(steer_last_5.begin(), steer_last_5.end());
          std::cout << "FABS CTE: " << fabs(cte) << " Steering Value: " << fabs(steer_value) << std::endl;
          if( ((fabs(steer_value) > 0.2 || fabs(cte)> 0.2) && throttle2> 0.4)){
             // steer_value = 1;
             throttle2 = 0.2 ;
             std::cout<<"XXXXX"<<std::endl;
           }

          //      std::cout<<"*** avg_steer "<<steer_value<<std::endl;
          //    }
          // }

          // DEBUG
          std::cout << "CTE: " << cte << " Steering Value: " << steer_value << std::endl;
          std::cout << " throttle Value: " << throttle2 << std::endl;
          json msgJson;
          msgJson["steering_angle"] = steer_value;
          msgJson["throttle"] = throttle2;
          auto msg = "42[\"steer\"," + msgJson.dump() + "]";
          std::cout << msg << std::endl;
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
        }
      } else {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }
  });

  // We don't need this since we're not using HTTP but if it's removed the program
  // doesn't compile :-(
  h.onHttpRequest([](uWS::HttpResponse *res, uWS::HttpRequest req, char *data, size_t, size_t) {
    const std::string s = "<h1>Hello world!</h1>";
    if (req.getUrl().valueLength == 1)
    {
      res->end(s.data(), s.length());
    }
    else
    {
      // i guess this should be done more gracefully?
      res->end(nullptr, 0);
    }
  });

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code, char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port))
  {
    std::cout << "Listening to port " << port << std::endl;
  }
  else
  {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  h.run();
}
