//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_FRAMEBUFFER_H
#define ALTCORE_FRAMEBUFFER_H

struct FRAMEBUFFER_T;
typedef struct FRAMEBUFFER_T Framebuffer;

struct SWAPCHAIN_BUFFER_T;


Framebuffer* framebuffer_open_swapchain_buf(struct SWAPCHAIN_BUFFER_T* swapchain_buf);

void framebuffer_close(Framebuffer* framebuffer);

#endif //ALTCORE_FRAMEBUFFER_H
