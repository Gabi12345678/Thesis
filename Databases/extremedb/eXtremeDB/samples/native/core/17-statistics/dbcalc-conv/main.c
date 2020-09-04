#include <common.h>
#include "mco.h"
#include "mcodbcalc.h"
#include "calc_db.h"

#define DEF_BLMIN 10
#define DEF_BLMAX 20
#define DEF_VLMIN 2
#define DEF_VLMAX 30
#define DEF_SLMIN 10
#define DEF_SLMAX 128


const char * db_name             = "testDB";

static char *appname = NULL;

#define CALC_PRPLAIN 0
#define CALC_PRGPLOT 1
#define CALC_PRLATEX 2
#define CALC_PRMAX   3

struct calc_options {
    int pg_size;
    int db_size;
    const char *db_name;
    int out;
    int help;
    int vlmin, vlmax;
    int slmin, slmax;
    int blmin, blmax;
};

MCO_RET calc_print_statistics(mco_calc_t *calc, int prn);
void calc_error(const char *fmt, ...);
int calc_getopt(int argc, char *argv[], struct calc_options *opts);
void set_disk_page_size(int size);

static MCO_RET plain_printer(mco_calc_t *calc);
static MCO_RET gnuplot_printer(mco_calc_t *calc);
static MCO_RET latex_printer(mco_calc_t *calc);

typedef MCO_RET (*printer_h)(mco_calc_t *);

static int _disk_page_size = 0;

static printer_h printers[] = {
    plain_printer,
    gnuplot_printer,
    latex_printer,
};

static const char *options[] = {
    "ps",
    "dbs",
    "db_name",
    "out",
    "vrange",
    "brange",
    "srange",
    "help"
};

#define OPT_PS     0
#define OPT_DBS    1
#define OPT_db_name 2
#define OPT_OUT    3
#define OPT_VRANGE 4
#define OPT_BRANGE 5
#define OPT_SRANGE 6
#define OPT_HELP   7
#define OPT_LAST   8

static int __getopt(const char *optname)
{
    int i;
    
    if (optname[0] != '-')
        return -1;

    for (i = 0; i < OPT_LAST; i++)
    {
        if (!strcmp(optname + 1, options[i]))
            return i;
    }

    return -1;
}

static void get_opt_range(const char *buf, int *min, int *max, int with_spec)
{    
    if (with_spec)
    {
        char c1, c2;
        
        if (sscanf(buf, "%d%c-%d%c", min, &c1, max, &c2) != 4)
        {
            *min = *max = -1;
            return;
        }
        if ((c1 == 'k') || (c1 == 'K'))
            *min <<= 10;
        if ((c2 == 'k') || (c2 == 'K'))
            *max <<= 10;
        if (((c1 != 'k') && (c1 != 'K')) && ((c1 != 'b') && (c1 != 'B'))
            && (c2 != 'k') && (c2 != 'K') && (c2 != 'b') && (c2 != 'B'))
        {
            *min = *max = -1;
        }
    }
    else
    {
        if (sscanf(buf, "%d-%d", min, max) != 2)
        {
            *min = *max = -1;
            return;
        }
    }
    if (*min > *max)
        *min = *max = -1;
}

static int get_opt_size(const char *sz)
{
    int ret;
    char c;

    if (sscanf(sz, "%d%c", &ret, &c) != 2)
        return -1;
    if ((c == 'B') || (c == 'b'))
        return ret;
    else if ((c == 'K') || (c == 'k'))
        return (ret << 10);
    else
        return -1;
}

static int get_opt_out(const char *outs)
{
    if (!strcmp(outs, "plain"))
        return CALC_PRPLAIN;
    else if (!strcmp(outs, "gnuplot"))
        return CALC_PRGPLOT;
    else if (!strcmp(outs, "latex"))
        return CALC_PRLATEX;
    else
        return -1;
}

