// tga2jpg.cpp - jpge/jpgd example command line app.


// Note: jpge.cpp/h and jpgd.cpp/h are completely standalone, i.e. they do not have any dependencies to each other.
#include "jpge.h"
#include "jpgd.h"
#define STB_IMAGE_IMPLEMENTATION
//#define STBI_NO_SIMD
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "timer.h"
#include <ctype.h>

typedef unsigned int uint;
typedef unsigned char uint8;

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#else
#define strcpy_s(d, c, s) strcpy(d, s)
#endif

static int print_usage()
{
	printf("Usage: jpge [options] <source_file> <dest_file> <quality_factor>\n");
	printf("\nRequired parameters (must follow options):\n");
	printf("source_file: Source image file, in any format stb_image.c supports.\n");
	printf("dest_file: Destination JPEG file.\n");
	printf("quality_factor: 1-100, higher=better (only needed in compression mode)\n");
	printf("\nDefault mode compresses source_file to dest_file. Alternate modes:\n");
	printf("-x: Exhaustive compression test (only needs source_file)\n");
	printf("-d: Test jpgd.h. source_file must be JPEG, and dest_file must be .TGA\n");
	printf("\nOptions supported in all modes:\n");
	printf("-glogfilename.txt: Append output to log file\n");
	printf("\nOptions supported in compression mode (the default):\n");
	printf("-o: Enable optimized Huffman tables (slower, but smaller files)\n");
	printf("-luma: Output Y-only image\n");
	printf("-h1v1, -h2v1, -h2v2: Chroma subsampling (default is either Y-only or H2V2)\n");
	printf("-m: Test mem to mem compression (instead of mem to file)\n");
	printf("-wfilename.tga: Write decompressed image to filename.tga\n");
	printf("-s: Use stb_image.h to decompress JPEG image, instead of jpgd.cpp\n");
	printf("-q: Use traditional JPEG Annex K quantization tables, instead of mozjpeg's default tables\n");
	printf("-no_simd: Don't use SIMD instructions\n");
	printf("-box_filtering: Use box filtering for chroma, instead of linear (decompression only)\n");
	printf("\nExample usages:\n");
	printf("Test compression: jpge orig.png comp.jpg 90\n");
	printf("Test decompression: jpge -d comp.jpg uncomp.tga\n");
	printf("Exhaustively test compressor: jpge -x orig.png\n");

	return EXIT_FAILURE;
}

static char s_log_filename[256];

static void log_printf(const char* pMsg, ...)
{
	va_list args;

	va_start(args, pMsg);
	char buf[2048];
	vsnprintf(buf, sizeof(buf) - 1, pMsg, args);
	buf[sizeof(buf) - 1] = '\0';
	va_end(args);

	printf("%s", buf);

	if (s_log_filename[0])
	{
		FILE* pFile = fopen(s_log_filename, "a+");
		if (pFile)
		{
			fprintf(pFile, "%s", buf);
			fclose(pFile);
		}
	}
}

static uint get_file_size(const char* pFilename)
{
	FILE* pFile = fopen(pFilename, "rb");
	if (!pFile) return 0;
	fseek(pFile, 0, SEEK_END);
	uint file_size = ftell(pFile);
	fclose(pFile);
	return file_size;
}

struct image_compare_results
{
	image_compare_results() { memset(this, 0, sizeof(*this)); }

	double max_err;
	double mean;
	double mean_squared;
	double root_mean_squared;
	double peak_snr;
};

static void get_pixel(int* pDst, const uint8* pSrc, bool luma_only, int num_comps)
{
	int r, g, b;
	if (num_comps == 1)
	{
		r = g = b = pSrc[0];
	}
	else if (luma_only)
	{
		const int YR = 19595, YG = 38470, YB = 7471;
		r = g = b = (pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) / 65536;
	}
	else
	{
		r = pSrc[0]; g = pSrc[1]; b = pSrc[2];
	}
	pDst[0] = r; pDst[1] = g; pDst[2] = b;
}

