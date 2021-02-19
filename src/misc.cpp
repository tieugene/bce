/*
 * TODO: options:
 * - input bk no=>hash table
 */

#include <cstdlib>
#include <stdio.h>
#include <unistd.h>
#include <filesystem>
#include "bce.h"
#include "misc.h"
#include "script.h" // cur_addr only
#if defined(__APPLE__)
#include <mach/mach.h>
#endif

static string  help_txt = "\
Usage: [options] <dat_dir> <locs_file>\n\
Options:\n\
-h        - this help\n\
-f n      - block starts from (default=0)\n\
-n n      - blocks to process (default=1, 0=all)\n\
-k <path> - file-based key-value folder\n\
-m        - use in-mem key-value\n\
-o        - output results\n\
-v[n]     - verbose (to stderr):\n\
    0 - errors only (default)\n\
    1 - short info (default n)\n\
    2 - mid\n\
    3 - full debug\n\
<dat_dir> - blk*.dat folder\n\
<locs_file> - file with block locations\
";

void        __prn_opts(void)
{
    cerr
        << "Options:" << endl
        << TAB << "From:" << TAB << OPTS.from << endl
        << TAB << "Num:" << TAB << OPTS.num << endl
        << TAB << "Quiet:" << TAB << OPTS.out << endl
        << TAB << "Debug:" << TAB << OPTS.verbose << endl
        << TAB << "DatDir:" << TAB << OPTS.datdir << endl
        << TAB << "LocsFile:" << TAB << OPTS.locsfile << endl
        << TAB << "Cache:" << TAB << OPTS.cachedir << endl
        << TAB << "InMem:" << TAB << OPTS.inmem << endl
    ;
}

bool        cli(int argc, char *argv[])
{
    int opt;
    bool retvalue = false;

    OPTS.from = -1;
    OPTS.num = 1;
    OPTS.datdir = "";
    // OPTS.cachedir = ".";
    OPTS.inmem = false;
    OPTS.out = false;
    OPTS.verbose = DBG_NONE;
    while ((opt = getopt(argc, argv, "hf:n:k:mov::")) != -1)  // FIXME: v?
    {
        switch (opt) {
            case 'h':
                cerr << help_txt << endl;
                return false;
                break;
            case 'f':   // FIXME: optarg < 0 | > 999999
                OPTS.from = atoi(optarg); // FIXME: try digit_optarg
                if (OPTS.from < 0)
                    OPTS.from = 0;
                break;
            case 'n':   // FIXME: optarg < 1 | > 999999
                //OPTS.num = *optarg == '*' ? 999999 : atoi(optarg);
                OPTS.num = atoi(optarg);  // FIXME: try digit_optarg
                if (OPTS.num == 0)
                    OPTS.num = 999999;
                break;
            case 'k':
                OPTS.cachedir = optarg;
                OPTS.cash = !OPTS.cachedir.empty();
                break;
            case 'm':
                OPTS.inmem = true;
                break;
            case 'o':
                OPTS.out = true;
                break;
            case 'v':   // FIXME: optarg = 0..5
                OPTS.verbose = (optarg) ? (DBG_LVL_T)atoi(optarg) : DBG_MIN;
                break;
            case '?':   // can handle optopt
                cerr << help_txt << endl;
                return false;
        }
    }
    // opterr - allways 1
    // optind - 1-st unhandled is argv[optarg] (if argc > optind)
    if ((argc - optind) < 2)
      cerr << "Too fiew mandatory arguments. Use -h for help" << endl;
    else if (!filesystem::exists(argv[optind]))
        cerr << argv[optind] << " not exists" << endl;
    else if (!filesystem::exists(argv[optind + 1]))
        cerr << argv[optind + 1] << " not exists" << endl;
    else if (!OPTS.cachedir.empty() and !filesystem::exists(OPTS.cachedir))
      cerr << OPTS.cachedir << " not exists" << endl;
    else {
        OPTS.datdir = argv[optind];
        OPTS.locsfile = argv[optind + 1];
        retvalue = true;
        if (OPTS.verbose > 1)   // TODO: up v-level
            __prn_opts();
    }
    return retvalue;
}

long        get_statm(void)   ///< returns used memory in kilobytes
{
    long    total = 0;  // rss, shared, text, lib, data, dt; man proc
#if defined (__linux__)
    ifstream statm("/proc/self/statm");
    statm >> total; // >> rss...
    statm.close();
    total *= (sysconf(_SC_PAGE_SIZE) >> 10);  // pages-ze = 4k in F32_x64
#elif defined(__APPLE__)
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (KERN_SUCCESS == task_info(mach_task_self(),
        TASK_BASIC_INFO, (task_info_t)&t_info,
        &t_info_count))
        total = t_info.virtual_size >> 10;
#endif
    return total;
}

long        memused(void)
{
    return get_statm();
}

uint32_t    read_v(void)   ///<read 1..4-byte int and forward;
{
    auto retvalue = static_cast<uint32_t>(*CUR_PTR.u8_ptr++);
    if ((retvalue & 0xFC) == 0xFC) {
        switch (retvalue & 0x03) {
            case 0: // 0xFC
                break;
            case 1: // 0xFD
                retvalue = static_cast<uint32_t>(*CUR_PTR.u16_ptr++);
                break;
            case 2: // 0xFE
                retvalue = *CUR_PTR.u32_ptr++;
                break;
            case 3: // 0xFF
                throw "Value too big";
        }
    }
    return retvalue;
}

uint32_t    read_32(void)  ///< Read 4-byte int and go forward
{
    return *CUR_PTR.u32_ptr++;
}

uint64_t    read_64(void)  ///< Read 8-byte int and go forward
{
    return *CUR_PTR.u64_ptr++;
}

uint8_t     *read_u8_ptr(uint32_t const size)
{
    auto retvalue = CUR_PTR.u8_ptr;
    CUR_PTR.u8_ptr += size;
    return retvalue;
}

uint32_t    *read_32_ptr(void)
{
    return CUR_PTR.u32_ptr++;
}

uint256_t   *read_256_ptr(void)
{
    return CUR_PTR.u256_ptr++;
}

string      ptr2hex(void const *vptr, size_t const size)
{
    static string hex_chars = "0123456789abcdef";
    string s;
    auto cptr = reinterpret_cast<char const *>(vptr);
    for (size_t i = 0; i < size; i++, cptr++) {
        s.push_back(hex_chars[(*cptr & 0xF0) >> 4]);
        s.push_back(hex_chars[(*cptr & 0x0F)]);
    }
    return s;
}

string      str2hex(const string &s)
{
    return ptr2hex(s.c_str(), s.size());
}
