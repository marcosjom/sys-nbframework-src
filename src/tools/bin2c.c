//
//  main.c
//  bin2c - Convert a binary file to a C array for inclusion in programs
//
//  Created by Stéphane Peter on 10/21/13.
//  Copyright (c) 2013 Catloaf Software, LLC. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    VTChar,
    VTDString,
    VTNSString
} VariableType;

typedef struct
{
    VariableType vtype;
    const char* prefix;
    int null_terminate;
    int line_len;
} OutOptions;

static void print_usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [-i inputfile] [-o output.h] [-l line_len] [-t type] [-p var_prefix] [-0h] -a var_name\n"
            "\t-0 add a null-char at the end of the generated array; only valid with -t char\n"
            "\t-h show this help\n"
            "\ttype can be: char (unsigned char array, default), nsstring (Objective-C NSString constant), dstring (D string)\n",
            argv0);
    exit(1);
}

static void print_d_str (FILE* in, FILE* out, const char* array, const OutOptions* opts)
{
    unsigned char c;
    unsigned int l = 0;

    fprintf(out, "%sstring %s = \n\t\"", opts->prefix, array);

    c = fgetc(in);
    while (!feof(in)) {
        l++;
        fprintf(out, "\\x%02x", c);
        if ((l % opts->line_len) == 0) {
            fprintf(out, "\"~\n\t\"");
        }
        c = fgetc(in);
    }
    fprintf(out, "\";\n");
}

static void print_c_str (FILE* in, FILE* out, const char* array, const OutOptions* opts)
{
    unsigned char c;
    unsigned int l = 0;
    int nullchar_printed = !opts->null_terminate;
    int eof;

    fprintf(out, "%sunsigned char %s[] = {", opts->prefix, array);

    c = fgetc(in);
    eof = feof(in);
    while (!eof || !nullchar_printed) {
        if (eof) {
            c = 0;
            nullchar_printed = 1;
        }

        if ((l % opts->line_len) == 0) {
            fprintf(out, "\n\t");
        }
        l++;
        fprintf(out, "0x%02x", c);
        c = fgetc(in);

        eof = feof(in);
        if (!eof || !nullchar_printed) {
            fprintf(out, ",");
        }
    }
    if (l) {
        fprintf(out, "\n");
    }
    fprintf(out, "};\n\n");
    fprintf(out, "%sunsigned int %s_len = %d;\n\n", opts->prefix, array, l);
}

static void print_nsstring (FILE* in, FILE* out, const char* array, const OutOptions* opts)
{
    unsigned char c;
    unsigned int l = 0;

    fprintf(out, "%sNSString *%s = \n\t@\"", opts->prefix, array);

    c = fgetc(in);
    while (!feof(in)) {
        l++;
        fprintf(out, "\\x%02x", c);
        if ((l % opts->line_len) == 0) {
            fprintf(out, "\"\n\t\"");
        }
        c = fgetc(in);
    }
    fprintf(out, "\";\n");
}

char* append_space_ifn (char* in)
{
    const size_t len = strlen(in);
    char *new_mem;

    if (in && len) {
        switch (in[len - 1]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
            return in;

        default:
            new_mem = (char*)realloc(in, len + 1 + 1);
            if (new_mem != NULL) {
                new_mem[len] = ' ';
                new_mem[len + 1] = '\0';
                return new_mem;
            }
        }
    }
    return in;
}