static char *get_idx_type(uint4 type)
{
    switch(type & MCO_IDXST_FUNCTION_MASK) {
        case MCO_IDXST_FUNCTION_REGULAR:
            return "Regular";
        case MCO_IDXST_FUNCTION_OID:
            return "OID";
        case MCO_IDXST_FUNCTION_AUTOOID:
            return "AUTOOID";
        case MCO_IDXST_FUNCTION_AUTOID:
            return "AutoID";
        case MCO_IDXST_FUNCTION_LIST:
            return "List";
        default:
            return "Unknown";
    }
}

static void plain_class_print(mco_calc_t *calc, mco_cc_t *cls,
                              mco_cc_info_t *info, void *unused)
{
    uint8 tmp, tmp1, tmp2;
    char c;
    
    printf(" -> %s%s:\n", (info->is_pers ? "[PERSISTENT] " : ""), cls->cc_name);
    printf("   * %-20s %8d\n", "Objects:", info->nobjs);
    printf("   * %-20s %8ld\n", "Time:", (long)cls->cc_time);
    if (info->nvecs)
        printf("   * %-20s %8d\n", "Vectors:", info->nvecs);
    if (info->nstrs)
        printf("   * %-20s %8d\n", "Strings:", info->nstrs);
    if (info->nblobs)
        printf("   * %-20s %8d\n", "Blobs:", info->nblobs);
    
    printf("   * %-20s %8d\n", "Pages min:", info->pages_min);
    printf("   * %-20s %8d\n", "Pages cur:", info->pages_cur);
    printf("   * %-20s %8d\n", "Pages max:", info->pages_max);
    tmp = info->bytes_min;
    tmp1 = info->bytes_cur;
    tmp2 = info->bytes_max;
    if (tmp > 0x400) {
        tmp >>= 10;
        tmp1 >>= 10;
        tmp2 >>= 10;
        c = 'K';
    }
    else
        c = 'B';

    printf("   * %-20s %8lld%c\n", "Space min:", tmp, c);
    printf("   * %-20s %8lld%c\n", "Space cur:", tmp1, c);
    printf("   * %-20s %8lld%c\n", "Space max:", tmp2, c);    
}

static void plain_index_print(mco_calc_t * calc, mco_index_stat_t *istat, void *unused)
{
    int tmp, psize = 0;
    char *str = NULL, c;
        
    if ((istat->type & MCO_IDXST_TYPE_MASK) == MCO_IDXST_TYPE_MEM) {
        str = "Inmem";
        psize = calc->pg_size;
    }
    else if ((istat->type & MCO_IDXST_TYPE_MASK) == MCO_IDXST_TYPE_DISK) {
        str = "Disk";
        psize = _disk_page_size;
    }
        
    printf(" -> %s [%s ", istat->plabel, str);
    if (istat->type & MCO_IDXST_FEATURE_UNIQUE)
        printf("unique ");
    if (istat->type & MCO_IDXST_FEATURE_UDF)
        printf("userdef ");

    tmp = 0;
    switch (istat->type & MCO_IDXST_NATURE_MASK)
    {
        case MCO_IDXST_NATURE_BTREE:
            tmp = istat->spec.btree.levels_num;
            printf("btree");
            break;
        case MCO_IDXST_NATURE_PTREE:
            tmp = istat->spec.ptree.levels_num;
            printf("ptree");
            break;
        case MCO_IDXST_NATURE_RTREE:
            tmp = istat->spec.rtree.levels_num;
            printf("rtree");
            break;
        case MCO_IDXST_NATURE_HASH:
            printf("hash");
            break;
        case MCO_IDXST_NATURE_META:
            printf("meta");
            break;
        default:
            printf("unknown");
    }

    printf("]\n");
    printf("   * %-20s        %s\n", "Type:", get_idx_type(istat->type));
    switch (istat->type & MCO_IDXST_NATURE_MASK)
    {
        case MCO_IDXST_NATURE_BTREE:
        case MCO_IDXST_NATURE_PTREE:
        case MCO_IDXST_NATURE_RTREE:
        {
            printf("   * %-20s %8d\n", "Levels:", tmp);
            break;
        }
        case MCO_IDXST_NATURE_HASH:
            printf("   * %-20s %8ld\n", "Avg chain length:",
                   (long)istat->spec.hash.avg_chain_length);
            printf("   * %-20s %8ld\n", "Max chain length:",
                   (long)istat->spec.hash.max_chain_length);
            break;
    }

    printf("   * %-20s %8ld\n", "Keys:", (long)istat->keys_num);
    printf("   * %-20s %8ld\n",
           (((istat->type & MCO_IDXST_TYPE_MASK) != MCO_IDXST_TYPE_DISK) ? "Pages:" : "Disk pages:"),
           (long)istat->pages_num);
    tmp = istat->pages_num * psize;
    if (tmp >= 0x400)
    {
        tmp >>= 10;
        c = 'K';
    }
    else
        c = 'B';

    printf("   * %-20s %8d%c\n", "Space:", tmp, c);
}

