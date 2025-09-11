# **Forklift Project for ELEX4699**
 
**Author:** Derek Wilson

**Group Members:** Derek Wilson, Kaveh Akhgarzhand

**Date:** 2025-09-10

## Background
This is a project repository for BCIT's ELEX 4699 electrical system design course. The code in this repository is raspberry-pi and opencv-based.

## Problem
The given challenge was for each group of 2 to design a robot to sort small 3D printed packages from one warehouse to another in a 46in x 46in arena. The design was left completely up to the team, with the only constraint being that the robot must fit into a 8in x 7in square in the beginning. Light sensors were used to monitor if the robot exceeded this limit. The robot could be ran manually or autonomously and had 2.5 minutes to sort as many of these packages as possible. In manual operation, the robot had to be driven by looking at the video feed from a mounted camera. In autonomous mode, the robot has to function without any intervention.

<img src="https://github.com/user-attachments/assets/be96a30a-d789-4f9d-b401-859ad6041bb7" 
     alt="Arena Image" 
     width="350">

## Our Design
We settled on a more "traditional" forklift design, focusing on manual operation.

<img src="https://github.com/user-attachments/assets/a3d3c733-558c-411d-92de-11037087ae12" 
     alt="Forklift Image" 
     width="350">

This design used 2 forks connected to a servo-controlled rack and pinion system to enable the fork to reach the first 2 levels of the warehouse. Given the time constraints, it was unlikely for us to be able to sort any boxes on the top level of the warehouse, therefore they were ignored. The mounted Raspberry Pi is powered by a standard 5V power bank typically used for smart phones, and the motors were being powered by a 9V battery attached to the back. 2 wheels were mounted at the front of the forklift, allowing for a sharp turn radius and more control. At the back of the forklift we mounted a solid peg since the driving surface was smooth enough to not need rear wheels. On the forklift is a Raspberry Pi 4 mounted inside the black and orange box, using a custom made PCB to interface with the motor driver, and the fork servo. The Raspberry Pi communicates with our laptop using TCP/IP for movement commands, and UDP for the camera feed. 

The entire assembly was modelled in SolidWorks and printed using a prusa 3D printer, with the exceptions of the Raspberry Pi itself, and the motor / wheels. The rack and pinion system's gear box allows the 180 degree rotation of the servo motor to linearly move the fork from slightly off the ground, to above the second level of the warehouse. 

## What we accomplished
During our most succesful run, we sorted 11 boxes from one warehouse to the next in the 2.5 minute time period. This attempt was not caught on video, but here is another demo run of the forklift with albeit, less succesful results:




## What I learned 
During this project, I deepened many different areas of the design process. Firstly, I took a deeper dive into SolidWorks and 3D design. I challenged myself to design the rack and pinion gear box, and learned about assemblies, simulation physics, and tolerances. In addition to this I learned first hand the advantages of 3D printing in the prototyping phase, especially the quick turnaround time to fabricate complex components. However, I also learned the limitations of 3D printing, especially with tolerances, material strength, and inaccuracies in small components. On the software side, I deepened my skills in C++ in Windows and Linux based systems, and became more familiar with the OpenCV library. I also learned the advantages of version control in the form of git and github, especially when working on different devices and operating systems. When building code, we used AI to help find potential inefficiences and bugs, helping making our final robot responsive, maneuverable, and quick. 

## What I would do next time 
In hindsight, I would add a second set of forks on top of the first set instead of using the rack and pinion system. Since the 3rd level of the warehouse didnt need to be reached, having the extra forks on top would eliminate the need for the extra moving components. I would also change the method of control, opting for a controller/joystick instead of WASD on the keyboard. This would allow for smoother control of the robot and would have made the runs smoother and quicker in return.


