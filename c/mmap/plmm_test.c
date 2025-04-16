
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long int   uint64_t;

typedef uint32_t    Ss_iobref ;
typedef uint64_t    Plmm_bm;        


/* page */
#define PAGE_SHIFT              12
#define PAGE_SIZE               (1UL << PAGE_SHIFT)

// TODO:
#define MULTI_INSTANCE

#define SS_INVALID_PAGENUM      0x00ffffff
#define SS_PAGES_PER_IOB        8
#define SS_INVALID_IOBREF       0x00ffffff
#define SS_PAGE_SIZE            4096

#define PLMM_MAX_BUSADDRESS     (1<<27)

#define PLMM_POOL_ID_HARD_LIMIT 64
#define PLMM_COMPONENT_MAX      64
#define PLMM_MAX_PROGID_LENGTH  80
#define PLMM_JOURNAL_MAX_SLOTS  16
#define PLMM_MAX_ASSERT_LENGTH  512


/* multiple instance*/
#define PLMM_INSTANCE_MASK      0xffffff
#define PLMM_INSTANCE_SHIFT     24
#define PLMM_MAX_INSTANCE       2

/* useful IOB numbers */
#define PLMM_PD_IOB                 0x00000000
#define PLMM_IT_IOB                 0x00000001
#define PLMM_FPS_IOB                0x00000002
#define PLMM_ANCHOR_IOB             0x00000003
#define PLMM_PD_EXTRA_IOBS          0x00000004
#define PLMM_IT_EXTRA_IOBS          0x00001000
#define PLMM_FPS_EXTRA_IOBS         0x00000803
#define PLMM_GEN_IOBS               0x00008000
#define PLMM_GEN_EXTRA_IOBS         0x01000000
#define PLMM_PD_EXTRA_EXTRA_IOBS    0x1fef8000
#define PLMM_IT_EXTRA_EXTRA_IOBS    0x1ff27000
#define PLMM_FPS_EXTRA_EXTRA_IOBS   0x1ff17800
#define PLMM_GEN_IOBS_OFFSET        0x110000

/* IOCTL numbers */
#define PLMM_IOC                    'k'
#define PLMM_QUERYVERSION           _IOR(PLMM_IOC, 0, uint32_t)
#define PLMM_QUERYNUMBEROFPAGES     _IOR(PLMM_IOC, 1, uint32_t)
#define PLMM_QUERYNUMBEROFPDPAGES   _IOR(PLMM_IOC, 2, uint32_t)
#define PLMM_QUERYNUMBEROFITPAGES   _IOR(PLMM_IOC, 3, uint32_t)
#define PLMM_QUERYNUMBEROFFPSPAGES  _IOR(PLMM_IOC, 4, uint32_t)
#define PLMM_QUERYNUMBEROFGENPAGES  _IOR(PLMM_IOC, 5, uint32_t)
#define PLMM_QUERYFIRSTFREEIOB      _IOR(PLMM_IOC, 6, uint32_t)
#define PLMM_QUERYANCHORPAGE        _IOR(PLMM_IOC, 7, uint32_t)

#define PLMM_MQMMAPCTRL             _IOR(PLMM_IOC, 44, uint32_t)
#define PLMM_QUERYNUMINSTANCE       _IOR(PLMM_IOC, 48, uint32_t)


#define MQ_MMAP mmap64
///////////////////////////////////////////////////////////////////////////////

/* page table entry*/
typedef uint32_t    Plmm_pte;

/* page desriptor */
typedef struct Plmm_pd_s
{
#ifdef MULTI_INSTANCE
    uint32_t busaddress:26;
    uint32_t unused:1;
#else    
    uint32_t busaddress:27;
#endif    
    uint32_t marked:1;
    uint32_t refcount:4;
}Plmm_pd;

typedef struct Plmm_iob_s
{
    union {
#ifndef __KERNEL__
    Plmm_bm valid;
#endif
    uint8_t valid8[SS_PAGES_PER_IOB];
    }u1;
    Ss_iobref   nextiob;
    uint32_t    flags;
    uint32_t    unusedpages;
    uint32_t    unusediobs;
    Plmm_pte    pages[SS_PAGES_PER_IOB];
}Plmm_iob;

