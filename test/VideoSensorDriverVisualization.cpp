#include "VideoSensorDriverVisualization.h"

#undef e
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

VIDEO::VideoSensorDriverVisualization::VideoSensorDriverVisualization()
    : VisualizationBase()
    , mWebCamStatus(false)
{
    // Read in the INI settings
    worldReadIniSettings();
}

VIDEO::VideoSensorDriverVisualization::~VideoSensorDriverVisualization()
{
    // Clean-up the graphics resources
    cleanupFrame(mVideo);
}

VIDEO::VideoSensorDriverVisualization& VIDEO::VideoSensorDriverVisualization::getInstance()
{
    static VideoSensorDriverVisualization instance;
    return instance;
}

void VIDEO::VideoSensorDriverVisualization::initialize()
{
}

void VIDEO::VideoSensorDriverVisualization::setVideoFrame(const VisualizerImageFrame_t& forwardVideoFrame)
{
    updateFrame(mVideo, forwardVideoFrame);
}

void VIDEO::VideoSensorDriverVisualization::setWebCameraStatus(const bool& status)
{
    mWebCamStatus = status;
}

bool VIDEO::VideoSensorDriverVisualization::getWebCameraStatus() const
{
    return mWebCamStatus;
}

void VIDEO::VideoSensorDriverVisualization::worldReadIniSettings()
{
}

