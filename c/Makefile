
SRCDIR=.
TESTDIR=.
INCDIR=.
OUTDIR=.

FLAGS = -static -Wall -pthread -g -I $(INCDIR) -D CPU_TYPE_x86_64 


all: kpageflags_read \
	parse_pageflags \
	parse_swapentry \
	virt_to_phys \
	anonymous_page_exam

clean:
	rm -f kpageflags_read \
	parse_pageflags \
	parse_swapentry \
	virt_to_phys \
	anonymous_page_exam

kpageflags_read: $(TESTDIR)/kpageflags_read.c
	gcc -o $(OUTDIR)/kpageflags_read $(TESTDIR)/kpageflags_read.c $(FLAGS)

parse_pageflags: $(TESTDIR)/parse_pageflags.c
	gcc -o $(OUTDIR)/parse_pageflags $(TESTDIR)/parse_pageflags.c $(FLAGS)

parse_swapentry: $(TESTDIR)/parse_swapentry.c
	gcc -o $(OUTDIR)/parse_swapentry $(TESTDIR)/parse_swapentry.c $(FLAGS)

virt_to_phys: $(TESTDIR)/virt_to_phys.c
	gcc -o $(OUTDIR)/virt_to_phys $(TESTDIR)/virt_to_phys.c $(FLAGS)

anonymous_page_exam: $(TESTDIR)/anonymous_page_exam.c
	gcc -o $(OUTDIR)/anonymous_page_exam $(TESTDIR)/anonymous_page_exam.c $(FLAGS)

