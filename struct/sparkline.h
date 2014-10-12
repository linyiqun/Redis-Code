/* sparkline.h -- ASCII Sparklines header file
 *
 * ---------------------------------------------------------------------------
 *
 * Copyright(C) 2011-2014 Salvatore Sanfilippo <antirez@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SPARKLINE_H
#define __SPARKLINE_H

/* sparkline是一类信息体积小和数据密度高的图表。目前它被用作一些测量，
 *相关的变化的信息呈现的方式，如平均温度，股市交投活跃。sparkline常常以一组多条的形式出现在柱状图，折线图当中。
 *可以理解为一个图线信息 */
/* A sequence is represented of many "samples" */
/* 可以理解为图像上的一个信息点，有文字，有值的大小 */
struct sample {
    double value;
    char *label;
};

/* 图线信息结构体，包括n个元素点，可以据此描述出图，绘图的可不是直接按点和值直接绘制的 */
struct sequence {
	//当前元素点个数
    int length;
    //总共的文字个数，有些点没有label描述，为NULL
    int labels;
    //元素点列表
    struct sample *samples;
    //元素中的最大值，最小值
    double min, max;
};

/* 定义了一些渲染图时候一些属性操作设置 */
#define SPARKLINE_NO_FLAGS 0
#define SPARKLINE_FILL 1      /* Fill the area under the curve. */
#define SPARKLINE_LOG_SCALE 2 /* Use logarithmic scale. */

struct sequence *createSparklineSequence(void);  //创建图线序列结构体
void sparklineSequenceAddSample(struct sequence *seq, double value, char *label); //在图线序列中添加一个信息点
void freeSparklineSequence(struct sequence *seq);   //释放图线序列
sds sparklineRenderRange(sds output, struct sequence *seq, int rows, int offset, int len, int flags); //渲染图线序列为一个图，其实就是得到一个字符串组成的图
sds sparklineRender(sds output, struct sequence *seq, int columns, int rows, int flags); //方法同上，只是少可一个偏移量

#endif /* __SPARKLINE_H */