// World frame drawing functions
void VIDEO::VideoSensorDriverVisualization::worldFramePreDisplayFcn(bool demoMode)
{
    ImGui::Begin("Video Sensor Driver");
    ImGui::SetWindowFontScale(1.0F);

    std::string str = (mWebCamStatus == true) ? "FWD" : "Clip";

    if (ImGui::TreeNodeEx("Cameras:", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Draw forward video panel
        ImGui::Checkbox(str.c_str(), &mWorldFrameSettings.displayCamera);

        if (mWorldFrameSettings.displayCamera)
        {
            // Current day
            ImGui::TextColored(ImVec4(UTILITYColors::White.r, UTILITYColors::White.g, UTILITYColors::White.b, 1.0F),
                               "Day:");
            ImGui::SameLine(100);
            ImGui::TextColored(ImVec4{UTILITYColors::Green.r, UTILITYColors::Green.g, UTILITYColors::Green.b, 1.0f},
                               getDayAndTime().c_str());
        }

        ImGui::TreePop();
    }
    ImGui::End();
}

void VIDEO::VideoSensorDriverVisualization::initializeFrame(VideoData_t& videoData)
{
    // Create a OpenGL texture identifier
    glGenTextures(1, &videoData.imageTexture);

    // Set the initialization flag
    videoData.isInitialized = videoData.imageTexture > 0;
}

void VIDEO::VideoSensorDriverVisualization::localFrameDisplayFcn()
{
    if (mWorldFrameSettings.displayCamera)
    {
        std::string str = (mWebCamStatus == true) ? "webcam" : "avi";
        ImGui::Begin(str.c_str());
        ImGui::SetWindowFontScale(1.0F);
        drawFrame(mVideo);
        if (mVideo.isInitialized && mVideo.frame != nullptr)
        {
            const float32_t          colPos = 50.0F;
            const UTILITY::Color3f_t color  = UTILITYColors::Red;
            ImGui::TextColored(ImVec4(UTILITYColors::White.r, UTILITYColors::White.g, UTILITYColors::White.b, 1.0F),
                               "Time:");
            ImGui::SameLine(colPos);
            ImGui::TextColored(ImVec4(UTILITYColors::Yellow.r, UTILITYColors::Yellow.g, UTILITYColors::Yellow.b, 1.0F),
                               getTime().c_str());
        }
        ImGui::End();
    }
}

void VIDEO::VideoSensorDriverVisualization::updateFrame(VideoData_t& videoData, const VisualizerImageFrame_t& frame)
{
    // Scoped mutex lock
    std::lock_guard<std::mutex> dataLock(videoData.dataMutex);

    // Update the frame information
    videoData.frame = &frame;
}

UTILITY::Vector2f_t VIDEO::VideoSensorDriverVisualization::ImVec2ToVector2f(const ImVec2& vec)
{
    return {vec.x, vec.y};
}

std::array<UTILITY::Vector2f_t, 2U>
    VIDEO::VideoSensorDriverVisualization::computeCurrentImageSize(const VideoData_t& video)
{
    UTILITY::Vector2f_t contentMinPos = {};
    UTILITY::Vector2f_t imageSize     = {};

    if (video.frame != nullptr)
    {
        // Get the content window size
        UTILITY::Vector2f_t windowPos = UTILITY::Vector2f_t(ImVec2ToVector2f(ImGui::GetWindowPos()));
        contentMinPos = UTILITY::Vector2f_t(ImVec2ToVector2f(ImGui::GetWindowContentRegionMin())) + windowPos;
        UTILITY::Vector2f_t contentMaxPos =
            UTILITY::Vector2f_t(ImVec2ToVector2f(ImGui::GetWindowContentRegionMax())) + windowPos;

        // Compute the window aspect ratio
        UTILITY::Vector2f_t windowSize        = contentMaxPos - contentMinPos;
        float32_t           windowAspectRatio = windowSize.x() / windowSize.y();

        // Compute the image aspect ratio
        float32_t imageAspectRatio =
            static_cast<float32_t>(video.frame->width) / static_cast<float32_t>(video.frame->height);

        // Truncate the positions to maintain image scaling
        imageSize = (windowAspectRatio > imageAspectRatio)
                        ? UTILITY::Vector2f_t(imageAspectRatio * windowSize.y(), windowSize.y())
                        : UTILITY::Vector2f_t(windowSize.x(), windowSize.x() / imageAspectRatio);
    }
    return {contentMinPos, imageSize};
}

ImVec2 VIDEO::VideoSensorDriverVisualization::formatToImVec2(const UTILITY::Vector2f_t& vec)
{
    return {vec.x(), vec.y()};
}

void VIDEO::VideoSensorDriverVisualization::drawFrame(VideoData_t& videoData)
{
    // Scoped mutex lock
    std::lock_guard<std::mutex> dataLock(videoData.dataMutex);

    // Only draw valid frames
    if (videoData.isInitialized && videoData.frame != nullptr)
    {
        // For convenience, get a reference to the frame
        const VisualizerImageFrame_t& frame = *videoData.frame;

        // Scoped mutex lock for the frame
        std::lock_guard<std::mutex> dataLock(frame.lock);

        // Bind image texture
        glBindTexture(GL_TEXTURE_2D, videoData.imageTexture);

        // Set filtering parameters for display
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Copy the current information to the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.width, frame.height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels);

        // plot the image
        ImGui::GetWindowDrawList()->AddImage(
            (void*) videoData.imageTexture,
            formatToImVec2(computeCurrentImageSize(mVideo).at(0)),
            formatToImVec2(
                UTILITY::Vector2f_t(computeCurrentImageSize(mVideo).at(0) + computeCurrentImageSize(mVideo).at(1))));

        // Unbind image texture by restoring default
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        // Attempt to initialize the required textures
        initializeFrame(videoData);

        // Display unavailable message
        ImGui::Text("Video unavailable");
    }
}

void VIDEO::VideoSensorDriverVisualization::cleanupFrame(VideoData_t& videoData)
{
    // Delete the texture
    glDeleteTextures(1, &videoData.imageTexture);
}

