#ifndef __BRCM_JPEG_ENCODE_DEF_H__
#define __BRCM_JPEG_ENCODE_DEF_H__

#define MAX_WIDTH   5000
#define MAX_HEIGHT  5000
#define MAX_ENCODED (15*1024*1024)
#define MAX_DECODED (MAX_WIDTH*MAX_HEIGHT*4)

extern void brcmjpeg_encode_init(void);
extern void brcmjpeg_encode_deinit(void);
extern void brcmjpeg_encode(uint8_t* in, uint32_t w, uint32_t h, uint8_t* out, uint32_t* out_size);
extern void brcmjpeg_decode(uint8_t* img, uint32_t size, uint8_t* out, uint32_t outsize);

#endif /* !__BRCM_JPEG_ENCODE_DEF_H__ */