// Compute image error metrics.
static void image_compare(image_compare_results& results, int width, int height, const uint8* pComp_image, int comp_image_comps, const uint8* pUncomp_image_data, int uncomp_comps, bool luma_only)
{
	double hist[256];
	memset(hist, 0, sizeof(hist));

	const uint first_channel = 0, num_channels = 3;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int a[3]; get_pixel(a, pComp_image + (y * width + x) * comp_image_comps, luma_only, comp_image_comps);
			int b[3]; get_pixel(b, pUncomp_image_data + (y * width + x) * uncomp_comps, luma_only, uncomp_comps);
			for (uint c = 0; c < num_channels; c++)
				hist[labs(a[first_channel + c] - b[first_channel + c])]++;
		}
	}

	results.max_err = 0;
	double sum = 0.0f, sum2 = 0.0f;
	for (uint i = 0; i < 256; i++)
	{
		if (!hist[i])
			continue;
		if (i > results.max_err)
			results.max_err = i;
		double x = i * hist[i];
		sum += x;
		sum2 += i * x;
	}

	// See http://bmrc.berkeley.edu/courseware/cs294/fall97/assignment/psnr.html
	double total_values = width * height;

	results.mean = sum / total_values;
	results.mean_squared = sum2 / total_values;

	results.root_mean_squared = sqrt(results.mean_squared);

	if (!results.root_mean_squared)
		results.peak_snr = 1e+10f;
	else
		results.peak_snr = log10(255.0f / results.root_mean_squared) * 20.0f;
}

// Simple exhaustive test. Tries compressing/decompressing image using all supported quality, subsampling, and Huffman optimization settings.
static int exhausive_compression_test(const char* pSrc_filename, bool use_jpgd)
{
	int status = EXIT_SUCCESS;

	// Load the source image.
	const int req_comps = 3; // request RGB image
	int width = 0, height = 0, actual_comps = 0;
	uint8* pImage_data = stbi_load(pSrc_filename, &width, &height, &actual_comps, req_comps);
	if (!pImage_data)
	{
		log_printf("Failed loading file \"%s\"!\n", pSrc_filename);
		return EXIT_FAILURE;
	}

	log_printf("Source file: \"%s\" Image resolution: %ix%i Actual comps: %i\n", pSrc_filename, width, height, actual_comps);

	int orig_buf_size = width * height * 3; // allocate a buffer that's hopefully big enough (this is way overkill for jpeg)
	if (orig_buf_size < 1024) orig_buf_size = 1024;
	void* pBuf = malloc(orig_buf_size);

	uint8* pUncomp_image_data = NULL;

	double max_err = 0;
	double lowest_psnr = 9e+9;
	double threshold_psnr = 9e+9;
	double threshold_max_err = 0.0f;

	image_compare_results prev_results;

	for (uint quality_factor = 1; quality_factor <= 100; quality_factor++)
	{
		for (uint subsampling = 0; subsampling <= jpge::H2V2; subsampling++)
		{
			for (uint optimize_huffman_tables = 0; optimize_huffman_tables <= 1; optimize_huffman_tables++)
			{
				// Fill in the compression parameter structure.
				jpge::params params;
				params.m_quality = quality_factor;
				params.m_subsampling = static_cast<jpge::subsampling_t>(subsampling);
				params.m_two_pass_flag = (optimize_huffman_tables != 0);

				int comp_size = orig_buf_size;
				if (!jpge::compress_image_to_jpeg_file_in_memory(pBuf, comp_size, width, height, req_comps, pImage_data, params))
				{
					status = EXIT_FAILURE;
					goto failure;
				}

				int uncomp_width = 0, uncomp_height = 0, uncomp_actual_comps = 0, uncomp_req_comps = 3;
				free(pUncomp_image_data);
				if (use_jpgd)
					pUncomp_image_data = jpgd::decompress_jpeg_image_from_memory((const stbi_uc*)pBuf, comp_size, &uncomp_width, &uncomp_height, &uncomp_actual_comps, uncomp_req_comps);
				else
					pUncomp_image_data = stbi_load_from_memory((const stbi_uc*)pBuf, comp_size, &uncomp_width, &uncomp_height, &uncomp_actual_comps, uncomp_req_comps);
				if (!pUncomp_image_data)
				{
					status = EXIT_FAILURE;
					goto failure;
				}

				if ((uncomp_width != width) || (uncomp_height != height))
				{
					status = EXIT_FAILURE;
					goto failure;
				}

				image_compare_results results;
				image_compare(results, width, height, pImage_data, req_comps, pUncomp_image_data, uncomp_req_comps, (params.m_subsampling == jpge::Y_ONLY) || (actual_comps == 1) || (uncomp_actual_comps == 1));
				//log_printf("Q: %3u, S: %u, O: %u, CompSize: %7u, Error Max: %3.3f, Mean: %3.3f, Mean^2: %5.3f, RMSE: %3.3f, PSNR: %3.3f\n", quality_factor, subsampling, optimize_huffman_tables, comp_size, results.max_err, results.mean, results.mean_squared, results.root_mean_squared, results.peak_snr);
				log_printf("%3u, %u, %u, %7u, %3.3f, %3.3f, %5.3f, %3.3f, %3.3f\n", quality_factor, subsampling, optimize_huffman_tables, comp_size, results.max_err, results.mean, results.mean_squared, results.root_mean_squared, results.peak_snr);
				if (results.max_err > max_err) max_err = results.max_err;
				if (results.peak_snr < lowest_psnr) lowest_psnr = results.peak_snr;

				if (quality_factor == 1)
				{
					if (results.peak_snr < threshold_psnr)
						threshold_psnr = results.peak_snr;
					if (results.max_err > threshold_max_err)
						threshold_max_err = results.max_err;
				}
				else
				{
					// Couple empirically determined tests - worked OK on my test data set.
					if ((results.peak_snr < (threshold_psnr - 3.0f)) || (results.peak_snr < 6.0f))
					{
						status = EXIT_FAILURE;
						goto failure;
					}
					if (optimize_huffman_tables)
					{
						if ((prev_results.max_err != results.max_err) || (prev_results.peak_snr != results.peak_snr))
						{
							status = EXIT_FAILURE;
							goto failure;
						}
					}
				}

				prev_results = results;
			}
		}
	}

	log_printf("Max error: %f Lowest PSNR: %f\n", max_err, lowest_psnr);

failure:
	free(pImage_data);
	free(pBuf);
	free(pUncomp_image_data);

	log_printf((status == EXIT_SUCCESS) ? "Success.\n" : "Exhaustive test failed!\n");
	return status;
}

