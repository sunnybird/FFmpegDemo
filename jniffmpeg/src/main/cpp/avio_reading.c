//
// Created by jinlong on 2019/3/9.
//

#include "avio_reading.h"
#include "nativelog.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>




static void log_callback_test2(void *ptr, int level, const char *fmt, va_list vl)
{
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



struct buffer_data {
	uint8_t *ptr;
	size_t size; ///< size left in the buffer
};


static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
	struct buffer_data *bd = (struct buffer_data *)opaque;
	buf_size = FFMIN(buf_size, bd->size);
	if (!buf_size)
		return AVERROR_EOF;
	LOGD("ptr:%p size:%zu\n", bd->ptr, bd->size);
	/* copy internal buffer data to buf */
	memcpy(buf, bd->ptr, buf_size);
	bd->ptr  += buf_size;
	bd->size -= buf_size;
	return buf_size;
}

int getAvioInfo(const char * filepath){
	
	
	AVFormatContext *fmt_ctx = NULL;
	AVIOContext *avio_ctx = NULL;
	uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
	size_t buffer_size, avio_ctx_buffer_size = 4096;
	char *input_filename = NULL;
	int ret = 0;
	struct buffer_data bd = { 0 };
	
	
	av_register_all();
	
	av_log_set_callback(log_callback_test2);
	
	
	input_filename = filepath;
	
	/* slurp file content into buffer */
	ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
	if (ret < 0)
		goto end;
	/* fill opaque structure used by the AVIOContext read callback */
	bd.ptr  = buffer;
	bd.size = buffer_size;
	if (!(fmt_ctx = avformat_alloc_context())) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	avio_ctx_buffer = av_malloc(avio_ctx_buffer_size);
	if (!avio_ctx_buffer) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
	                              0, &bd, &read_packet, NULL, NULL);
	if (!avio_ctx) {
		ret = AVERROR(ENOMEM);
		goto end;
	}
	fmt_ctx->pb = avio_ctx;
	ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
	if (ret < 0) {
		LOGD("Could not open input\n");
		goto end;
	}
	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		LOGD("Could not find stream information\n");
		goto end;
	}
	av_dump_format(fmt_ctx, 0, input_filename, 0);
	end:
	avformat_close_input(&fmt_ctx);
	/* note: the internal buffer could have changed, and be != avio_ctx_buffer */
	if (avio_ctx) {
		av_freep(&avio_ctx->buffer);
		av_freep(&avio_ctx);
	}
	av_file_unmap(buffer, buffer_size);
	if (ret < 0) {
		LOGD("Error occurred: %s\n", av_err2str(ret));
		return 1;
	}
	return 0;



}