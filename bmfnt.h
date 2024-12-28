#ifndef _BMFNT_H
#define _BMFNT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef struct string_buf
{
    char *data;
    size_t len;
} string_buf;

typedef struct bmf_padding
{
    int up;
    int right;
    int down;
    int left;
} bmf_padding_t;

typedef struct bmf_spacing
{
    int horizontal;
    int vertical;
} bmf_spacing_t;

typedef struct bmf_info
{
    string_buf face;
    int size;
    int bold;
    int italic;
    string_buf charset;
    int unicode;
    int stretchh;
    int smooth;
    int aa;
    bmf_padding_t padding;
    bmf_spacing_t spacing;
    int outline;
} bmf_info_t;

typedef struct bmf_common
{
    int line_height;
    int base;
    int scale_w;
    int scale_h;
    int pages;
    int packed;
    int alpha_chnl;
    int red_chnl;
    int green_chnl;
    int blue_chnl;
} bmf_common_t;

typedef struct bmf_page
{
    int id;
    string_buf file;
} bmf_page_t;

typedef struct bmf_char
{
    int id;
    int x;
    int y;
    int width;
    int height;
    int xoffset;
    int yoffset;
    int xadvance;
    int page;
    int chnl;
} bmf_char_t;

typedef struct bmf_kerning
{
    int first;
    int second;
    int amount;
} bmf_kerning_t;

typedef struct bmf_fnt
{
    char *data;
    bmf_info_t info;
    bmf_common_t common;
    bmf_page_t page;
    int chars_count;
    bmf_char_t *chars;
    int kernings_count;
    bmf_kerning_t *kernings;
} bmf_fnt_t;

void bmf_read_from_txt_file(const char *path, bmf_fnt_t *fnt);
void bmf_free(bmf_fnt_t *fnt);
void bmf_print(FILE *stream, bmf_fnt_t *fnt);

#ifdef BMFNT_IMPLEMENTATION

void parse_info(string_buf buf, bmf_info_t *info)
{
    char *start = buf.data;
    char *end = buf.data;
    start = strstr(buf.data, "face=");
    if (start != NULL)
    {
        start = start + 6;
        end = strchr(start, '"');
        info->face.data = start;
        info->face.len = end - start;
    }
    start = end + 1;
    sscanf(start, " size=%d bold=%d italic=%d",
           &info->size,
           &info->bold,
           &info->italic);
    start = strstr(end, "charset=");
    if (start != NULL)
    {
        start = start + 9;
        end = strchr(start, '"');
        info->charset.data = start;
        info->charset.len = end - start;
    }
    start = end + 1;
    sscanf(start, " unicode=%d stretchH=%d smooth=%d aa=%d padding=%d,%d,%d,%d spacing=%d,%d outline=%d",
           &info->unicode,
           &info->stretchh,
           &info->smooth,
           &info->aa,
           &info->padding.up,
           &info->padding.right,
           &info->padding.down,
           &info->padding.left,
           &info->spacing.horizontal,
           &info->spacing.vertical,
           &info->outline);
}

void parse_common(string_buf buf, bmf_common_t *common)
{
    // common lineHeight=32 base=26 scaleW=256 scaleH=256 pages=1 packed=0 alphaChnl=1 redChnl=0 greenChnl=0 blueChnl=0
    const char *fmt = "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d alphaChnl=%d redChnl=%d greenChnl=%d blueChnl=%d";
    sscanf(buf.data, fmt,
           &common->line_height,
           &common->base,
           &common->scale_w,
           &common->scale_h,
           &common->pages,
           &common->packed,
           &common->alpha_chnl,
           &common->red_chnl,
           &common->green_chnl,
           &common->blue_chnl);
}

void parse_page(string_buf buf, bmf_page_t *page)
{
    char *start = buf.data;
    char *end = buf.data;

    sscanf(start, "page id=%d", &page->id);

    start = strstr(buf.data, "file=");
    if (start != NULL)
    {
        start = start + 6;
        end = strchr(start, '"');
        page->file.data = start;
        page->file.len = end - start;
    }
}

void parse_char(string_buf buf, bmf_char_t *ch)
{
    // char id=32   x=51    y=165   width=2     height=32    xoffset=0     yoffset=0     xadvance=7     page=0  chnl=15
    const char *fmt = "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\r";
    sscanf(buf.data, fmt,
           &ch->id,
           &ch->x,
           &ch->y,
           &ch->width,
           &ch->height,
           &ch->xoffset,
           &ch->yoffset,
           &ch->xadvance,
           &ch->page,
           &ch->chnl);
}

void parse_alloc_chars_cnt(string_buf buf, bmf_fnt_t *fnt)
{
    sscanf(buf.data, " chars count=%d ", &fnt->chars_count);
    assert(fnt->chars_count > 0 && "invalid data");
    fnt->chars = (bmf_char_t *)calloc(fnt->chars_count, sizeof(*fnt->chars));
}