static MCO_RET plain_printer(mco_calc_t *calc)
{
    MCO_RET rc = MCO_S_OK;
    int tmp;
    char c;

    printf("== Statistics ==\n");
    printf("INMEM DB information:\n");
    printf(" * %-23s %8d\n", "Page size:", calc->pg_size);
    printf(" * %-23s %8d\n", "Total pages:", calc->total_pgs);
    printf(" * %-23s %8d\n", "Free pages:", calc->free_pgs);
    printf(" * %-23s %8d\n", "Classes:", calc->ncls);
    tmp = (calc->total_pgs - calc->free_pgs) * calc->pg_size;
    if (tmp > 0x400)
    {
        tmp >>= 10;
        c = 'K';
    }
    else
        c = 'B';
    
    printf(" * %-23s %7d%c\n", "Used:", tmp, c);
    printf("DB classes:\n");
    rc = mco_calc_cinfo_browse(calc, plain_class_print, NULL);
    if (rc != MCO_S_OK)
        return rc;

    printf("DB indices:\n");
    rc = mco_calc_iinfo_browse(calc, plain_index_print, NULL);

    printf("\n\nOptimal page size: %d bytes\n", calc->ops);
    return rc;
}

struct gnuplot_private {
    long regularsb;
    long oidsb;
    long autooidsb;
    long listsb;
    long autoidsb;
};

static void gnuplot_index_print(mco_calc_t * calc, mco_index_stat_t *istat, void *priv)
{
    struct gnuplot_private *gp = (struct gnuplot_private *)priv;
    long tmp = (long)(istat->pages_num * calc->pg_size);
    
    switch(istat->type & MCO_IDXST_FUNCTION_MASK)
    {
        case MCO_IDXST_FUNCTION_REGULAR:
            gp->regularsb += tmp;
            break;
        case MCO_IDXST_FUNCTION_OID:
            gp->oidsb += tmp;
            break;
        case MCO_IDXST_FUNCTION_AUTOOID:
            gp->autooidsb += tmp;
            break;
        case MCO_IDXST_FUNCTION_AUTOID:
            gp->autoidsb += tmp;
            break;
        case MCO_IDXST_FUNCTION_LIST:
            gp->listsb += tmp;
            break;
    }
}

static void gnuplot_class_print(mco_calc_t *calc, mco_cc_t *cls,
                                mco_cc_info_t *info, void *unused)
{
    printf("%s(%d)\t%lld\t%lld\t%lld\n", cls->cc_name, info->nobjs,
           info->bytes_min, info->bytes_cur, info->bytes_max);
}

