//
// Created by jinlong on 2019/3/9.
//



#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/pixfmt.h>

#include "nativelog.h"


static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx = NULL;
static int width, height;
static enum AVPixelFormat pix_fmt;
static AVStream *video_st = NULL, *audio_st = NULL;
static const char *src_filename = NULL;
static const char *video_des_filename = NULL;
static const char *audio_des_filename = NULL;
static FILE *video_des_file = NULL;
static FILE *audio_des_file = NULL;

static uint8_t *video_dst_data[4] = {NULL};
static int video_dst_linesize[4];
static int video_dst_bufsize;

static int video_stream_index = -1, audio_stream_index = -1;
static AVFrame *frame = NULL;
static AVFrame *pFrameRGB = NULL;
static AVPacket pkt;
static int video_frame_count = 0;
static int audio_frame_count = 0;

static AVBitStreamFilterContext *h264bsfc = NULL;

struct SwsContext *sws_ctx = NULL;


static int refcount = 0;

static int i = 0;


static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename) {
	FILE *f;
	int i;
	f = fopen(filename, "w");
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}

static void save_frame(AVFrame *pFrame, int width, int height) {
	
	FILE *pFile;
	char *szFilename = "/data/share/frame.yuv";
	int y;
	
	pFile = fopen(szFilename, "wb");
	
	if (pFile == NULL)
		return;
	
	int y_size = width * height;
	int u_size = y_size / 4;
	int v_size = y_size / 4;
	
	//写入文件
	//首先写入Y，再是U，再是V
	//in_frame_picture->data[0]表示Y
	fwrite(pFrame->data[0], 1, y_size, pFile);
	//in_frame_picture->data[1]表示U
	fwrite(pFrame->data[1], 1, u_size, pFile);
	//in_frame_picture->data[2]表示V
	fwrite(pFrame->data[2], 1, v_size, pFile);
	
	// Close file
	fclose(pFile);
	
}

static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
	FILE *pFile;
	char szFilename[32];
	int y;
	
	// Open file
	sprintf(szFilename, "/data/share/frame%d.ppm", iFrame);
	pFile = fopen(szFilename, "wb");
	if (pFile == NULL)
		return;
	
	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);
	
	// Write pixel data
	for (y = 0; y < height; y++)
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
	
	// Close file
	fclose(pFile);
}


static void log_callback_android(void *ptr, int level, const char *fmt, va_list vl) {
	va_list vl2;
	char *line = malloc(128 * sizeof(char));
	static int print_prefix = 1;
	va_copy(vl2, vl);
	av_log_format_line(ptr, level, fmt, vl2, line, 128, &print_prefix);
	va_end(vl2);
	line[127] = '\0';
	LOGD("%s", line);
	free(line);
}

