# This branch provides an ImGui AVI and Web-Cam player.

Avi-formatted files are decoded using the H264 decoder, thus the raw image data can be exploited to implement imaging algorithms. The webcam is accessed using the OpenCV library, but the data is housed in a general raw data format (which can be visualized through ImGui). Webcam images are tracked using a Neural Network Model. 

# Prerequisite (Windows or Unix):
* Conan
* CMake (latest)
* Visual Studio 2022 (Win)

# Build (Window):
* Run **run_debug.bat** or **run_release.bat**

# Webcam Example:
![image](https://github.com/rajiv1977/conan-imgui-visualizer/assets/16018587/5dec6a4c-a5c5-4df1-9e0c-65168e69c1bf)

# Avi Example:
![image](https://github.com/rajiv1977/conan-imgui-visualizer/assets/16018587/1bf97e74-d334-4e60-b413-f8f34080ab3b)