static MCO_RET gnuplot_printer(mco_calc_t *calc)
{
    MCO_RET rc = MCO_S_OK;
    struct gnuplot_private priv;

    printf("Name\tMin\tUsed\tMax\n");
    printf("####\t###\t####\t###\n");
    rc = mco_calc_cinfo_browse(calc, gnuplot_class_print, NULL);
    if (rc != MCO_S_OK)
        return rc;

    memset(&priv, 0, sizeof(priv));
    rc = mco_calc_iinfo_browse(calc, gnuplot_index_print, (void *)&priv);
    if (rc != MCO_S_OK)
        return rc;

    if (priv.regularsb)
        printf("Regulars(idx)\t0\t%ld\t0\n", priv.regularsb);
    if (priv.oidsb)
        printf("OIDs\t0\t%ld\t0\n", priv.oidsb);
    if (priv.autooidsb)
        printf("AUTOOIDs\t0\t%ld\t0\n", priv.autooidsb);
    if (priv.autoidsb)
        printf("AutoIDs\t0\t%ld\t0\n", priv.autoidsb);
    if (priv.listsb)
        printf("Lists\t0\t%ld\t0\n", priv.listsb);
    
    return rc;
}

static void latex_index_print(mco_calc_t * calc, mco_index_stat_t *istat, void *unused)
{
    char *str = NULL, c;
    int tmp;
        
    switch (istat->type & MCO_IDXST_NATURE_MASK)
    {
        case MCO_IDXST_NATURE_BTREE:
            str = "btree";
            break;
        case MCO_IDXST_NATURE_PTREE:
            str = "ptree";
            break;
        case MCO_IDXST_NATURE_RTREE:
            str = "rtree";
            break;
        case MCO_IDXST_NATURE_HASH:
            str = "hash";
            break;
        case MCO_IDXST_NATURE_META:
            str = "meta";
            break;
        default:
            printf("unknown");
    }

    printf("\\verb+%s+ & %s & %s & %ld & %ld &",
           istat->plabel, get_idx_type(istat->type), str,
           (long)istat->keys_num, (long)istat->pages_num);
    if ((istat->type & MCO_IDXST_TYPE_MASK) == MCO_IDXST_TYPE_DISK)
        tmp = istat->pages_num * _disk_page_size;
    else
        tmp = istat->pages_num * calc->pg_size;
    if (tmp >= 0x400) {
        tmp >>= 10;
        c = 'K';
    }
    else
        c = 'B';

    printf("%d%c \\\\\n", tmp, c);
}

static void latex_class_print(mco_calc_t *calc, mco_cc_t *cls,
                              mco_cc_info_t *info, void *unused)
{
    uint8 tmp1, tmp2, tmp3;
    char c;

    
    printf("\\verb+%s+ & %d & %ld & %d & %d & %d & ", cls->cc_name, info->nobjs,
           (long)cls->cc_time, info->pages_min, info->pages_cur, info->pages_max);

    tmp1 = info->bytes_min;
    tmp2 = info->bytes_cur;
    tmp3 = info->bytes_max;
    if (tmp1 > 0x400)
    {
        c = 'K';
        tmp1 >>= 10;
        tmp2 >>= 10;
        tmp3 >>= 10;
    }
    else
        c = 'B';

    printf("%lld%c & %lld%c & %lld%c & %d & %d & %d \\\\\n",
           tmp1, c, tmp2, c, tmp3, c, info->nvecs, info->nstrs,
           info->nblobs);
}