static int decode_packet(int *got_frame, int cached) {
	int ret = 0;
	int decoded = pkt.size;
	*got_frame = 0;
	if (pkt.stream_index == video_stream_index) {


//		av_bitstream_filter_filter(h264bsfc, fmt_ctx->streams[video_stream_index]->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		
		/* decode video frame */
		ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			LOGD("Error decoding video frame (%s)\n", av_err2str(ret));
			return ret;
		}
		
		if (*got_frame) {
			if (frame->width != width || frame->height != height ||
			    frame->format != pix_fmt) {
				/* To handle this change, one could call av_image_alloc again and
				 * decode the following frames into another rawvideo file. */
				LOGD("Error: Width, height and pixel format have to be "
				     "constant in a rawvideo file, but the width, height or "
				     "pixel format of the input video changed:\n"
				     "old: width = %d, height = %d, format = %s\n"
				     "new: width = %d, height = %d, format = %s\n",
				     width, height, av_get_pix_fmt_name(pix_fmt),
				     frame->width, frame->height,
				     av_get_pix_fmt_name(frame->format));
				return -1;
			}
			LOGD("video_frame%s n:%d coded_n:%d\n，%s",
			     cached ? "(cached)" : "",
			     video_frame_count++, frame->coded_picture_number,
			     av_get_pix_fmt_name(frame->format));
			
			
			//////////////// https://www.ffmpeg.org/doxygen/4.0/demuxing_decoding_8c-example.html
			///////////////
			/* copy decoded frame to destination buffer:
			 * this is required since rawvideo expects non aligned data */
			
			
			av_image_copy(video_dst_data, video_dst_linesize,
			              (const uint8_t **) (frame->data), frame->linesize,
			              pix_fmt, width, height);
			
			
			/* write to rawvideo file */
			fwrite(video_dst_data[0], 1, video_dst_bufsize, video_des_file);

			///////////////
			///////////////



			char buf[1024];
			LOGD("saving frame %3d\n", video_dec_ctx->frame_number);
			snprintf(buf, sizeof(buf), "%s-%d", video_des_filename, video_dec_ctx->frame_number);
			pgm_save(frame->data[0], frame->linesize[0],
			         frame->width, frame->height, buf);
			
			
			// save as yuv file
//			save_frame(frame,video_dec_ctx->width,video_dec_ctx->has_b_frames);
			
		}
		
		
	} else if (pkt.stream_index == audio_stream_index) {
		/* decode audio frame */
		ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
		if (ret < 0) {
			LOGD("Error decoding audio frame (%s)\n", av_err2str(ret));
			return ret;
		}
		/* Some audio decoders decode only part of the packet, and have to be
		 * called again with the remainder of the packet data.
		 * Sample: fate-suite/lossless-audio/luckynight-partial.shn
		 * Also, some decoders might over-read the packet. */
		decoded = FFMIN(ret, pkt.size);
		if (*got_frame) {
			size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample(frame->format);
			LOGD("audio_frame%s n:%d nb_samples:%d pts:%s\n",
			     cached ? "(cached)" : "",
			     audio_frame_count++, frame->nb_samples,
			     av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));
			/* Write the raw audio data samples of the first plane. This works
			 * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
			 * most audio decoders output planar audio, which uses a separate
			 * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
			 * In other words, this code will write only the first audio channel
			 * in these cases.
			 * You should use libswresample or libavfilter to convert the frame
			 * to packed data. */
			fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_des_file);
		}
	}
	/* If we use frame reference counting, we own the data and need
	 * to de-reference it when we don't use it anymore */
	if (*got_frame && refcount)
		av_frame_unref(frame);
	return decoded;
}


static int open_codec_context(int *strem_index,
                              AVCodecContext **dec_ctx,
                              AVFormatContext *fmt_ctx, enum AVMediaType type) {
	
	int ret, stream_index;
	AVStream *st;
	AVCodec *dec = NULL;
	AVDictionary *opts = NULL;
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
	if (ret < 0) {
		LOGD("Could not find %s stream in input file '%s'\n",
		     av_get_media_type_string(type), src_filename);
		return ret;
	} else {
		stream_index = ret;
		st = fmt_ctx->streams[stream_index];
		/* find decoder for the stream */
		dec = avcodec_find_decoder(st->codecpar->codec_id);
		if (!dec) {
			LOGE("Failed to find %s codec\n",
			     av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}
		/* Allocate a codec context for the decoder */
		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			LOGD("Failed to allocate the %s codec context\n",
			     av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}
		/* Copy codec parameters from input stream to output codec context */
		if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
			LOGD("Failed to copy %s codec parameters to decoder context\n",
			     av_get_media_type_string(type));
			return ret;
		}
		/* Init the decoders, with or without reference counting */
		av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
		if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
			LOGD("Failed to open %s codec\n",
			     av_get_media_type_string(type));
			return ret;
		}
		*strem_index = stream_index;
	}
	return 0;
}


