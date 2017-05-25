#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <FreeImage.h>
#include "plugin.h"

#define RGBA_CHANNELS 4

typedef struct STRUCT_KERNEL {
	int16_t *kernel;
	uint16_t w;
	uint16_t h;
	float divisor;
} KERNEL;

static inline void trunc_to_uint8_t(int32_t var, uint8_t *dest);
static RGBQUAD *get_pixel_rel(const IMAGE *img, int32_t x, int32_t y, int32_t r_x, int32_t r_y);
static void image_convolve_at(const IMAGE *img, int32_t x, int32_t y, KERNEL *kern, RGBQUAD *p_dest);
static void image_convolve(const IMAGE *img, IMAGE *img_dest, KERNEL *kern);
static int convolution_parse_args(const char **plugin_args, unsigned int plugin_args_count);
int convolution_process(const IMAGE *img, IMAGE *img_dest,
			const char **plugin_args,
			unsigned int plugin_args_count);
int convolution_setup(void);
void convolution_cleanup(void);

// Define valid plugin arguments.
static char *plugin_valid_args[10] = {
	"kernel",
	"divisor",
	"channels"
};

// Define plugin parameters.
PLUGIN_INFO PLUGIN_INFO_NAME(convolution) = {
	.name = "convolution",
	.descr = "A convolution matrix plugin.",
	.author = "Eero Talus",
	.year = "2017",
	.valid_args = plugin_valid_args,
	.valid_args_count = 3,
	.plugin_process = convolution_process,
	.plugin_setup = convolution_setup,
	.plugin_cleanup = convolution_cleanup
};

// Define convolution kernel.
static int16_t plugin_kernel_array[9] = { 0 };
static KERNEL plugin_kernel = {
	.kernel = plugin_kernel_array,
	.w = 3,
	.h = 3,
	.divisor = 1.0f
};

static inline void trunc_to_uint8_t(int32_t var, uint8_t *dest) {
	/*
	* Truncate a value so that if it is negative the result is 0
	* and if its larger than 255 the result is 255. Otherwise the
	* result is the value itself.
	*/
	if (var < 0) {
		*dest = 0;
	} else if (var > 255) {
		*dest = 255;
	} else {
		*dest = (uint8_t) var;
	}
}

static RGBQUAD *get_pixel_rel(const IMAGE *img, int32_t x, int32_t y, int32_t r_x, int32_t r_y) {
	int32_t n_x = x + r_x;
	int32_t n_y = y + r_y;

	if (n_x < 0) {
		if (n_y < 0) {
			// NW corner.
			return img->img;
		} else if (n_y >= img->h){
			// SW corner.
			return img->img + (img->h - 1)*img->w;
		} else {
			// Left side.
			return img->img + n_y*img->w;
		}
	} else if (n_x >= img->w) {
		if (n_y < 0) {
			// NE corner.
			return img->img + img->w - 1;
		} else if (n_y >= img->h) {
			// SE corner.
			return img->img + img->h*img->w - 1;
		} else {
			// Right side.
			return img->img + n_y*img->w + img->w - 1;
		}
	} else {
		if (n_y < 0) {
			// Top.
			return img->img + n_x;
		} else if (n_y >= img->h) {
			// Bottom.
			return img->img + (img->h - 1)*img->w + n_x;
		} else {
			// Normal.
			return img->img + n_y*img->w + n_x;
		}
	}
	return NULL;
}

