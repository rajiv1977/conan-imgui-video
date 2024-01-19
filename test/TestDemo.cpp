#include "ImGuiVis.h"

int main(void)
{
    std::string camInput = "webcam";
    // std::string               camInput = "file_example_AVI_1920_2_3MG.avi";
    std::shared_ptr<ImGuiVis> run(new ImGuiVis(camInput));
    return EXIT_SUCCESS;
}
