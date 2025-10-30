#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    int fd_usbcam = -1;
    const char *device_name = "/dev/video0";
    int iret;

    fd_usbcam = open(device_name, O_RDWR);
    if (fd_usbcam < 0) {
        perror("打开视频设备失败");
        return -1;
    }
    printf("视频设备 %s 打开成功 (fd=%d)\n", device_name, fd_usbcam);

    struct v4l2_capability cap;
    iret = ioctl(fd_usbcam, VIDIOC_QUERYCAP, &cap);
    if (iret < 0) {
        perror("查询设备能力失败 (VIDIOC_QUERYCAP)");
        close(fd_usbcam);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "设备不支持视频捕获\n");
        close(fd_usbcam);
        return -1;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "设备不支持流式IO\n");
        close(fd_usbcam);
        return -1;
    }
    printf("设备能力查询成功。\n");

    struct v4l2_format tv4l2_format;
    memset(&tv4l2_format, 0, sizeof(tv4l2_format));
    tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tv4l2_format.fmt.pix.width = 640;
    tv4l2_format.fmt.pix.height = 480;
    tv4l2_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    tv4l2_format.fmt.pix.field = V4L2_FIELD_ANY;

    iret = ioctl(fd_usbcam, VIDIOC_S_FMT, &tv4l2_format);
    if (iret < 0) {
        perror("设置视频格式失败 (VIDIOC_S_FMT)");
        close(fd_usbcam);
        return -1;
    }
    if (tv4l2_format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) {
        fprintf(stderr, "警告: 驱动不支持YUYV格式，实际格式为 %c%c%c%c\n",
                tv4l2_format.fmt.pix.pixelformat & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 8) & 0xFF,
                (tv4l2_format.fmt.pix.pixelformat >> 16) & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 24) & 0xFF);
    }
    printf("视频格式设置成功 (Width: %d, Height: %d, PixelFormat: %c%c%c%c)\n",
        tv4l2_format.fmt.pix.width, tv4l2_format.fmt.pix.height,
        tv4l2_format.fmt.pix.pixelformat & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 8) & 0xFF,
        (tv4l2_format.fmt.pix.pixelformat >> 16) & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 24) & 0xFF);


    struct v4l2_requestbuffers tV4L2_reqbuf;
    memset(&tV4L2_reqbuf, 0, sizeof(tV4L2_reqbuf));
    tV4L2_reqbuf.count = 3;
    tV4L2_reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4L2_reqbuf.memory = V4L2_MEMORY_MMAP;

    iret = ioctl(fd_usbcam, VIDIOC_REQBUFS, &tV4L2_reqbuf);
    if (iret < 0) {
        perror("请求缓冲区失败 (VIDIOC_REQBUFS)");
        close(fd_usbcam);
        return -1;
    }
    if (tV4L2_reqbuf.count < 1) {
        fprintf(stderr, "缓冲区数量不足。\n");
        close(fd_usbcam);
        return -1;
    }
    printf("请求到 %d 个缓冲区。\n", tV4L2_reqbuf.count);

    unsigned int n_buffers = tV4L2_reqbuf.count;
    void **buffer_start_addrs = calloc(n_buffers, sizeof(void *));
    unsigned int *buffer_lengths = calloc(n_buffers, sizeof(unsigned int));
    if (!buffer_start_addrs || !buffer_lengths) {
        perror("calloc 分配内存失败");
        close(fd_usbcam);
        return -1;
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer tV4L2buf;
        memset(&tV4L2buf, 0, sizeof(tV4L2buf));
        tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tV4L2buf.memory = V4L2_MEMORY_MMAP;
        tV4L2buf.index = i;

        iret = ioctl(fd_usbcam, VIDIOC_QUERYBUF, &tV4L2buf);
        if (iret < 0) {
            char err_msg[100];
            sprintf(err_msg, "查询缓冲区 %d 失败 (VIDIOC_QUERYBUF)", i);
            perror(err_msg);
            for(unsigned int j=0; j<i; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }

        buffer_lengths[i] = tV4L2buf.length;
        buffer_start_addrs[i] = mmap(NULL,
                                     tV4L2buf.length,
                                     PROT_READ | PROT_WRITE,
                                     MAP_SHARED,
                                     fd_usbcam,
                                     tV4L2buf.m.offset);

        if (buffer_start_addrs[i] == MAP_FAILED) {
            char err_msg[100];
            sprintf(err_msg, "映射缓冲区 %d 失败 (mmap)", i);
            perror(err_msg);
            for(unsigned int j=0; j<i; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }
        printf("缓冲区 %d 映射成功 (地址: %p, 长度: %u)\n", i, buffer_start_addrs[i], buffer_lengths[i]);
    }

    for (unsigned int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer tV4L2buf;
        memset(&tV4L2buf, 0, sizeof(tV4L2buf));
        tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        tV4L2buf.memory = V4L2_MEMORY_MMAP;
        tV4L2buf.index = i;

        iret = ioctl(fd_usbcam, VIDIOC_QBUF, &tV4L2buf);
        if (iret < 0) {
            char err_msg[100];
            sprintf(err_msg, "缓冲区 %d 入队失败 (VIDIOC_QBUF)", i);
            perror(err_msg);
            for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }
    }
    printf("所有缓冲区均已入队。\n");

    enum v4l2_buf_type type_stream = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    iret = ioctl(fd_usbcam, VIDIOC_STREAMON, &type_stream);
    if (iret < 0) {
        perror("启动视频流失败 (VIDIOC_STREAMON)");
        for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
        free(buffer_start_addrs);
        free(buffer_lengths);
        close(fd_usbcam);
        return -1;
    }
    printf("视频流已启动。\n");

    struct v4l2_buffer tV4L2_dqbuf;
    memset(&tV4L2_dqbuf, 0, sizeof(tV4L2_dqbuf));
    tV4L2_dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    tV4L2_dqbuf.memory = V4L2_MEMORY_MMAP;

    iret = ioctl(fd_usbcam, VIDIOC_DQBUF, &tV4L2_dqbuf);
    if (iret < 0) {
        perror("从队列取出缓冲区失败 (VIDIOC_DQBUF)");
        ioctl(fd_usbcam, VIDIOC_STREAMOFF, &type_stream);
        for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
        free(buffer_start_addrs);
        free(buffer_lengths);
        close(fd_usbcam);
        return -1;
    }
    printf("成功获取一帧数据，缓冲区索引: %d, 数据大小: %d 字节。\n", tV4L2_dqbuf.index, tV4L2_dqbuf.bytesused);

    FILE *fp_yuv = fopen("frame.yuv", "wb");
    if (fp_yuv == NULL) {
        perror("打开文件frame.yuv失败");
        ioctl(fd_usbcam, VIDIOC_STREAMOFF, &type_stream);
        for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
        free(buffer_start_addrs);
        free(buffer_lengths);
        close(fd_usbcam);
        return -1;
    }

    fwrite(buffer_start_addrs[tV4L2_dqbuf.index], 1, tV4L2_dqbuf.bytesused, fp_yuv);
    fclose(fp_yuv);
    printf("图像数据已保存到 frame.yuv (宽度: %d, 高度: %d, 格式: YUYV)\n",
           tv4l2_format.fmt.pix.width, tv4l2_format.fmt.pix.height);

    iret = ioctl(fd_usbcam, VIDIOC_STREAMOFF, &type_stream);
    if (iret < 0) {
        perror("停止视频流失败 (VIDIOC_STREAMOFF)");
    }
    printf("视频流已停止。\n");

    for (unsigned int i = 0; i < n_buffers; ++i) {
        if (buffer_start_addrs[i] != MAP_FAILED && buffer_start_addrs[i] != NULL) {
            if (munmap(buffer_start_addrs[i], buffer_lengths[i]) == -1) {
                char err_msg[100];
                sprintf(err_msg, "解除缓冲区 %d 映射失败", i);
                perror(err_msg);
            }
        }
    }
    free(buffer_start_addrs);
    free(buffer_lengths);
    printf("所有缓冲区已解除映射并释放内存。\n");

    if (close(fd_usbcam) == -1) {
        perror("关闭视频设备失败");
        return -1;
    }
    printf("视频设备已关闭。\n");

    return 0;
}