static MCO_RET latex_printer(mco_calc_t *calc)
{
    MCO_RET rc = MCO_S_OK;

    printf("\\documentstyle[11pt]{article}\n");
    printf("\\setlength{\\topmargin}{-.5in}\n");
    printf("\\setlength{\\textheight}{9in}\n");
    printf("\\setlength{\\oddsidemargin}{.125in}\n");
    printf("\\setlength{\\textwidth}{6.25in}\n");
    printf("\\begin{document}\n");
    printf("\\title{Database statistics}\n");
    printf("\\maketitle\n\n");

    printf("%%%%%% MAIN INFORMATION TABLE START %%%%%%\n");
    printf("\\begin{tabular}{|l|l|}\n");
    printf("\\hline\n");
    printf("\\multicolumn{2}{|c|}{\\textbf{Main information}} \\\\\n");
    printf("\\hline\n");
    printf("Page size & %d \\\\\n", calc->pg_size);
    printf("Total pages & %d \\\\\n", calc->total_pgs);
    printf("Free pages & %d \\\\\n", calc->free_pgs);
    printf("Classes & %d \\\\\n", calc->ncls);
    printf("\\hline\n");
    printf("\\end{tabular} \\\\\n");
    printf("%%%%%% MAIN INFORMATION TABLE END %%%%%%\n\n");

    printf("%%%%%% CLASSES TABLE START %%%%%%\n");
    printf("\\begin{tabular}{|l|l|l||l|l|l||l|l|l||l|l|l|}\n");
    printf("\\hline\n");
    printf("\\multicolumn{12}{|c|}{\\textbf{Classes information}} \\\\\n");
    printf("\\hline\n");
    printf("\\textbf{Name}&\\textbf{Objects}&\\textbf{Time}&\\multicolumn{3}{l||}{\\textbf{Pages}}&"
           "\\multicolumn{3}{l||}{\\textbf{Space}}&\\multicolumn{3}{l|}{\\textbf{Dyn. Items}} \\\\\n");
    printf("\\cline{4-12}\n");
    printf("&&&Min&Cur&Max&Min&Cur&Max&Vecs&Strs&Blobs \\\\\n");
    printf("\\hline\n");    
    
    rc = mco_calc_cinfo_browse(calc, latex_class_print, NULL);
    if (rc != MCO_S_OK)
        return rc;

    printf("\\end{tabular} \\\\\n");
    printf("%%%%%% CLASSES TABLE END %%%%%%\n\n");

    printf("%%%%%% INDICES TABLE START %%%%%%\n");
    printf("\\begin{tabular}{|l|l|l|l|l|l|}\n");
    printf("\\hline\n");
    printf("\\multicolumn{6}{|c|}{\\textbf{Indices information}} \\\\\n");
    printf("\\hline\n");
    printf("\\textbf{Name}&\\textbf{Type}&\\textbf{Structure}&"
           "\\textbf{Keys}&\\textbf{Pages}&\\textbf{Space} \\\\\n");        

    rc = mco_calc_iinfo_browse(calc, latex_index_print, NULL);
    if (rc != MCO_S_OK)
        return rc;
    
    printf("\\hline\n");
    printf("\\end{tabular} \\\\\n");
    printf("%%%%%% INDICES TABLE END %%%%%%\n\n");
    printf("\\end{document}\n");
    return rc;
}

MCO_RET calc_print_statistics(mco_calc_t *calc, int prn)
{
    if ((prn < 0) || (prn >= CALC_PRMAX))
        return MCO_S_OK;

    return printers[prn](calc);
}

void calc_error(const char *fmt, ...)
{
  /* @rw 090817: comment out this function because va_start and va_end not found...
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
  */
    sample_os_shutdown();
    dbg_exit(EXIT_FAILURE);
}

void set_disk_page_size(int size)
{
    _disk_page_size = size;
}

int calc_getopt(int argc, char *argv[], struct calc_options *opts)
{
    int o;
    int i = 0;

    while (++i < argc)
    {
        o = __getopt(argv[i]);
        if (o == OPT_HELP)
        {
            opts->help = 1;
            return 0;
        }
        if ((i + 1) >= argc)
            return -1;
                
        switch (o)
        {
            case OPT_db_name:
                opts->db_name = argv[++i];
                break;
            case OPT_DBS:
                opts->db_size = get_opt_size(argv[++i]);
                break;
            case OPT_PS:
                opts->pg_size = get_opt_size(argv[++i]);
                break;

            case OPT_OUT:
                opts->out = get_opt_out(argv[++i]);
                break;
            case OPT_VRANGE:
                get_opt_range(argv[++i], &opts->vlmin, &opts->vlmax, 0);
                break;
            case OPT_SRANGE:
                get_opt_range(argv[++i], &opts->slmin, &opts->slmax, 1);
                break;
            case OPT_BRANGE:
                get_opt_range(argv[++i], &opts->blmin, &opts->blmax, 1);
                break;
            default:
                return -1;
        }
    }

    return 0;
}

