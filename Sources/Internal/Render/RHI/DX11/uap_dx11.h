#ifndef __DAVAFRAMEWORK_UAP_DX11__
#define __DAVAFRAMEWORK_UAP_DX11__

void init_device_and_swapchain_uap(void* panel);
void resize_swapchain(int32 width, int32 height);
void get_device_description(char* dst);

#endif