typedef enum
{
    PLMM_JOURNAL_OP_NONE = 0,
    PLMM_JOURNAL_OP_COPY,
    PLMM_JOURNAL_OP_COPY_HARDEN,
    PLMM_JOURNAL_OP_MERGE,
    PLMM_JOURNAL_OP_MERGE_HARDEN,
    PLMM_JOURNAL_OP_SWAP,
    PLMM_JOURNAL_OP_AND,
    PLMM_JOURNAL_OP_ANDNOT,
    PLMM_JOURNAL_OP_MAX
}Plmm_journal_op;

#define PLMM_PATCHDATA_SIZE 128
typedef struct Plmm_patchdata_s
{
    Ss_iobref   reserved;
    uint8_t     buffer[PLMM_PATCHDATA_SIZE];
    uint32_t    terminator;
}Plmm_patchdata;

typedef struct Plmm_anc_s
{
    uint32_t    fpsindex;
    Ss_iobref   iobindex;
    uint32_t    oldlrc;
    uint32_t    newlrc;
    uint32_t    hardennewdata;
    uint32_t    startupprogress;
    uint16_t    node_site;
    uint16_t    node_iogroup;
    uint32_t    unused[4];
    Plmm_pte    zero_page;
    Plmm_pte    one_page;
    Plmm_pte    sink_page;
    Ss_iobref   zero_iobref;
    Ss_iobref   one_iobref;
    Ss_iobref   pools[PLMM_POOL_ID_HARD_LIMIT];
    Ss_iobref   components[PLMM_COMPONENT_MAX];
    uint8_t     programid[PLMM_MAX_PROGID_LENGTH];
    uint8_t     old_dll_id[PLMM_MAX_PROGID_LENGTH];
    uint8_t     new_dll_id[PLMM_MAX_PROGID_LENGTH];
    uint8_t     unused2[16];
    uint64_t    nodeid;
    uint64_t    clusterid;
    uint64_t    idlrc;
    Plmm_journal_op journal_op[PLMM_JOURNAL_MAX_SLOTS];
    Plmm_iob        journal_iob1[PLMM_JOURNAL_MAX_SLOTS];
    Plmm_iob        journal_iob2[PLMM_JOURNAL_MAX_SLOTS];
    Ss_iobref       journal_iobref1[PLMM_JOURNAL_MAX_SLOTS];
    Ss_iobref       journal_iobref2[PLMM_JOURNAL_MAX_SLOTS];
#ifdef __KERNEL__
    uint64_t        journal_bitmaps[PLMM_JOURNAL_MAX_SLOTS*2];
#else
    Plmm_bm        journal_bitmaps1[PLMM_JOURNAL_MAX_SLOTS];
    Plmm_bm        journal_bitmaps2[PLMM_JOURNAL_MAX_SLOTS];
#endif
    uint32_t        journal_lrc[PLMM_JOURNAL_MAX_SLOTS];
    uint8_t         assertstring[PLMM_MAX_ASSERT_LENGTH];
    Plmm_patchdata  patchdata;
    uint32_t        hwlevel;
    uint64_t        idlrc2;
}Plmm_anc;


///////////////////////////////////////////////////////////////////////////////
static int mq_memory_device;
static uint32_t mq_num_instances;
static uint32_t mq_num_iob_pages[PLMM_MAX_INSTANCE];
static uint32_t mq_num_pd_pages[PLMM_MAX_INSTANCE];
static uint32_t mq_num_gen_pages[PLMM_MAX_INSTANCE];
static Plmm_pd  *mq_pd[PLMM_MAX_INSTANCE];
static Plmm_iob *mq_it[PLMM_MAX_INSTANCE];
static uint8_t  *mq_base[PLMM_MAX_INSTANCE];
static Plmm_anc *mq_anc;