static void usage(void)
{
    int len = strlen(appname);
    
    printf("Usage: %s [-ps INT<b|k>] [-dbs INT<b|k>] [-db_name STR] [-out STR]\n", appname);
    printf("%*s       [-vrange min-max] [-brange min<b|k>-max<b|k>] [-srange min<b|k>-max<b|k>]\n",
           len, "");
    printf("%*s -ps      Is a page size in bytes(if there is B or b letter after an integer)\n", len, "");
    printf("%*s          or in kilobytes(if there is K or k letter after an integer)\n", len, "");
    printf("%*s          Default size: %dB\n", len, "", MEMORY_PAGE_SIZE);
    printf("%*s -dbs     Database size. It is an integer value with (b|B) or (k|K) size specificator after it\n", len, "");
    printf("%*s          determining how much memory will be allocated for the database\n", len, "");
    printf("%*s          Default size: %dK\n", len, "", (DATABASE_SIZE >> 10));
    printf("%*s -db_name  Database name. Default name: \"%s\"\n", len, "", db_name);
    printf("%*s -out     Specifies a type of data output. There are three available types:\n", len, "");
    printf("%*s           plain   - human readable plain text format (default)\n", len, "");
    printf("%*s           gnuplot - formatted GNUPLOT adapted output\n", len, "");
    printf("%*s           latex -   output in LaTeX format\n", len, "");
    printf("%*s -vrange  Specifies min and max vectors length(format: min_len-max_len).\n", len, "");
    printf("%*s          default values: %d-%d\n", len, "", DEF_VLMIN, DEF_VLMAX);
    printf("%*s -srange  Specifies min and max strings length in bytes(b|B) or kilobytes(k|K).\n", len, "");
    printf("%*s          default values: %db-%db\n", len, "", DEF_SLMIN, DEF_SLMAX);
    printf("%*s -brange  Specifies min and max range of blob fields in bytes(b|B) or kilobytes(k|K).\n", len, "");
    printf("%*s          default values: %db-%db\n", len, "", DEF_BLMIN, DEF_BLMAX);
    printf("%*s -help    Show this help message\n", len, "");
    dbg_exit(EXIT_SUCCESS);
}

static void recognize_options(struct calc_options *opts)
{
    if (opts->help)
        usage();
    if (!opts->db_name)
        opts->db_name = db_name;
    if (opts->pg_size < 0)
        usage();
    else if (!opts->pg_size || (opts->pg_size < MEMORY_PAGE_SIZE))
        opts->pg_size = MEMORY_PAGE_SIZE;
    if (opts->db_size < 0)
        usage();
    else if (!opts->db_size)
        opts->db_size = DATABASE_SIZE;
    if (opts->out < 0)
        usage();
    if ((opts->blmin < 0) || (opts->blmax < 0) ||
        (opts->vlmin < 0) || (opts->vlmax < 0) ||
        (opts->slmin < 0) || (opts->slmax < 0))
        usage();
    if (!opts->blmin && !opts->blmax)
    {
        opts->blmin = DEF_BLMIN;
        opts->blmax = DEF_BLMAX;
    }
    if (!opts->vlmin && !opts->vlmax)
    {
        opts->vlmin = DEF_VLMIN;
        opts->vlmax = DEF_VLMAX;
    }
    if (!opts->slmin && !opts->slmax)
    {
        opts->slmin = DEF_SLMIN;
        opts->slmax = DEF_SLMAX;
    }
}

void error_handler(MCO_RET n) 
{
    fprintf(stderr, "eXtremeDB runtime fatal error: %d\n", n);
    sample_os_shutdown();
    dbg_exit( - 1);
}