static void image_convolve_at(const IMAGE *img, int32_t x, int32_t y, KERNEL *kern, RGBQUAD *p_dest) {
	int16_t multiplier = 0;
	int32_t p_dest_tmp[RGBA_CHANNELS] = { 0 };
	RGBQUAD *p = NULL;

	for (int16_t k_y = 0; k_y < kern->h; k_y++) {
		for (int16_t k_x = 0; k_x < kern->w; k_x++) {
			multiplier = kern->kernel[k_y*kern->w + k_x];
			if (multiplier == 0) {
				continue;
			}

			p = get_pixel_rel(img, x, y, k_x - 1, k_y - 1);
			if (p != NULL) {
				p_dest_tmp[0] += multiplier*p->rgbRed;
				p_dest_tmp[1] += multiplier*p->rgbGreen;
				p_dest_tmp[2] += multiplier*p->rgbBlue;
				p_dest_tmp[3] += multiplier*p->rgbReserved;
			}
		}
	}

	if (kern->divisor != 0.0f && kern->divisor != 1.0f) {
		p_dest_tmp[0] = round(p_dest_tmp[0]/kern->divisor);
		p_dest_tmp[1] = round(p_dest_tmp[1]/kern->divisor);
		p_dest_tmp[2] = round(p_dest_tmp[2]/kern->divisor);
		p_dest_tmp[3] = round(p_dest_tmp[3]/kern->divisor);
	}

	trunc_to_uint8_t(p_dest_tmp[0], &p_dest->rgbRed);
	trunc_to_uint8_t(p_dest_tmp[1], &p_dest->rgbGreen);
	trunc_to_uint8_t(p_dest_tmp[2], &p_dest->rgbBlue);
	trunc_to_uint8_t(p_dest_tmp[3], &p_dest->rgbReserved);
	//printf("(%i, %i, %i, %i)\n", p_dest->rgbRed, p_dest->rgbGreen, p_dest->rgbBlue, p_dest->rgbReserved);
}

static void image_convolve(const IMAGE *img, IMAGE *img_dest, KERNEL *kern) {
	for (uint32_t y = 0; y < img->h; y++) {
		for (uint32_t x = 0; x < img->w; x++) {
			image_convolve_at(img, x, y, kern,
				img_dest->img + y*img_dest->w + x);
		}
	}
}

static int convolution_parse_args(const char **plugin_args, unsigned int plugin_args_count) {
	uint8_t kernel_parsed = 0;
	uint8_t divisor_parsed = 0;
	uint8_t c_s = 0, k = 0;

	printf("convolution: Parsing plugin args.\n");
	for (unsigned int i = 0; i < plugin_args_count; i++) {
		printf("convolution: ARG %i => %s: %s\n", i, plugin_args[i*2], plugin_args[i*2 + 1]);
		if (strcmp(plugin_args[i*2], "kernel") == 0) {
			for (unsigned int c_e = 0; c_e < strlen(plugin_args[i*2 + 1]); c_e++) {
				if (plugin_args[i*2 + 1][c_e] == ',') {
					// Add the kernel multipliers one by one.
					plugin_kernel.kernel[k] = strtol(plugin_args[i*2 + 1] + c_s, NULL, 10);
					c_s = c_e + 1;

					k++;
					if (k >= 9) {
						break;
					}
				}
			}
			// Add the last kernel multiplier.
			if (k < 9) {
				plugin_kernel.kernel[k] = strtol(plugin_args[i*2 + 1] + c_s, NULL, 10);
			}

			// Only set the kernel parsed flag if all the multipliers are found.
			if (k == 8) {
				kernel_parsed = 1;
			}
		} else if (strcmp(plugin_args[i*2], "divisor") == 0) {
			plugin_kernel.divisor = strtof(plugin_args[i*2 + 1], NULL);
			divisor_parsed = 1;
		}
	}

	if (kernel_parsed == 1 && divisor_parsed == 1) {
		printf("convolution: Plugin args parsed.\n");
		return 0;
	}

	printf("convolution: Failed to parse plugin args.\n");
	return 1;
}

int convolution_process(const IMAGE *img, IMAGE *img_dest,
			const char **plugin_args,
			unsigned int plugin_args_count) {
	if (convolution_parse_args(plugin_args, plugin_args_count) != 0) {
		return 1;
	}
	printf("convolution: Received %i bytes of image data.\n", img_bytelen(img));
	if (img_realloc(img_dest, img->w, img->h) != 0) {
		return 1;
	}
	image_convolve(img, img_dest, &plugin_kernel);
	printf("convolution: Processed %i bytes of data.\n", img_bytelen(img));
	return 0;
}

int convolution_setup(void) {
	return 0;
}

void convolution_cleanup(void) {

}
