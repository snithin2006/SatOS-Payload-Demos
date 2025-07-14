#include <nvjpeg.h>
#include <iostream>

int main()
{
    nvjpegHandle_t handle;
    nvjpegCreateSimple(&handle);
    std::cout << "nvJPEG initialized successfully." << std::endl;
    nvjpegDestroy(handle);
    return 0;
}