void parse_kerning(string_buf buf, bmf_kerning_t *kerning)
{
    // kerning first=32  second=65  amount=-1
    const char *fmt = "kerning first=%d second=%d amount=%d";
    sscanf(buf.data, fmt,
           &kerning->first,
           &kerning->second,
           &kerning->amount);
}

void parse_alloc_kernings_cnt(string_buf buf, bmf_fnt_t *fnt)
{
    sscanf(buf.data, " kernings count=%d ", &fnt->kernings_count);
    assert(fnt->kernings_count > 0 && "invalid data");
    fnt->kernings = (bmf_kerning_t *)calloc(fnt->kernings_count, sizeof(*fnt->kernings));
}

char *next_line(char *start, string_buf *line)
{
    char *end = strchr(start, '\n');
    line->data = start;
    line->len = end - start;
    return end;
}

void bmf_read_from_txt_file(const char *path, bmf_fnt_t *fnt)
{
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fnt->data = (char *)malloc(len + 1);
    fread(fnt->data, sizeof(char), len, fp);
    fnt->data[len] = '\0';
    fclose(fp);

    string_buf line = {0};

    char *end;
    end = next_line(fnt->data, &line);
    assert(line.len > 0 && "invalid data");
    parse_info(line, &fnt->info);

    end = next_line(end + 1, &line);
    assert(line.len > 0 && "invalid data");
    parse_common(line, &fnt->common);

    end = next_line(end + 1, &line);
    assert(line.len > 0 && "invalid data");
    parse_page(line, &fnt->page);

    end = next_line(end + 1, &line);
    assert(line.len > 0 && "invalid data");
    parse_alloc_chars_cnt(line, fnt);

    for (int i = 0; i < fnt->chars_count; i++)
    {
        end = next_line(end + 1, &line);
        assert(line.len > 0 && "invalid data");
        parse_char(line, &fnt->chars[i]);
    }

    end = next_line(end + 1, &line);
    if (line.len == 0)
    {
        printf("kering details not available!\n");
        return;
    }
    parse_alloc_kernings_cnt(line, fnt);

    for (int i = 0; i < fnt->kernings_count; i++)
    {
        end = next_line(end + 1, &line);
        assert(line.len > 0 && "invalid data");
        parse_kerning(line, &fnt->kernings[i]);
    }
}

void bmf_free(bmf_fnt_t *fnt)
{
    if (fnt == NULL)
        return;
    if (fnt->kernings != NULL)
        free(fnt->kernings);
    if (fnt->chars != NULL)
        free(fnt->chars);
    free(fnt->data);
}

void bmf_print(FILE *stream, bmf_fnt_t *fnt)
{
    bmf_info_t *info = &fnt->info;
    fprintf(stream, "face=\"%.*s\""
                    " size=%d bold=%d italic=%d charset=\"%.*s\" unicode=%d"
                    " stretchh=%d smooth=%d aa=%d padding=%d,%d,%d,%d"
                    " spacing=%d,%d outline=%d\n",
            (int)info->face.len, info->face.data,
            info->size,
            info->bold,
            info->italic,
            (int)info->charset.len, info->charset.data,
            info->unicode,
            info->stretchh,
            info->smooth,
            info->aa,
            info->padding.up, info->padding.right, info->padding.down, info->padding.left,
            info->spacing.horizontal, info->spacing.vertical,
            info->outline);

    {
        bmf_common_t *common = &fnt->common;
        const char *print_fmt = "common lineHeight=%d base=%d scaleW=%d scaleH%d pages=%d packed=%d alphaChnl=%d redChnl=%d greenChnl=%d blueChnl=%d\n";
        fprintf(stream, print_fmt, common->line_height,
                common->base,
                common->scale_w,
                common->scale_h,
                common->pages,
                common->packed,
                common->alpha_chnl,
                common->red_chnl,
                common->green_chnl,
                common->blue_chnl);
    }

    bmf_page_t *page = &fnt->page;
    fprintf(stream, "page id=%d file=\"%.*s\"\n", page->id, (int)page->file.len, page->file.data);

    {
        fprintf(stream, "chars count=%d\n", fnt->chars_count);
        const char *print_fmt = "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d\n";
        for (int i = 0; i < fnt->chars_count; i++)
        {
            bmf_char_t *ch = &fnt->chars[i];
            fprintf(stream, print_fmt,
                    ch->id,
                    ch->x,
                    ch->y,
                    ch->width,
                    ch->height,
                    ch->xoffset,
                    ch->yoffset,
                    ch->xadvance,
                    ch->page,
                    ch->chnl);
        }
    }

    {
        fprintf(stream, "kernings count=%d\n", fnt->kernings_count);
        const char *print_fmt = "kerning first=%d second=%d amount=%d\n";
        for (int i = 0; i < fnt->kernings_count; i++)
        {
            bmf_kerning_t *kerning = &fnt->kernings[i];
            fprintf(stream, print_fmt,
                    kerning->first,
                    kerning->second,
                    kerning->amount);
        }
    }
}

#endif //BMFNT_IMPLEMENTATION

#endif