// Test JPEG file decompression using jpgd.h
static int test_jpgd(const char* pSrc_filename, const char* pDst_filename, bool use_jpgd, bool no_simd, bool box_filtering)
{
	// Load the source JPEG image.
	const int req_comps = 4; // request RGB image
	int width = 0, height = 0, actual_comps = 0;

	if (use_jpgd)
		printf("Using jpgd::decompress_jpeg_image_from_file\n");
	else
		printf("Using stbi_load\n");

	timer tm;
	tm.start();
	
	uint8* pImage_data;

	// Set N > 1 to load the image multiple times, for profiling.
	const int N = 1;
	for (int t = 0; t < N; t++)
	{
		if (use_jpgd)
		{
			pImage_data = jpgd::decompress_jpeg_image_from_file(pSrc_filename, &width, &height, &actual_comps, req_comps, 
				(no_simd ? jpgd::jpeg_decoder::cFlagDisableSIMD : 0) |
				(box_filtering ? jpgd::jpeg_decoder::cFlagBoxChromaFiltering : 0) );
		}
		else
		{
			
			pImage_data = stbi_load(pSrc_filename, &width, &height, &actual_comps, req_comps);
		}

		if (t != (N - 1))
			free(pImage_data);
	}

	tm.stop();
	//exit(0);

	if (!pImage_data)
	{
		log_printf("Failed loading JPEG file \"%s\"!\n", pSrc_filename);
		return EXIT_FAILURE;
	}

	log_printf("Source JPEG file: \"%s\", image resolution: %ix%i, actual comps: %i\n", pSrc_filename, width, height, actual_comps);

	log_printf("Decompression time: %3.3fms\n", tm.get_elapsed_ms());

	if (!stbi_write_tga(pDst_filename, width, height, req_comps, pImage_data))
	{
		log_printf("Failed writing image to file \"%s\"!\n", pDst_filename);
		free(pImage_data);
		return EXIT_FAILURE;
	}
	log_printf("Wrote decompressed image to TGA file \"%s\"\n", pDst_filename);

	log_printf("Success.\n");

	free(pImage_data);
	return EXIT_SUCCESS;
}