// world to pixel coordinate transformation
// WorldCoordinates -> Cam coordinates - > Film coordinates  -> Pixel coordinates
// shear — Skew coefficient, which is non zero if the image axes are not perpendicular.
// shear  = fx tanα, where α is pixel skew
UTILITY::Vector2f_t
    VIDEO::VideoSensorDriverVisualization::worldToPixelCoordinates(const UTILITY::Vector3f_t& position_m,
                                                                   const UTILITY::Vector2f_t& principalPoint_pixel,
                                                                   const UTILITY::Vector3f_t& cameraPosition_m,
                                                                   float32_t                  focalLength_m,
                                                                   float32_t                  pixelSize_m,
                                                                   float32_t                  roll_rad,
                                                                   float32_t                  pitch_rad,
                                                                   float32_t                  yaw_rad,
                                                                   float32_t                  alpha)
{

    // focal length in x and y directions (unit in pixels)
    const float32_t fx = (focalLength_m / pixelSize_m);
    const float32_t fy = fx;

    // Skew coefficient, which is non-zero if the image axes are not perpendicular.
    const float32_t shear = fx * std::tan(UTILITY::degreesToRadians(alpha));

    // Convert world coordinates to camera coordinates, scale down, plus rotate translation(rigid body transformation)
    boost::numeric::ublas::matrix<float32_t> rRoll(3, 3);
    rRoll(0, 0) = 1.0F;
    rRoll(0, 1) = 0.0F;
    rRoll(0, 2) = 0.0F;
    rRoll(1, 0) = 0.0F;
    rRoll(1, 1) = std::cos(roll_rad);
    rRoll(1, 2) = std::sin(roll_rad);
    rRoll(2, 0) = 0.0F;
    rRoll(2, 1) = -std::sin(roll_rad);
    rRoll(2, 2) = std::cos(roll_rad);

    // rotates around the y-axis
    boost::numeric::ublas::matrix<float32_t> rPitch(3, 3);
    rPitch(0, 0) = std::cos(pitch_rad);
    rPitch(0, 1) = 0.0F;
    rPitch(0, 2) = -std::sin(pitch_rad);
    rPitch(1, 0) = 0.0F;
    rPitch(1, 1) = 1.0F;
    rPitch(1, 2) = 0.0F;
    rPitch(2, 0) = std::sin(pitch_rad);
    rPitch(2, 1) = 0.0F;
    rPitch(2, 2) = std::cos(pitch_rad);

    // rotates around the z-axis
    boost::numeric::ublas::matrix<float32_t> rYaw(3, 3);
    rYaw(0, 0) = std::cos(yaw_rad);
    rYaw(0, 1) = std::sin(yaw_rad);
    rYaw(0, 2) = 0.0F;
    rYaw(1, 0) = -std::sin(yaw_rad);
    rYaw(1, 1) = std::cos(yaw_rad);
    rYaw(1, 2) = 0.0F;
    rYaw(2, 0) = 0.0F;
    rYaw(2, 1) = 0.0F;
    rYaw(2, 2) = 1.0F;

    // rotation matrix
    boost::numeric::ublas::matrix<float32_t> rYawPitch(3, 3);
    rYawPitch    = prod(rYaw, rPitch);
    const auto r = prod(rYawPitch, rRoll);

    // A rigid body change (rotation + translation) into a new coordinate axis
    boost::numeric::ublas::matrix<float32_t> extrinsicParameters(3, 4);
    extrinsicParameters(0, 0) = r(0, 0);
    extrinsicParameters(0, 1) = r(0, 1);
    extrinsicParameters(0, 2) = r(0, 2);
    extrinsicParameters(0, 3) = cameraPosition_m.x();
    extrinsicParameters(1, 0) = r(1, 0);
    extrinsicParameters(1, 1) = r(1, 1);
    extrinsicParameters(1, 2) = r(1, 2);
    extrinsicParameters(1, 3) = cameraPosition_m.y();
    extrinsicParameters(2, 0) = r(2, 0);
    extrinsicParameters(2, 1) = r(2, 1);
    extrinsicParameters(2, 2) = r(2, 2);
    extrinsicParameters(2, 3) = cameraPosition_m.z();

    // Discrete sampling of image coordinates
    boost::numeric::ublas::matrix<float32_t> intrinsicParameters(3, 3);
    intrinsicParameters(0, 0) = fx;
    intrinsicParameters(0, 1) = shear;
    intrinsicParameters(0, 2) = principalPoint_pixel.x();
    intrinsicParameters(1, 0) = 0.0F;
    intrinsicParameters(1, 1) = fy;
    intrinsicParameters(1, 2) = principalPoint_pixel.y();
    intrinsicParameters(2, 0) = 0.0F;
    intrinsicParameters(2, 1) = 0.0F;
    intrinsicParameters(2, 2) = 1.0F;

    // camera Matrix
    const auto cameraMatrix = prod(intrinsicParameters, extrinsicParameters);

    boost::numeric::ublas::vector<float32_t> worldPosInCamCoordinate(4);
    worldPosInCamCoordinate(0) = position_m.x();
    worldPosInCamCoordinate(1) = position_m.y();
    worldPosInCamCoordinate(2) = position_m.z();
    worldPosInCamCoordinate(3) = 1.0F;

    // pixel coordinate
    const auto pixelCoordinatePoint = prod(cameraMatrix, worldPosInCamCoordinate);

    return UTILITY::Vector2f_t(pixelCoordinatePoint(0) / pixelCoordinatePoint(2),
                               pixelCoordinatePoint(1) / pixelCoordinatePoint(2));
}

