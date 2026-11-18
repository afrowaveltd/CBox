#ifndef CBOX_STYLE_H
#define CBOX_STYLE_H

#define RESET "\x1b[0m"
#define BOLD "\x1b[1m"
#define DIM "\x1b[2m"
#define ITALIC "\x1b[3m"
#define UNDERLINE "\x1b[4m"
#define STRIKETHROUGH "\x1b[9m"
#define INVERSE "\x1b[7m"
#define NO_BOLD "\x1b[21m"
#define NO_ITALIC "\x1b[23m"
#define NO_UNDERLINE "\x1b[24m"
#define NO_STRIKETHROUGH "\x1b[29m"
#define NO_INVERSE "\x1b[27m"
#define NO_DIM "\x1b[22m"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define CYAN "\x1b[36m"
#define BLACK "\x1b[30m"
#define MAGENTA "\x1b[35m"
#define WHITE "\x1b[37m"
#define DEFAULT_FOREGROUND "\x1b[39m"

#define BACKGROUND_BLACK "\x1b[40m"
#define BACKGROUND_RED "\x1b[41m"
#define BACKGROUND_GREEN "\x1b[42m"
#define BACKGROUND_YELLOW "\x1b[43m"
#define BACKGROUND_BLUE "\x1b[44m"
#define BACKGROUND_MAGENTA "\x1b[45m"
#define BACKGROUND_CYAN "\x1b[46m"
#define BACKGROUND_WHITE "\x1b[47m"
#define BACKGROUND_DEFAULT "\x1b[49m"

#define THICK_LINE "========================================"
#define THIN_LINE "-----------------------------------------"

#endif // CBOX_STYLE_H