static int get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt) {
	int i;
	struct sample_fmt_entry {
		enum AVSampleFormat sample_fmt;
		const char *fmt_be, *fmt_le;
	} sample_fmt_entries[] = {
			{AV_SAMPLE_FMT_U8,  "u8",    "u8"},
			{AV_SAMPLE_FMT_S16, "s16be", "s16le"},
			{AV_SAMPLE_FMT_S32, "s32be", "s32le"},
			{AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
			{AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
	};
	*fmt = NULL;
	for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
		struct sample_fmt_entry *entry = &sample_fmt_entries[i];
		if (sample_fmt == entry->sample_fmt) {
			*fmt = AV_NE(entry->fmt_be, entry->fmt_le);
			return 0;
		}
	}
	LOGD("sample format %s is not supported as output format\n",
	     av_get_sample_fmt_name(sample_fmt));
	return -1;
}


int demuxer_simple(const char *filepath, const char *vfilpath, const char *afilepath) {
	int ret = 0, got_frame;
	src_filename = filepath;
	video_des_filename = vfilpath;
	audio_des_filename = afilepath;
	
	av_register_all();
//	av_log_set_callback(log_callback_android);
	
	pFrameRGB = av_frame_alloc();
	
	
	if (avformat_open_input(&fmt_ctx, src_filename, NULL, NULL) < 0) {
		LOGE("Could not open source file %s\n", src_filename);
		ret = -1;
		return ret;
	}
	
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		LOGE("Could not find stream information");
		ret = -2;
		return ret;
	}
	
	if (open_codec_context(&video_stream_index, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
		video_st = fmt_ctx->streams[video_stream_index];
		video_des_file = fopen(video_des_filename, "wb");
		if (!video_des_file) {
			LOGE("Could not open destination file %s\n", video_des_filename);
			ret = 1;
			goto end;
		}
		/* allocate image where the decoded image will be put */
		width = video_dec_ctx->width;
		height = video_dec_ctx->height;
		pix_fmt = video_dec_ctx->pix_fmt;
		ret = av_image_alloc(video_dst_data, video_dst_linesize,
		                     width, height, pix_fmt, 1);
		if (ret < 0) {
			LOGE("Could not allocate raw video buffer\n");
			goto end;
		}
		video_dst_bufsize = ret;
	}
	if (open_codec_context(&audio_stream_index, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
		audio_st = fmt_ctx->streams[audio_stream_index];
		audio_des_file = fopen(audio_des_filename, "wb");
		if (!audio_des_file) {
			LOGE("Could not open destination file %s\n", audio_des_filename);
			ret = 1;
			goto end;
		}
	}
	
	
	sws_ctx = sws_getContext(video_dec_ctx->width,
	                         video_dec_ctx->height,
	                         video_dec_ctx->pix_fmt,
	                         video_dec_ctx->width,
	                         video_dec_ctx->height,
	                         AV_PIX_FMT_RGB24,
	                         SWS_BICUBIC,
	                         NULL,
	                         NULL,
	                         NULL
	);
	
	
	/* dump input information to stderr */
//	av_dump_format(fmt_ctx, 0, src_filename, 0);
	
	if (!audio_st && !video_st) {
		LOGE("Could not find audio or video stream in the input, aborting\n");
		ret = 1;
		goto end;
	}
	frame = av_frame_alloc();
	
	if (!frame) {
		LOGE("Could not allocate frame\n");
		ret = AVERROR(ENOMEM);
		goto end;
	}
	
	/* initialize packet, set data to NULL, let the demuxer fill it */
	av_init_packet(&pkt);
	
	pkt.data = NULL;
	pkt.size = 0;
	if (video_st)
		LOGE("Demuxing video from file '%s' into '%s'\n", src_filename, video_des_filename);
	if (audio_st)
		LOGE("Demuxing audio from file '%s' into '%s'\n", src_filename, audio_des_filename);
	
	
	h264bsfc = av_bitstream_filter_init("h264_mp4toannexb");
	
	/* read frames from the file */
	while (av_read_frame(fmt_ctx, &pkt) >= 0) {
		AVPacket orig_pkt = pkt;
		do {
			ret = decode_packet(&got_frame, 0);
			if (ret < 0)
				break;
			pkt.data += ret;
			pkt.size -= ret;
		} while (pkt.size > 0);
		av_packet_unref(&orig_pkt);
	}
	/* flush cached frames */
	pkt.data = NULL;
	pkt.size = 0;
	do {
		decode_packet(&got_frame, 1);
	} while (got_frame);
	LOGE("Demuxing succeeded.\n");
	if (video_st) {
		LOGE("Play the output video file with the command:\n"
		     "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
		     av_get_pix_fmt_name(pix_fmt), width, height,
		     video_des_filename);
	}
	if (audio_st) {
		enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
		int n_channels = audio_dec_ctx->channels;
		const char *fmt;
		if (av_sample_fmt_is_planar(sfmt)) {
			const char *packed = av_get_sample_fmt_name(sfmt);
			LOGE("Warning: the sample format the decoder produced is planar "
			     "(%s). This example will output the first channel only.\n",
			     packed ? packed : "?");
			sfmt = av_get_packed_sample_fmt(sfmt);
			n_channels = 1;
		}
		if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
			goto end;
		LOGE("Play the output audio file with the command:\n"
		     "ffplay -f %s -ac %d -ar %d %s\n",
		     fmt, n_channels, audio_dec_ctx->sample_rate,
		     audio_des_filename);
	}
	end:
	avcodec_free_context(&video_dec_ctx);
	avcodec_free_context(&audio_dec_ctx);
	av_bitstream_filter_close(h264bsfc);
	avformat_close_input(&fmt_ctx);
	if (video_des_file)
		fclose(video_des_file);
	if (audio_des_file)
		fclose(audio_des_file);
	av_frame_free(&frame);
	av_free(video_dst_data[0]);
	return ret < 0;
	
}