UTILITY::Vector3f_t VIDEO::VideoSensorDriverVisualization::worldToCameraCoordinate(const UTILITY::Vector3f_t& worldPos)
{
    return UTILITY::Vector3f_t(worldPos.y(), worldPos.z(), worldPos.x());
}

std::vector<UTILITY::Vector2f_t>
    VIDEO::VideoSensorDriverVisualization::polyLineClipping(const std::vector<UTILITY::Vector2f_t>& lines1,
                                                            const std::vector<UTILITY::Vector2f_t>& lines2)
{
    std::vector<UTILITY::Vector2f_t> internalLine;
    for (const auto& l : lines1)
    {
        if (internalLine.size() < lines1.size() - 2) // Avoid last 2 elements
        {
            internalLine.push_back(l);
        }
    }
    for (int32_t i = lines2.size() - 3; i > 0; i--) // Avoid last 3 elements
    {
        internalLine.push_back(lines2.at(i));
    }
    internalLine.push_back(lines1.at(0));
    return internalLine;
}

void VIDEO::VideoSensorDriverVisualization::getWebCamFrame(const cv::Mat& frame)
{
    auto iplFrame                 = cvIplImage(frame);
    mLibVideoIOFrame.width        = iplFrame.width;
    mLibVideoIOFrame.height       = iplFrame.height;
    mLibVideoIOFrame.stride       = (iplFrame.width + 3) & 0xfffffffc;
    mLibVideoIOFrame.pixels       = (uint8_t*) iplFrame.imageData;
    mLibVideoIOFrame.type         = VIDEO::LVIO_RGB_8BPP;
    mLibVideoIOFrame.decodeStatus = VIDEO::LVIO_SUCCESS;
    setVideoFrame(mLibVideoIOFrame);
    mWebCamOn = true;
}

inline int VIDEO::VideoSensorDriverVisualization::pixelSaturate(int v, int minimum, int maximum)
{
    if (v < minimum)
    {
        return minimum;
    }
    if (v > maximum)
    {
        return maximum;
    }
    return v;
}

