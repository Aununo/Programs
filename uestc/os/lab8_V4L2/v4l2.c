#include <fcntl.h>           // 用于 open() 和 O_* 标志
#include <sys/ioctl.h>       // 用于 ioctl()
#include <linux/videodev2.h> // 用于 V4L2 结构体和定义
#include <sys/mman.h>        // 用于 mmap() 和 munmap()
#include <stdio.h>           // 用于 printf(), fopen(), fwrite(), fclose(), perror(), sprintf()
#include <string.h>          // 用于 memset(), memcpy()
#include <errno.h>           // 用于 errno
#include <unistd.h>          // 用于 close()
#include <stdlib.h>          // 用于 calloc(), free(), exit() (或者 return -1 从main)

int main() { // <--- 所有代码应在此函数内或被此函数调用
    int fd_usbcam = -1;
    const char *device_name = "/dev/video0"; // 如果您的摄像头设备号不同，请调整
    int iret; // <--- 声明 iret

    // 1. 打开视频设备
    fd_usbcam = open(device_name, O_RDWR);
    if (fd_usbcam < 0) {
        perror("打开视频设备失败");
        return -1;
    }
    printf("视频设备 %s 打开成功 (fd=%d)\n", device_name, fd_usbcam);

    // (建议加入：查询设备能力 VIDIOC_QUERYCAP - 验证是否支持视频捕获和流IO)
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

    // 2. 设置视频格式 (VIDIOC_S_FMT)
    struct v4l2_format tv4l2_format;
    memset(&tv4l2_format, 0, sizeof(tv4l2_format));
    tv4l2_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // [cite: 12]
    tv4l2_format.fmt.pix.width = 640;  // 设置期望的宽度，例如640
    tv4l2_format.fmt.pix.height = 480; // 设置期望的高度，例如480
    tv4l2_format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // 设置为YUYV格式 [cite: 12]
    tv4l2_format.fmt.pix.field = V4L2_FIELD_ANY; // 通常用 V4L2_FIELD_ANY 或 V4L2_FIELD_NONE，除非确定是隔行扫描 [cite: 12]

    iret = ioctl(fd_usbcam, VIDIOC_S_FMT, &tv4l2_format); // [cite: 12]
    if (iret < 0) {
        perror("设置视频格式失败 (VIDIOC_S_FMT)");
        close(fd_usbcam);
        return -1;
    }
    // 确认实际应用的格式，特别是宽度、高度和像素格式，因为驱动可能调整它们
    if (tv4l2_format.fmt.pix.pixelformat != V4L2_PIX_FMT_YUYV) {
        fprintf(stderr, "警告: 驱动不支持YUYV格式，实际格式为 %c%c%c%c\n",
                tv4l2_format.fmt.pix.pixelformat & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 8) & 0xFF,
                (tv4l2_format.fmt.pix.pixelformat >> 16) & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 24) & 0xFF);
        // 可以选择在此处失败，或继续尝试处理驱动选择的格式
    }
    printf("视频格式设置成功 (Width: %d, Height: %d, PixelFormat: %c%c%c%c)\n",
        tv4l2_format.fmt.pix.width, tv4l2_format.fmt.pix.height,
        tv4l2_format.fmt.pix.pixelformat & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 8) & 0xFF,
        (tv4l2_format.fmt.pix.pixelformat >> 16) & 0xFF, (tv4l2_format.fmt.pix.pixelformat >> 24) & 0xFF);


    // 3. 请求缓冲区 (VIDIOC_REQBUFS)
    struct v4l2_requestbuffers tV4L2_reqbuf;
    memset(&tV4L2_reqbuf, 0, sizeof(tV4L2_reqbuf));
    tV4L2_reqbuf.count = 3; // 申请3个缓冲区 [cite: 17]
    tV4L2_reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // [cite: 17]
    tV4L2_reqbuf.memory = V4L2_MEMORY_MMAP; // 使用内存映射方式 [cite: 17, 18]

    iret = ioctl(fd_usbcam, VIDIOC_REQBUFS, &tV4L2_reqbuf); // [cite: 17]
    if (iret < 0) {
        perror("请求缓冲区失败 (VIDIOC_REQBUFS)");
        close(fd_usbcam);
        return -1;
    }
    if (tV4L2_reqbuf.count < 1) { // 确保至少有一个缓冲区被分配
        fprintf(stderr, "缓冲区数量不足。\n");
        close(fd_usbcam);
        return -1;
    }
    printf("请求到 %d 个缓冲区。\n", tV4L2_reqbuf.count);

    // 保存实际分配到的缓冲区数量
    unsigned int n_buffers = tV4L2_reqbuf.count;
    // 为映射后的缓冲区地址和长度创建数组
    void **buffer_start_addrs = calloc(n_buffers, sizeof(void *));
    unsigned int *buffer_lengths = calloc(n_buffers, sizeof(unsigned int));
    if (!buffer_start_addrs || !buffer_lengths) {
        perror("calloc 分配内存失败");
        close(fd_usbcam);
        return -1;
    }

    // 4. 查询并映射缓冲区 (VIDIOC_QUERYBUF, mmap)
    for (unsigned int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer tV4L2buf;
        memset(&tV4L2buf, 0, sizeof(tV4L2buf));
        tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // [cite: 19]
        tV4L2buf.memory = V4L2_MEMORY_MMAP; // [cite: 19]
        tV4L2buf.index = i; // 要查询的缓冲区索引 [cite: 19]

        iret = ioctl(fd_usbcam, VIDIOC_QUERYBUF, &tV4L2buf); // [cite: 19]
        if (iret < 0) {
            char err_msg[100];
            sprintf(err_msg, "查询缓冲区 %d 失败 (VIDIOC_QUERYBUF)", i);
            perror(err_msg);
            // 清理：解除已映射的缓冲区，释放内存，关闭设备
            for(unsigned int j=0; j<i; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }

        buffer_lengths[i] = tV4L2buf.length; // [cite: 20]
        buffer_start_addrs[i] = mmap(NULL, // 内核选择起始地址
                                     tV4L2buf.length,
                                     PROT_READ | PROT_WRITE, // 映射的内存区域可读可写 [cite: 20]
                                     MAP_SHARED, // 共享映射 [cite: 20]
                                     fd_usbcam,
                                     tV4L2buf.m.offset); // 缓冲区的偏移量 [cite: 20]

        if (buffer_start_addrs[i] == MAP_FAILED) {
            char err_msg[100];
            sprintf(err_msg, "映射缓冲区 %d 失败 (mmap)", i);
            perror(err_msg);
            // 清理
            for(unsigned int j=0; j<i; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }
        printf("缓冲区 %d 映射成功 (地址: %p, 长度: %u)\n", i, buffer_start_addrs[i], buffer_lengths[i]);
    }

    // 5. 将缓冲区加入队列 (VIDIOC_QBUF)
    for (unsigned int i = 0; i < n_buffers; ++i) {
        struct v4l2_buffer tV4L2buf; // 重新声明和初始化 tV4L2buf
        memset(&tV4L2buf, 0, sizeof(tV4L2buf));
        tV4L2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // [cite: 25]
        tV4L2buf.memory = V4L2_MEMORY_MMAP; // [cite: 25]
        tV4L2buf.index = i; // 指定要放入队列的缓冲区编号 [cite: 25]

        iret = ioctl(fd_usbcam, VIDIOC_QBUF, &tV4L2buf); // [cite: 25]
        if (iret < 0) {
            char err_msg[100];
            sprintf(err_msg, "缓冲区 %d 入队失败 (VIDIOC_QBUF)", i);
            perror(err_msg);
            // 清理
            for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
            free(buffer_start_addrs);
            free(buffer_lengths);
            close(fd_usbcam);
            return -1;
        }
    }
    printf("所有缓冲区均已入队。\n");

    // 6. 启动视频流 (VIDIOC_STREAMON)
    enum v4l2_buf_type type_stream = V4L2_BUF_TYPE_VIDEO_CAPTURE; // 避免与上面的 type 重名（如果存在） [cite: 27]
    iret = ioctl(fd_usbcam, VIDIOC_STREAMON, &type_stream); // [cite: 27]
    if (iret < 0) {
        perror("启动视频流失败 (VIDIOC_STREAMON)");
        // 清理
        for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
        free(buffer_start_addrs);
        free(buffer_lengths);
        close(fd_usbcam);
        return -1;
    }
    printf("视频流已启动。\n");

    // 7. 从队列中取出一帧数据 (VIDIOC_DQBUF)
    struct v4l2_buffer tV4L2_dqbuf;
    memset(&tV4L2_dqbuf, 0, sizeof(tV4L2_dqbuf));
    tV4L2_dqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // [cite: 26]
    tV4L2_dqbuf.memory = V4L2_MEMORY_MMAP; // [cite: 26]

    // 等待一帧数据 (这里使用阻塞模式的ioctl)
    iret = ioctl(fd_usbcam, VIDIOC_DQBUF, &tV4L2_dqbuf); // [cite: 26]
    if (iret < 0) {
        perror("从队列取出缓冲区失败 (VIDIOC_DQBUF)");
        // 清理
        // 注意：此时可能部分缓冲区仍在队列中或已映射，STREAMOFF可能需要先调用
        ioctl(fd_usbcam, VIDIOC_STREAMOFF, &type_stream); // 尝试停止流
        for(unsigned int j=0; j<n_buffers; ++j) if(buffer_start_addrs[j] != MAP_FAILED) munmap(buffer_start_addrs[j], buffer_lengths[j]);
        free(buffer_start_addrs);
        free(buffer_lengths);
        close(fd_usbcam);
        return -1;
    }
    printf("成功获取一帧数据，缓冲区索引: %d, 数据大小: %d 字节。\n", tV4L2_dqbuf.index, tV4L2_dqbuf.bytesused);

    // 8. 保存图像数据到文件
    FILE *fp_yuv = fopen("frame.yuv", "wb");
    if (fp_yuv == NULL) {
        perror("打开文件frame.yuv失败");
        // 清理， DQBUF成功后，数据在 tV4L2_dqbuf.index 对应的映射区
        // 可以选择将此缓冲区重新入队或直接停止
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

    // (可选：如果要继续捕获，将 tV4L2_dqbuf 重新入队 VIDIOC_QBUF)
    // ioctl(fd_usbcam, VIDIOC_QBUF, &tV4L2_dqbuf);

    // 9. 停止视频流 (VIDIOC_STREAMOFF)
    iret = ioctl(fd_usbcam, VIDIOC_STREAMOFF, &type_stream); // [cite: 28]
    if (iret < 0) {
        perror("停止视频流失败 (VIDIOC_STREAMOFF)");
        // 仍然尝试清理和关闭
    }
    printf("视频流已停止。\n");

    // 10. 解除内存映射 (munmap)
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

    // 11. 关闭视频设备
    if (close(fd_usbcam) == -1) { // [cite: 1]
        perror("关闭视频设备失败");
        return -1; // 或者记录错误
    }
    printf("视频设备已关闭。\n");

    return 0; // main 函数正常结束
}