void bin2c(const char *infile, const char *outfile, const char *array, const OutOptions* opts)
{
    FILE *in = NULL, *out = NULL;
    //
    if (infile) {
        fopen_s(&in, infile, "rb");
    } else {
        in = stdin;
    }
    if (in) {
        if (outfile) {
            fopen_s(&out, outfile, "w");
        } else {
            out = stdout;
        }
        if (out) {
            if (infile) {
                fprintf(out, "// Imported from file '%s'\n\n", infile);
                if (array != NULL) {
                    fprintf(out, "#ifndef %s_h\n", array);
                    fprintf(out, "#define %s_h\n\n", array);
                }
            }
            if (opts->vtype == VTChar) {
                print_c_str(in, out, array, opts);
            } else if (opts->vtype == VTDString) {
                print_d_str(in, out, array, opts);
            } else { // NSString
                print_nsstring(in, out, array, opts);
            }
            if (infile && array != NULL) {
                fprintf(out, "#endif //%s_h\n\n", array);
            }
            if (out != stdout)
                fclose(out);
        } else {
            fprintf(stderr, "Failed to open file for writing: %s", outfile);
        }
        if (in != stdin)
            fclose(in);
    } else{
        fprintf(stderr, "Failed to open file for reading: %s", infile);
    }
}

int main(int argc,  char * const argv[])
{
    const char *infile = NULL, *outfile = NULL, *array = NULL;
    const char *prefix = NULL;
    OutOptions opts;
    int iArg, opt;

    opts.vtype = VTChar;
    opts.line_len = 80;
    opts.null_terminate = 0;

    for (iArg = 0; iArg < argc; iArg++) {
        const char* arg = argv[iArg];
        if (0 == strcmp(arg, "-i")) {
            if ((iArg + 1) < argc) {
                infile = _strdup(argv[++iArg]);
            }
        } else if (0 == strcmp(arg, "-o")) {
            if ((iArg + 1) < argc) {
                outfile = _strdup(argv[++iArg]);
            }
        } else if (0 == strcmp(arg, "-a")) {
            if ((iArg + 1) < argc) {
                array = _strdup(argv[++iArg]);
            }
        } else if (0 == strcmp(arg, "-l")) {
            if ((iArg + 1) < argc) {
                opts.line_len = atoi(argv[++iArg]);
            }
        } else if (0 == strcmp(arg, "-t")) {
            if ((iArg + 1) < argc) {
                const char* arg2 = argv[++iArg];
                if (0 == strcmp(arg2, "char")) {
                    opts.vtype = VTChar;
                } else if (0 == strcmp(arg2, "nsstring")) {
                    opts.vtype = VTNSString;
                } else if (0 == strcmp(arg2, "dstring")) {
                    opts.vtype = VTDString;
                } else {
                    print_usage(argv[0]);
                }

            }
        } else if (0 == strcmp(arg, "-0h")) {
            opts.null_terminate = 1;
        } else if (0 == strcmp(arg, "-p")) {
            if ((iArg + 1) < argc) {
                append_space_ifn(_strdup(argv[++iArg]));
            }
        } else if (0 == strcmp(arg, "-h")) {
            print_usage(argv[0]);
        }
    }
    //
    if (argc == 0) {
        print_usage(argv[0]);
    }

    /*
    while( (opt = getopt(argc, argv, "i:o:a:l:t:p:0h")) != -1) {
        switch (opt) {
            case 'i':
                infile = _strdup(optarg);
                break;
            case 'o':
                outfile = _strdup(optarg);
                break;
            case 'a':
                array = _strdup(optarg);
                break;
            case 'l':
                opts.line_len = atoi(optarg);
                break;
            case 't':
                if (!strcmp(optarg, "char")) {
                    opts.vtype = VTChar;
                } else if (!strcmp(optarg, "nsstring")) {
                    opts.vtype = VTNSString;
                } else if (!strcmp(optarg, "dstring")) {
                    opts.vtype = VTDString;
                } else {
                    print_usage(argv[0]);
                }
                break;
            case '0':
                opts.null_terminate = 1;
                break;
            case 'p':
                prefix = append_space_ifn(_strdup(optarg));
                break;
            case 'h':
            default:
                print_usage(argv[0]);
                break;
        }
    }*/

    opts.prefix = (prefix ? prefix : "const ");

    if (!array) {
        print_usage(argv[0]);
    } else {
        bin2c(infile, outfile, array, &opts);
    }

    return 0;
}