int main()
{
    uint32_t mapmode = 1;
    int rc = 0;
    uint32_t instance;
    uint32_t version;
    char keypress_buf[8];

    mq_memory_device = open("/dev/plmm", O_RDWR);
    if (mq_memory_device == -1)
    {
        printf("open device failed\n");
        return -1;
    }
    
    rc = ioctl(mq_memory_device, PLMM_QUERYVERSION, &version);
    if (rc == 0)
    {
        printf("PLMM  version = 0x%x\n", version);
    }

    rc = ioctl(mq_memory_device, PLMM_QUERYNUMINSTANCE, &mq_num_instances);
    if (rc == 0)
    {
        printf("PLMM mq_num_instances = 0x%d\n", mq_num_instances);
    }

    for (instance = 0; instance < mq_num_instances; instance++)
    {
        rc |= ioctl(mq_memory_device, PLMM_QUERYNUMBEROFITPAGES, &mq_num_iob_pages[instance]);
        rc |= ioctl(mq_memory_device, PLMM_QUERYNUMBEROFPDPAGES, &mq_num_pd_pages[instance]);
        rc |= ioctl(mq_memory_device, PLMM_QUERYNUMBEROFGENPAGES, &mq_num_gen_pages[instance]);

        /* Enable MQ process style mmaps */
        rc |= ioctl(mq_memory_device, PLMM_MQMMAPCTRL, &mapmode);
        
        if (rc != 0)
        {
            printf("PLMM  ioctl faied: %d\n", rc);
            break;
        }

        mq_it[instance] = (Plmm_iob *)MQ_MMAP(
            0,
            mq_num_iob_pages[instance]*PAGE_SIZE,
            PROT_READ,
            MAP_FILE|MAP_SHARED|MAP_POPULATE,
            mq_memory_device,
            ((uint64_t)(PLMM_IT_IOB + (instance << PLMM_INSTANCE_SHIFT))) * PAGE_SIZE);
        if (mq_it[instance] == MAP_FAILED)
        {
            mq_it[instance] = 0;
            rc = 1;
            printf("PLMM  MQ_MAP mq_it faied\n");
        }

        mq_pd[instance] = (Plmm_pd *)MQ_MMAP(
            0,
            mq_num_pd_pages[instance]*PAGE_SIZE,
            PROT_READ,
            MAP_FILE|MAP_SHARED|MAP_POPULATE,
            mq_memory_device,
            ((uint64_t)(PLMM_PD_IOB + (instance << PLMM_INSTANCE_SHIFT))) * PAGE_SIZE);
        if (mq_pd[instance] == MAP_FAILED)
        {
            mq_pd[instance] = 0;
            rc = 1;
            printf("PLMM  MQ_MAP mq_pd faied\n");
        }

        mq_base[instance] = (uint8_t *)MQ_MMAP(
            0L,
            mq_num_gen_pages[instance]*PAGE_SIZE,
            PROT_READ,
            MAP_FILE|MAP_SHARED|MAP_POPULATE,
            mq_memory_device,
            ((uint64_t)(PLMM_ANCHOR_IOB + (instance << PLMM_INSTANCE_SHIFT))) * PAGE_SIZE);
        if (mq_base[instance] == MAP_FAILED)
        {
            mq_base[instance] = 0;
            rc = 1;
            printf("PLMM  MQ_MAP mq_base faied\n");
        }
        else
        {
            if (instance == 0)
            {
                mq_anc = (Plmm_anc *)mq_base[instance];
            }
        }
    }

    if (rc == 0)
    {
        for (instance = 0; instance < mq_num_instances; instance++)
        {
            printf("show instance %d /dev/plmm base info\n", instance+1);
            printf("number of iob page is   %d\n", mq_num_iob_pages[instance]);
            printf("number of pd page is    %d\n", mq_num_pd_pages[instance]);
            printf("number of gen page is   %d\n", mq_num_gen_pages[instance]);
        }
    }

    // cleanup
    for (instance = 0; instance < mq_num_instances; instance++)
    {
        if (mq_base[instance] != 0)
        {
            munmap(mq_base[instance], (mq_num_gen_pages[instance] + 1) * PAGE_SIZE);
            mq_base[instance] = 0;
        }

        if (mq_it[instance] != 0)
        {
            munmap(mq_it[instance], mq_num_iob_pages[instance] * PAGE_SIZE);
            mq_it[instance] = 0;
        }

        if (mq_pd[instance] != 0)
        {
            munmap(mq_pd[instance], mq_num_pd_pages[instance] * PAGE_SIZE);
            mq_pd[instance] = 0;
        }
    }
    if (mq_memory_device != -1)
    {
        close(mq_memory_device);
        mq_memory_device = -1;
    }

    printf("PLMM test : finish show , press x to exit!\n");
    do
    {
        /* code */
        fgets(keypress_buf, 8-1, stdin);
    } while (0!= strcmp(keypress_buf, "x\n"));
   
    return 0;
}