int main(int argc, char *argv[])
{
    MCO_RET rc;
    mco_calc_t calc;
    struct calc_options opts;
    sample_memory_t dbmem;

    sample_os_initialize(DEFAULT);

    memset(&opts, 0, sizeof(opts));
    appname = argv[0];
    if (calc_getopt(argc, argv, &opts) < 0) {
        usage();
    }
    recognize_options(&opts);

    mco_error_set_handler(&error_handler);

    rc = mco_runtime_start();
    
    mco_calc_init(&calc, calc_db_get_dictionary());
    
    calc.dsl.v_lmin = opts.vlmin;
    calc.dsl.v_lmax = opts.vlmax;
    calc.dsl.s_lmin = opts.slmin;
    calc.dsl.s_lmax = opts.slmax;
    calc.dsl.b_lmin = opts.blmin;
    calc.dsl.b_lmax = opts.blmax;
    
    printf("== MCO eXtremeDB calculation tool ==\n");
    printf("type \"%s -help\" to get more configuration information\n\n", appname);
    
    /* Create the database  */
    rc = sample_open_database( opts.db_name, calc_db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
    if (rc) {
        printf("\nerror %d creating database", rc);
        sample_os_shutdown();
        dbg_exit(1);
    }
    rc = mco_db_connect(opts.db_name, &calc.db);
    if (rc != MCO_S_OK)
        calc_error("mco_db_connect failed. (MCO_RET = %d)", rc);

    /*
     * Note:
     * mco_calc_reg_schema_classes function expects that your
     * schema contains "schema-calculator" construction which
     * includes a set of classes that can be registered in calculator
     * with mentioned above function.
     *
     * But it's not necessary to use schema built-in calculator.
     * You can register needful classes in your code using either
     * mco_calc_class_reg function to register your classes
     * dynamically or you can use MCO_CALC_DEFINE_CLASS construction
     * which is used for static class definition(it doesn't allocate dynamic memory)
     *
     * example 1) mco_calc_class_reg:
     *  MCO_RET rc;
     *  rc = mco_calc_class_reg(calc_ref, class_name, number_of_objects);
     *  rc |= mco_calc_class_reg(calc_ref, another_class_name, number_of_objects);
     *  ...
     *
     * example 2) MCO_CALC_DEFINE_CLASS
     * static mco_cc_t my_classes[] = {
     *    MCO_CALC_DEFINE_CLASS(class_name1, number_of_objects),
     *    ...,
     *    MCO_CALC_DEFINE_CLASS(class_nameN, number_of_objects),
     *    MCO_CALC_CLASSES_END,
     * };
     * int main(void)
     * {
     *   ...
     *   rc = mco_calc_register_classes(calc_ref, my_classes);
     * }
     *
     * Where calc_ref is a reference to the mco_calc_t object, class_name is
     * a name of class *without* quotes and number_of_objects is(obviously)
     * a number of objects of a given class to create.
     */ 
    rc = mco_calc_reg_schema_classes(&calc, calc_db_get_calculator());
    if (rc != MCO_S_OK)
        calc_error("failed to register one of classes (MCO_RET = %d)", rc);

    rc = mco_calc_fill_db(&calc);
    if (rc != MCO_S_OK)
        calc_error("failed to fill database (MCO_RET = %d)", rc);
    
    rc = mco_calc_stat_collect(&calc);
    if (rc != MCO_S_OK)
        calc_error("failed to collect database statistics (MCO_RET = %d)", rc);
    
    rc = calc_print_statistics(&calc, opts.out);
    if (rc != MCO_S_OK)
        calc_error("failed to display statistics (MCO_RET = %d)", rc);

    /* NOTE: calculator *must* be deinitialized before you disconnect from the database */
    mco_calc_deinit(&calc);
    mco_db_disconnect(calc.db);
    sample_close_database(opts.db_name, &dbmem);
    mco_runtime_stop();

    printf("\nPress Enter to dbg_exit");
    getchar();

    sample_os_shutdown();
    dbg_exit(EXIT_SUCCESS);
}