uint8_t* VIDEO::VideoSensorDriverVisualization::processRGB(const uint8_t* Yo,
                                                           const uint8_t* Uo,
                                                           const uint8_t* Vo,
                                                           uint32_t       cols,
                                                           uint32_t       rows,
                                                           uint32_t       strideOut,
                                                           uint32_t       strideY,
                                                           uint32_t       strideU,
                                                           uint32_t       strideV)
{
    // YUV-->RGB conversion factors for JPEG data (from https://en.wikipedia.org/wiki/YCbCr#ITU-R_BT.601_conversion)
    const int K1 = int(1.402f * (1 << 16));
    const int K2 = int(0.714f * (1 << 16));
    const int K3 = int(0.334f * (1 << 16));
    const int K4 = int(1.772f * (1 << 16));

    uint8_t* rgb = (uint8_t*) malloc(strideOut * rows);
    if (rgb == NULL)
    {
        char msg[256];
        sprintf_s(&msg[0],
                  sizeof(msg),
                  "Out of memory error in libVideoIO. Could not allocate for size: {width:%u,height:%u}",
                  strideOut / 3,
                  rows);
        throw std::runtime_error(static_cast<const char*>(&msg[0]));
    }

    // convert pixel values 4-at-a-time from YUV to RGB
    for (int y = 0; y < (int) (rows - 1); y += 2)
    {
        const uint8_t* Y1 = Yo + strideY * y;
        const uint8_t* Y2 = Yo + strideY * (y + 1);
        const uint8_t* U  = Uo + strideU * y / 2;
        const uint8_t* V  = Vo + strideV * y / 2;

        uint8_t* out_ptr1 = rgb + y * strideOut;
        uint8_t* out_ptr2 = rgb + (y + 1) * strideOut;

        for (int x = 0; x < (int) (cols - 1); x += 2)
        {
            int8_t uf = *U - 128;
            int8_t vf = *V - 128;

            int R = *Y1 + (K1 * vf >> 16);
            int G = *Y1 - (K2 * vf >> 16) - (K3 * uf >> 16);
            int B = *Y1 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr1++ = (uint8_t) R;
            *out_ptr1++ = (uint8_t) G;
            *out_ptr1++ = (uint8_t) B;

            Y1++;
            R = *Y1 + (K1 * vf >> 16);
            G = *Y1 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y1 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr1++ = (uint8_t) R;
            *out_ptr1++ = (uint8_t) G;
            *out_ptr1++ = (uint8_t) B;

            R = *Y2 + (K1 * vf >> 16);
            G = *Y2 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y2 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr2++ = (uint8_t) R;
            *out_ptr2++ = (uint8_t) G;
            *out_ptr2++ = (uint8_t) B;
            Y2++;
            R = *Y2 + (K1 * vf >> 16);
            G = *Y2 - (K2 * vf >> 16) - (K3 * uf >> 16);
            B = *Y2 + (K4 * uf >> 16);

            pixelSaturate(R, 0, 255);
            pixelSaturate(G, 0, 255);
            pixelSaturate(B, 0, 255);

            *out_ptr2++ = (uint8_t) R;
            *out_ptr2++ = (uint8_t) G;
            *out_ptr2++ = (uint8_t) B;

            Y1++;
            Y2++;
            U++;
            V++;
        }
    }
    return rgb;
}

void VIDEO::VideoSensorDriverVisualization::getRGBFrame(int      width,
                                                        int      height,
                                                        uint8_t* data1,
                                                        uint8_t* data2,
                                                        uint8_t* data3,
                                                        int*     lineSize,
                                                        int      displayPictureNumber,
                                                        float    samplingTime)
{
    // YUV420P::VideoIOFrame_t libVideoIOFrame = {};
    mLibVideoIOFrame.width  = width;
    mLibVideoIOFrame.height = height;
    mLibVideoIOFrame.stride = (width + 3) & 0xfffffffc; // round to multiple of 4

    mLibVideoIOFrame.pixels = processRGB(data1 /* Y - Input */,
                                         data2 /* U - Input */,
                                         data3 /* V - Input */,
                                         mLibVideoIOFrame.width,
                                         mLibVideoIOFrame.height,
                                         mLibVideoIOFrame.stride * 3, // need stride value in bytes here
                                         lineSize[0],
                                         lineSize[1],
                                         lineSize[2]);

    mLibVideoIOFrame.type         = VIDEO::LVIO_RGB_8BPP;
    mLibVideoIOFrame.decodeStatus = VIDEO::LVIO_SUCCESS;
    mLibVideoIOFrame.frameNumber  = displayPictureNumber;
    mLibVideoIOFrame.fps          = samplingTime;

    setVideoFrame(mLibVideoIOFrame);
}

std::string VIDEO::VideoSensorDriverVisualization::getTime()
{
    auto        today = std::chrono::system_clock::now();
    auto        ptr   = std::chrono::system_clock::to_time_t(today);
    auto        tm    = localtime(&ptr);
    std::string strLocalTime =
        std::to_string(tm->tm_hour) + ":" + std::to_string(tm->tm_min) + ":" + std::to_string(tm->tm_sec);
    return strLocalTime;
}

std::string VIDEO::VideoSensorDriverVisualization::getDayAndTime()
{
    auto        today = std::chrono::system_clock::now();
    auto        ptr   = std::chrono::system_clock::to_time_t(today);
    std::string strDayAndTime(ctime(&ptr));
    return strDayAndTime;
}
