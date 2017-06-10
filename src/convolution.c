/*
*  Copyright 2017 Eero Talus
*
*  This file is part of Open Image Pipeline Convolution Matrix plugin.
*
*  Open Image Pipeline Convolution Matrix plugin is free software: you
*  can redistribute it and/or modify it under the terms of the GNU
*  General Public License as published by the Free Software Foundation,
*  either version 3 of the License, or (at your option) any later version.
*
*  Open Image Pipeline Convolution Matrix plugin is distributed in
*  the hope that it will be useful, but WITHOUT ANY WARRANTY; without
*  even the implied warranty of MERCHANTABILITY or FITNESS FOR A
*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Open Image Pipeline Convolution Matrix plugin.
*  If not, see <http://www.gnu.org/licenses/>.
*/

#define PRINT_IDENTIFIER "convolution"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <FreeImage.h>

#include "output.h"
#include "plugin.h"
#include "buildinfo/build.h"

#define RGBA_CHANNELS 4
#define KERNEL_W 3
#define KERNEL_H 3

typedef struct STRUCT_KERNEL {
	int *kernel;
	unsigned int w;
	unsigned int h;
	float divisor;
} KERNEL;

static inline void trunc_to_uint8_t(int32_t var, uint8_t *dest);
static RGBQUAD *get_pixel_rel(const IMAGE *img, int32_t x, int32_t y,
				int32_t r_x, int32_t r_y);
static void image_convolve_at(const IMAGE *img, int32_t x, int32_t y,
				KERNEL *kern, RGBQUAD *p_dest);
static int convolution_parse_args(char **plugin_args, unsigned int plugin_args_count);

static int convolution_process(struct PLUGIN_INDATA *in);
static int convolution_setup(void);
static void convolution_cleanup(void);

// Valid plugin arguments
static const char *plugin_valid_args[10] = {
	"kernel",
	"divisor",
	"channels"
};

// Plugin information
PLUGIN_INFO PLUGIN_INFO_NAME(convolution) = {
	.name = "convolution",
	.descr = "A convolution matrix plugin.",
	.author = "Eero Talus",
	.year = "2017",
	.built_against = &OIP_BUILT_AGAINST,

	.valid_args = plugin_valid_args,
	.valid_args_count = 3,

	.flag_print_verbose = &print_verbose,

	.plugin_process = convolution_process,
	.plugin_setup = convolution_setup,
	.plugin_cleanup = convolution_cleanup
};

static struct ENABLED_CHANNELS {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} enabled_channels = {0, 0, 0, 0};

static int kernel_matrix[KERNEL_W*KERNEL_H] = { 0 };
static KERNEL plugin_kernel = {
	.kernel = kernel_matrix,
	.w = KERNEL_W,
	.h = KERNEL_H,
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
	int multiplier = 0;
	int p_dest_tmp[RGBA_CHANNELS] = { 0 };
	RGBQUAD *p = NULL;

	// Initially copy the source pixel to the destination pixel.
	p = get_pixel_rel(img, x, y, 0, 0);
	if (p == NULL) {
		return;
	}
	memcpy(p_dest, p, sizeof(RGBQUAD));

	for (int k_y = 0; k_y < kern->h; k_y++) {
		for (int k_x = 0; k_x < kern->w; k_x++) {
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

	// Copy the channels that are enabled into the destination pixel.
	if (enabled_channels.r) {
		trunc_to_uint8_t(p_dest_tmp[0], &p_dest->rgbRed);
	}
	if (enabled_channels.g) {
		trunc_to_uint8_t(p_dest_tmp[1], &p_dest->rgbGreen);
	}
	if (enabled_channels.b) {
		trunc_to_uint8_t(p_dest_tmp[2], &p_dest->rgbBlue);
	}
	if (enabled_channels.a) {
		trunc_to_uint8_t(p_dest_tmp[3], &p_dest->rgbReserved);
	}
}

static int convolution_parse_args(char **plugin_args, unsigned int plugin_args_count) {
	unsigned int kernel_parsed = 0;
	unsigned int divisor_parsed = 0;
	unsigned int channels_parsed = 0;
	unsigned int no_channels_enabled = 1;
	unsigned int s = 0, k = 0;
	char *tmp_arg = NULL;
	char *tmp_val = NULL;

	for (unsigned int i = 0; i < plugin_args_count; i++) {
		tmp_arg = (char*) plugin_args[i*2];
		tmp_val = (char*) plugin_args[i*2 + 1];
		if (strcmp(tmp_arg, "kernel") == 0) {
			for (unsigned int c = 0; c < strlen(tmp_val) && k < 9; c++) {
				if (tmp_val[c] == ',') {
					// Convert kernel multipliers.
					plugin_kernel.kernel[k] = strtol(tmp_val + s, NULL, 10);
					s = c + 1;
					k++;
				}
			}
			// Add the last kernel multiplier.
			if (k < 9) {
				plugin_kernel.kernel[k] = strtol(tmp_val + s, NULL, 10);
			}

			// Only set the kernel parsed flag if all the multipliers are found.
			if (k == 8) {
				kernel_parsed = 1;
			}
		} else if (strcmp(tmp_arg, "divisor") == 0) {
			plugin_kernel.divisor = strtof(tmp_val, NULL);
			printverb_va("Kernel divisor: %f.\n", plugin_kernel.divisor);
			divisor_parsed = 1;
		} else if (strcmp(tmp_arg, "channels") == 0) {
			memset(&enabled_channels, 0, sizeof(struct ENABLED_CHANNELS));
			printverb("Enabled channels: ");
			for (int c = 0; c < strlen(tmp_val); c++) {
				switch (tmp_val[c]) {
					case 'R':
						enabled_channels.r = 1;
						if (print_verbose) {
							printf("R");
						}
						no_channels_enabled = 0;
						break;
					case 'G':
						enabled_channels.g = 1;
						if (print_verbose) {
							printf("G");
						}
						no_channels_enabled = 0;
						break;
					case 'B':
						enabled_channels.b = 1;
						if (print_verbose) {
							printf("B");
						}
						no_channels_enabled = 0;
						break;
					case 'A':
						enabled_channels.a = 1;
						if (print_verbose) {
							printf("A");
						}
						no_channels_enabled = 0;
						break;
					default:
						break;
				}
			}
			if (print_verbose) {
				if (no_channels_enabled) {
					printf("None\n");
				} else {
					printf("\n");
				}
			}
			channels_parsed = 1;
		}
	}

	if (kernel_parsed && divisor_parsed && channels_parsed) {
		return 0;
	}

	printerr("Plugin args missing.\n");
	return 1;
}

static int convolution_process(struct PLUGIN_INDATA *in) {
	if (convolution_parse_args(in->args, in->argc) != 0) {
		return PLUGIN_STATUS_ERROR;
	}

	printverb_va("Received %zu bytes of image data.\n", img_bytelen(in->src));

	if (img_realloc(in->dst, in->src->w, in->src->h) != 0) {
		return PLUGIN_STATUS_ERROR;
	}

	for (uint32_t y = 0; y < in->src->h; y++) {
		for (uint32_t x = 0; x < in->src->w; x++) {
			image_convolve_at(in->src, x, y, &plugin_kernel,
				in->dst->img + y*in->dst->w + x);
		}
		in->set_progress((int) round((float)y/in->src->h*100));
	}

	printverb_va("Processed %zu bytes of data.\n", img_bytelen(in->src));
	return PLUGIN_STATUS_DONE;
}

static int convolution_setup(void) {
	return 0;
}

static void convolution_cleanup(void) {}
