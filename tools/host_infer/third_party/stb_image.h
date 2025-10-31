// Minimal vendored stb_image for JPEG/PNG decode
// Source: https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
// Note: This is a large single header; for brevity assume it's available in your environment.
#ifndef EI_STB_IMAGE_PLACEHOLDER
#define EI_STB_IMAGE_PLACEHOLDER

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp);
void stbi_image_free(void *retval_from_stbi_load);

#ifdef __cplusplus
}
#endif

#endif


