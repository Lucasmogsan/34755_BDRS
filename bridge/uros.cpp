/*
 * 
 ***************************************************************************
 *   Copyright (C) 2022 by DTU (Christian Andersen)                        *
 *   jca@elektro.dtu.dk                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Lesser General Public License for more details.                   *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "uros.h"
#include "std_msgs/String.h"
// #include "geometry_msgs/Pose.h"
// #include "geometry_msgs/Pose2D.h"
#include <nav_msgs/Odometry.h>
#include <tf2/LinearMath/Quaternion.h>
#include "main.h" 
#include "ubridge.h"
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <ros/master.h>

URos rosif;

void URos::setup()
{ // bridge source name
  setSourceID("ros");
  //
  ros::init(argcs, argvs, "bridge");
  rosOK = ros::master::check();
  if (rosOK)
    printf("# roscore is running OK\n");
  else
    printf("# roscore is not running\n");
  if (rosOK)
  {  // ROS stuff
    ros::NodeHandle n;
    nodeHandle = &n;
    // advertise message types
    bridge_raw_pub = nodeHandle->advertise<std_msgs::String>("/bridge_raw", 100);
    odom_pub = nodeHandle->advertise<nav_msgs::Odometry>("odom", 50);
  //   bridge_pose2d_pub = nodeHandle->advertise<geometry_msgs::Pose2D>("/pose2d", 100, false);
  }  
}

void URos::run()
{ // ensure ROS is listenin
  if (rosOK)
  {
    ros::spin();
    fprintf(stderr, "### ROS is no longer OK - shutting down ####\n");
//     quitBridge = true;
  }
  else
    printf("# ROS is NOT running\n");
}

bool URos::isActive()
{
  return rosOK;
}


void URos::publishPose(const char* msg)
{
  if (not rosOK)
    return;
  // timestamp as Unix timestamp
  ros::Time rost = ros::Time::now();
  // start at first parameter
  const char * p1= msg;
  t1 = strtof(p1, (char**)&p1); // Teensy time
  x = strtof(p1, (char**)&p1);
  y = strtof(p1, (char**)&p1);
  h = strtof(p1, (char**)&p1);
  if (poseNew)
  { // no old pose, so use the same
    x2 = x;
    y2 = y;
    h2 = h;
    dt = 1.0;
    poseNew = false;
  }
  else
    // dt is using the Teensy time
    dt = t1 - t2;
  double vx = (x - x2)/dt;
  double vy = (y - y2)/dt;
  double vh = (h - h2)/dt;
  //
  // debug
  if (false and n++ % 40 < 7)
  { // debug message
    printf("# %d URos::send pose: x=%g, y=%g, h=%g - %s", n, x, y, h, msg);
    printf("# %d URos::send vel :vx=%g,vy=%g,vh=%g - dt=%g\n", n, vx, vy, vh, dt);
  }
  // debug end
  //
  //odometry is in 6DOF, quaternion created from yaw
  tf2::Quaternion q;
  q.setRPY(0, 0, h);
  // quaternion is not the same as 
  geometry_msgs::Quaternion odom_quat;
  odom_quat.x = q.x();
  odom_quat.y = q.y();
  odom_quat.z = q.z();
  odom_quat.w = q.w();
  // odometry time as a ROS structure (sec, nanoSec)
  //
  // publish transform
  static tf2_ros::TransformBroadcaster br;
  geometry_msgs::TransformStamped trs;
  trs.header.stamp = rost;
  trs.header.frame_id = "odom";
  trs.child_frame_id = "base_link";
  trs.transform.translation.x = x;
  trs.transform.translation.y = y;
  trs.transform.translation.z = 0;
  trs.transform.rotation = odom_quat;
  br.sendTransform(trs);
  //
  // odometry status
  nav_msgs::Odometry odom;
  odom.header.stamp = rost;
  odom.header.frame_id = "odom";
  odom.child_frame_id = "base_link";
  //set the position
  odom.pose.pose.position.x = x;
  odom.pose.pose.position.y = y;
  odom.pose.pose.position.z = 0.0;
  odom.pose.pose.orientation = odom_quat;
  //set the velocity
  odom.twist.twist.linear.x = vx;
  odom.twist.twist.linear.y = vy;
  odom.twist.twist.angular.z = vh;
  //publish the message
  odom_pub.publish(odom);
  //
  t2 = t1;
  x2 = x;
  y2 = y;
  h2 = h;
}


void URos::sendString(const char* message, int msTimeout)
{
  const char * p1 = strchr(message, ':');
  p1++;
  if (strncmp(p1, "pose ", 5) == 0)
  {
    publishPose(p1 + 5);
  }
  else
  {
    std_msgs::String msg;
  //   printf("# URos::sendString: this is for ROS '%s' - handling is missing\n", message);
    msg.data = message;
    bridge_raw_pub.publish(msg);
  //   ROS_INFO("%s", msg.data.c_str());
  }
}

