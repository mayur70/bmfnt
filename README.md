# bmfnt
header only bmfont fnt file parser written in c

basic implementation of [bmfont](https://angelcode.com/products/bmfont/doc/file_format.html)


usage
```c
#include <stdio.h>

#define BMFNT_IMPLEMENTATION
#include "bmfnt.h"

int main(int argc, char *argv[])
{
    bmf_fnt_t fnt = {0};
    bmf_read_from_txt_file("res/fonts/arial.fnt", &fnt);
    bmf_print(stdout, &fnt);
    bmf_free(&fnt);
    return 0;
}
```
